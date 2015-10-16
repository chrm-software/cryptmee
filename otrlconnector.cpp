#include "otrlconnector.h"
#include "configuration.h"

#include <QString>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <Qt>
#include <QCoreApplication>
#include <QtConcurrentRun>
#include <QFutureWatcher>

OTRLConnector::OTRLConnector(ImControlThread *_imControl)
{
    this->isGeneratingPrivKey = false;
    this->myImControl = _imControl;

    qDebug() << "OTRLConnector(): init OTRL...";
    OTRL_INIT;

    qDebug() << "OTRLConnector(): create userstate...";
    this->otrUserState = otrl_userstate_create();

    ui_ops.policy              = (*OTRLConnector::policy_cb);
    ui_ops.create_privkey      = (*OTRLConnector::create_privkey_cb);
    ui_ops.is_logged_in        = (*OTRLConnector::is_online_cb);
    ui_ops.inject_message      = (*OTRLConnector::inject_message_cb);
    ui_ops.update_context_list = (NULL);
    ui_ops.new_fingerprint     = (*OTRLConnector::new_fingerprint_cb);
    ui_ops.write_fingerprints  = (*OTRLConnector::write_fingerprints_cb);
    ui_ops.gone_secure         = (*OTRLConnector::gone_secure_cb);
    ui_ops.gone_insecure       = (*OTRLConnector::gone_insecure_cb);
    ui_ops.still_secure        = (NULL);
    ui_ops.log_message         = (NULL);
    ui_ops.notify              = (*OTRLConnector::notify_cb);
    ui_ops.display_otr_message = (*OTRLConnector::display_otr_message_cb);
    ui_ops.protocol_name       = (*OTRLConnector::protocol_name_cb);
    ui_ops.protocol_name_free  = (*OTRLConnector::protocol_name_free_cb);
    ui_ops.max_message_size    = (NULL);
    ui_ops.account_name        = (*OTRLConnector::account_name_cb);
    ui_ops.account_name_free   = (*OTRLConnector::account_name_free_cb);


    // Read keys and fingerprints
    qDebug() << "OTRLConnector(): load private key...";
    otrl_privkey_read(this->otrUserState,
                      QString(OTR_PRIVKEY_FILE).toLatin1().constData());

    qDebug() << "OTRLConnector(): load fingerprints...";
    otrl_privkey_read_fingerprints(this->otrUserState,
                                   QFile::encodeName(OTR_FINGERPRINTS_FILE).constData(),
                                   NULL,
                                   NULL);

    this->libOTRVersion = QString::number(OTRL_VERSION_MAJOR) + "." + QString::number(OTRL_VERSION_MINOR);
    qDebug() << "OTRLConnector(): OTR initialized. Using libotr version: " << this->libOTRVersion;
}

QString OTRLConnector::getLibOTRVersion()
{
    return this->libOTRVersion;
}

OTRLConnector::~OTRLConnector()
{
    qDebug() << "OTRLConnector::~OTRLConnector(): destroy object";
    otrl_userstate_free(this->otrUserState);
}

void OTRLConnector::setAccountName(QString _account)
{
    this->otrAccountName = _account;
}

// Checks if private key exists for given account name
bool OTRLConnector::hasPrivKeyForAccount(QString _account)
{
    return otrl_privkey_find(this->otrUserState,
                             _account.toUtf8().constData(),
                             OTR_PROTOCOL_STRING);
}

// Gets fingerprint for one private key
QString OTRLConnector::getFingerprintForAccount(QString _account)
{
    char fingerprintBuf[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];

    OtrlPrivKey* privKey = otrl_privkey_find(this->otrUserState,
                                             _account.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING);

    char* success = otrl_privkey_fingerprint(this->otrUserState,
                                             fingerprintBuf,
                                             privKey->accountname,
                                             OTR_PROTOCOL_STRING);
    if(success)
        return QString(fingerprintBuf);
    else
        return QString("NOT FOUND");
}

// Initial key management
bool OTRLConnector::setUpKeys()
{
    qDebug() << "OTRLConnector::setUpKeys(): load private key...";

    if (!this->hasPrivKeyForAccount(this->otrAccountName))
    {
        qDebug() << "OTRLConnector::setUpKeys(): Key not found! Generate one...";

        if(this->isGeneratingPrivKey) {
            qWarning() << "OTRLConnector::setUpKeys(): Already generating a key. Wait...";
            return false;
        }

        this->create_privkey(this->otrAccountName.toUtf8().constData(),
                             OTR_PROTOCOL_STRING);

        char fingerprint[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];

        if (otrl_privkey_fingerprint(this->otrUserState,
                                     fingerprint,
                                     this->otrAccountName.toUtf8().constData(),
                                     OTR_PROTOCOL_STRING))
        {
            qDebug() << "OTRLConnector::setUpKeys(): Key genearted successfully! Fingerprint: " << QString(fingerprint);
            this->myImControl->meegoNotification(QString("OTR key generated successfully.\nFingerprint: ") + fingerprint);
            this->myImControl->guiConnector(ACTION_KEY_GENERATED, QString(fingerprint));

        } else {
            qWarning() << "OTRLConnector::setUpKeys(): Unable to create private key. OTR will not work!";
            this->myImControl->meegoNotification("Unable to create private key. OTR will not work!");
            this->myImControl->guiConnector(ACTION_ERROR_OCCURED, QString("Unable to create private key. OTR will not work!"));

            return false;
        }
    }

    return true;
}

// Init OTR session to a remote contact
void OTRLConnector::startSession(const QString& _account, const QString& _contact)
{
    char* msg = otrl_proto_default_query_msg(_account.toUtf8().constData(),
                                             OTRL_POLICY_DEFAULT);

    qDebug() << "OTRLConnector::startSession(): init message: " << msg;

    this->myImControl->sendXMPPMessage(_contact, _account, QString("Opening OTR session"), QString::fromUtf8(msg));
    this->myImControl->meegoNotification(QObject::tr("Restarting OTR session for contact: ") + _contact);

    free(msg);
}

// Encrypt and send message
QString OTRLConnector::encryptMessage(const QString& _account, const QString& _contact,
                                      const QString& _message)
{
    qDebug() << "OTRLConnector::encryptMessage(): msg: " << _message.left(30) << "[...]";

    char* encMessage = NULL;
    gcry_error_t err;

    err = otrl_message_sending(this->otrUserState,
                               &ui_ops, this,
                               _account.toUtf8().constData(), OTR_PROTOCOL_STRING,
                               _contact.toUtf8().constData(),
                               _message.toUtf8().constData(),
                               NULL, &encMessage,
                               NULL, NULL);
    if (err)
    {
        QString err_message = QObject::tr("Encrypting message to %1 "
                                          "failed.\nThe message was not sent.")
                                          .arg(_contact);

        this->myImControl->meegoNotification(err_message);
        return QString();
    }

    if (encMessage)
    {        
        QString retMessage(QString::fromUtf8(encMessage));
        otrl_message_free(encMessage);

        qDebug() << "OTRLConnector::encryptMessage(): Encryption was successfull: " << retMessage.left(30) << "[...]";
        return retMessage;
    }

    return QString();
}

// Decrypt received message
QString OTRLConnector::decryptMessage(const QString& _account, const QString& _sender, const QString& _message)
{
    qDebug() << "OTRLConnector::decryptMessage(): encrypted msg: " << _message.left(30) << "[...]";

    // Decrypt here
    QByteArray accArray  = _account.toUtf8();
    QByteArray userArray = _sender.toUtf8();
    const char* accountName = accArray.constData();
    const char* userName    = userArray.constData();

    int ignoreMessage = 0;
    char* newMessage  = NULL;
    OtrlTLV* tlvs     = NULL;
    OtrlTLV* tlv      = NULL;

    ignoreMessage = otrl_message_receiving(this->otrUserState,
                                           &ui_ops,
                                           this,
                                           accountName,
                                           OTR_PROTOCOL_STRING,
                                           userName,
                                           _message.toUtf8().constData(),
                                           &newMessage, &tlvs, NULL, NULL);

    if(ignoreMessage) {
        qDebug() << "OTRLConnector::decryptMessage(): received message should be ignored.";
        return QString("[IN: Internal OTR message]");
    }

    qDebug() << "OTRLConnector::decryptMessage(): decrypted msg: " << QString(newMessage).left(30) << "[...]";
    QString retVal = ENCRYPT_SYMBOL + " " + QString::fromUtf8(newMessage);

    // Show decrypted msg as notification
    this->myImControl->meegoNotification(_sender + "\n" + QString::fromUtf8(newMessage));

    otrl_message_free(newMessage);

    // Check if remote has closed the session
    tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
    if (tlv) {
        qDebug() << "OTRLConnector::decryptMessage(): Session closed!";
        this->myImControl->meegoNotification(QObject::tr("OTR session closed for contact ") + _sender);
    }

    return retVal;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

OtrlUserState OTRLConnector::get_userstate(const char* username){
    Q_UNUSED(username);
    return this->otrUserState;
}

void OTRLConnector::create_privkey(const char* accountname, const char* protocol)
{
    qDebug() << "OTRLConnector()::create_privkey(): generate private key (will take a while...) for: " << accountname;

    if(this->isGeneratingPrivKey)
        return;

    this->isGeneratingPrivKey = true;

    QEventLoop loop;
    QFutureWatcher<gcry_error_t> watcher;

    QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));

    QFuture<gcry_error_t> future = QtConcurrent::run(otrl_privkey_generate,
                                                     this->otrUserState,
                                                     QString(OTR_PRIVKEY_FILE).toLatin1().constData(),
                                                     accountname,
                                                     protocol);
    watcher.setFuture(future);
    loop.exec();

    this->isGeneratingPrivKey = false;

    // Old code
    // otrl_privkey_generate(this->otrUserState, QString(OTR_PRIVKEY_FILE).toLatin1().constData(), accountname, protocol);

    otrl_privkey_read(this->otrUserState, QString(OTR_PRIVKEY_FILE).toLatin1().constData());

    qDebug("OTRLConnector()::create_privkey(): key generated");
}

// OTR wants to send a message
void OTRLConnector::inject_message(const char* accountname, const char* protocol,
                                   const char* recipient, const char* message)
{
    Q_UNUSED(protocol);

    qDebug() << "OTRLConnector::inject_message(): ";
    qDebug() << "from " << accountname;
    qDebug() << "to " << recipient;
    qDebug() << "message: " << QString(message).left(30) << "[...]";

    // Send OTR genearted message
    this->myImControl->sendXMPPMessage(QString(recipient), QString(accountname),
                                       QString("[OUT: Internal OTR message]"), QString::fromUtf8(message));
}


// OTR wants to write all fingerprints
void OTRLConnector::write_fingerprints()
{
    otrl_privkey_write_fingerprints(this->otrUserState,
                                    QFile::encodeName(OTR_FINGERPRINTS_FILE).constData());
}

void OTRLConnector::new_fingerprint(const char *username, char fingerprint[20])
{
    this->myImControl->meegoNotification("Fingerprint: " + QString(username) + "\n" + QString(fingerprint));
    this->myImControl->guiConnector(ACTION_NEW_FINGERPRINT, QString(fingerprint));
}

void OTRLConnector::gone_secure(ConnContext* context)
{
    Q_UNUSED(context);
    this->myImControl->meegoNotification(QObject::tr("Secure OTR session opened."));
}

void OTRLConnector::gone_insecure(ConnContext* context)
{
    Q_UNUSED(context);
    this->myImControl->meegoNotification(QObject::tr("OTR session closed"));
}

int OTRLConnector::display_otr_message(const char *msg)
{
    this->myImControl->meegoNotification(QString(msg));
    return 1;
}

void OTRLConnector::notify(OtrlNotifyLevel level,
                           const char *accountname,
                           const char *protocol,
                           const char *username,
                           const char *title, const char *primary, const char *secondary)
{
    Q_UNUSED(protocol);

    QString account = QString::fromUtf8(accountname);
    QString contact = QString::fromUtf8(username);
    QString message = QString(title) + "\n" + QString(primary) + ". " + QString(secondary);

    qDebug() << "OTRLConnector::notify(): Got OTR notify: " << message;

    /*if (level == OTRL_NOTIFY_ERROR || level == OTRL_NOTIFY_WARNING)
    {
        // On error or warning restart OTR for this contact
        qDebug() << "OTRLConnector::notify(): OTRL_NOTIFY_ERROR - No idea what to do here...";
        //this->startSession(account, contact);

        return;
    }*/

    // Show msg
    this->myImControl->meegoNotification(message);
}

void OTRLConnector::expireSession(const QString& _account, const QString& _contact)
{
    ConnContext* context = otrl_context_find(this->otrUserState,
                                             _contact.toUtf8().constData(),
                                             _account.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
                                             false, NULL, NULL, NULL);

    if (context && (context->msgstate == OTRL_MSGSTATE_ENCRYPTED))
    {
        otrl_context_force_finished(context);
    }
}

void OTRLConnector::endSession(const QString& _account, const QString& _contact)
{
    otrl_message_disconnect(this->otrUserState, &ui_ops, this,
                            _account.toUtf8().constData(), OTR_PROTOCOL_STRING,
                            _contact.toUtf8().constData());
}

QStringList OTRLConnector::getFingerprints()
{
    QStringList fpList;
    ConnContext* context;
    ::Fingerprint* fingerprint;

    for (context = this->otrUserState->context_root; context != NULL; context = context->next)
    {
        fingerprint = context->fingerprint_root.next;
        while(fingerprint)
        {
            char fpHash[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
            otrl_privkey_hash_to_human(fpHash, fingerprint->fingerprint);

            QString fp(QString(fpHash) + "|" + QString::fromUtf8(context->username));

            fpList.append(fp);
            fingerprint = fingerprint->next;
        }
    }

    return fpList;
}

const char* OTRLConnector::account_name(const char* account,
                                        const char* protocol)
{
    Q_UNUSED(protocol);

    qDebug() << "OTRLConnector::account_name()";
    return qstrdup(this->myImControl->getReadableAccountName(QString::fromUtf8(account)).toUtf8().constData());
}

void OTRLConnector::account_name_free(const char* account_name)
{
    delete [] account_name;
}


////////////// libotr callbacks //////////////

OtrlPolicy OTRLConnector::policy_cb(void *opdata, ConnContext *context)
{
    qDebug() << "OTR-CB: policy_cb(): return: " << OTRL_POLICY_ALWAYS;
    return OTRL_POLICY_ALWAYS;
}

void OTRLConnector::create_privkey_cb(void *opdata, const char *accountname,
                                      const char *protocol)
{
    qDebug() << "OTR-CB: create_privkey_cb()";
    qDebug() << "account = " << accountname;
    qDebug() << "filename = " << OTR_PRIVKEY_FILE;

    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->create_privkey(accountname, protocol);
    else
        qWarning() << "OTRLConnector::create_privkey_cb(): opdata == NULL";
}


const char* OTRLConnector::protocol_name_cb(void *opdata, const char *protocol)
{
    qDebug() << "OTR-CB: protocol_name_cb(): return: " << OTR_PROTOCOL_STRING;
    return OTR_PROTOCOL_STRING;
}

void OTRLConnector::protocol_name_free_cb(void* opdata, const char* protocol_name) {
    qDebug() << "OTR-CB: protocol_name_free_cb()";
    // Nothing to do
}

void OTRLConnector::new_fingerprint_cb(void *opdata, OtrlUserState us,
                                       const char *accountname, const char *protocol, const char *username,
                                       unsigned char fingerprint[20])
{
    qDebug() << "OTR-CB: new_fingerprint_cb() Fingerprint has changed! Infor user in GUI!";
    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->new_fingerprint(username, reinterpret_cast<char*>(fingerprint));
}

void OTRLConnector::write_fingerprints_cb(void *opdata)
{
    qDebug() << "OTR-CB: write_fingerprints_cb(): Write new fingerprint to store.";
    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->write_fingerprints();
}


int OTRLConnector::display_otr_message_cb(void *opdata, const char *accountname,
                                          const char *protocol, const char *username, const char *msg)
{

    qDebug() << "OTR-CB: display_otr_message_cb() TODO";
    qDebug("message = '%s'\n", msg);

    if(opdata != NULL)
        return static_cast<OTRLConnector*>(opdata)->display_otr_message(msg);

    return -1;
}


int OTRLConnector::is_online_cb(void *opdata, const char *accountname,
                                const char *protocol, const char *recipient)
{
    qDebug() << "OTR-CB: is_online_cb() TODO, will always return 1";
    return 1;
}

void OTRLConnector::gone_secure_cb(void* opdata, ConnContext* context) {
    qDebug() << "OTR-CB: gone_secure_cb()";

    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->gone_secure(context);
}

void OTRLConnector::gone_insecure_cb(void* opdata, ConnContext* context) {
    qDebug() << "OTR-CB: gone_insecure_cb()";

    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->gone_insecure(context);
}

void OTRLConnector::inject_message_cb(void *opdata, const char *accountname,
                                      const char *protocol, const char *recipient, const char *message)
{
    qDebug() << "OTR-CB: inject_message_cb()";

    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->inject_message(accountname, protocol, recipient, message);
    else
        qWarning() << "OTRLConnector::inject_message_cb(): opdata == NULL";
}

void OTRLConnector::notify_cb(void *opdata, OtrlNotifyLevel level,
                              const char *accountname, const char *protocol, const char *username,
                              const char *title, const char *primary, const char *secondary)
{
    qDebug() << "OTR-CB: notify_cb()";

    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->notify(level, accountname, protocol, username, title, primary, secondary);
}

const char* OTRLConnector::account_name_cb(void* opdata, const char* account,
                                           const char* protocol)
{
    qDebug() << "OTR-CB: account_name_cb()";

    if(opdata != NULL)
        return static_cast<OTRLConnector*>(opdata)->account_name(account, protocol);
}

void OTRLConnector::account_name_free_cb(void* opdata, const char* account_name)
{
    qDebug() << "OTR-CB: account_name_free_cb()";

    if(opdata != NULL)
        static_cast<OTRLConnector*>(opdata)->account_name_free(account_name);
}
