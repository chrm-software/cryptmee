#include <QTime>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QMetaType>
#include <QSettings>
#include <QDir>
#include <QTimer>

#include "imcontrolthread.h"

typedef QMap<QString, QVariant> KeyVarPair;
typedef QList<KeyVarPair> KeyVarPairList;

Q_DECLARE_METATYPE(KeyVarPair)
Q_DECLARE_METATYPE(KeyVarPairList)

ImControlThread::ImControlThread(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    qDebug() << "ImControlThread::ImControlThread()!";
    this->isActive = false;

    // Use mc-tool to get account list
    this->process_mctool = new QProcess(this);
    connect(this->process_mctool, SIGNAL(finished(int)), this, SLOT(mctoolFinished(int)));
    connect(this->process_mctool, SIGNAL(error(QProcess::ProcessError)), this, SLOT(mctoolError(QProcess::ProcessError)));
    connect(this, SIGNAL(gotAccounts()), this, SLOT(initLater()));

    this->otrConnector = NULL;
    this->myTrackerAccess = NULL;
    this->emojiManager = new EmojiManager(this);

    this->initializeObjects();

    this->updateContactsLaterTimer = new QTimer(this);
    this->updateContactsLaterTimer->setSingleShot(true);
    connect(this->updateContactsLaterTimer, SIGNAL(timeout()), this, SLOT(updateXMPPContactsLater()));

    // Autostart
    QTimer::singleShot(3500, this, SLOT(autoStart()));
}

ImControlThread::~ImControlThread()
{
    qDebug() << "ImControlThread::~ImControlThread(): destroy object";

    this->stopOTR(true);

    QStringList keys = this->chatHistory.keys();

    for(int i=0; i<keys.size(); i++) {
        qDebug() << "cleanup messages for contact: " + keys[i] << ", size: " << this->chatHistory[keys[i]].size();
        for(int j=0; j<this->chatHistory[keys[i]].size(); j++) {
            ChatMessage* cm = this->chatHistory[keys[i]].takeFirst();
            if(cm != NULL)
                delete cm;
        }
    }

    if(this->otrConnector != NULL)
        delete this->otrConnector;

    if(this->myTrackerAccess != NULL)
        delete this->myTrackerAccess;

    if(this->emojiManager != NULL)
        delete this->emojiManager;
}

QString ImControlThread::getLibOTRVersion()
{
    if(this->otrConnector != NULL)
        return "LibOTR version: " + this->otrConnector->getLibOTRVersion();
    else
        return QString();
}

void ImControlThread::initializeObjects() {
    qDebug() << "ImControlThread::initializeObjects()...";

    // Create tmp dir
    QDir tmpDir(OTR_DIR);
    if(!tmpDir.exists()) {
        tmpDir.mkpath(OTR_DIR);
    }

    // Create key file
    QFile privKeyFile(OTR_PRIVKEY_FILE);
    privKeyFile.open(QIODevice::ReadWrite);
    privKeyFile.close();

    if(this->myTrackerAccess == NULL) {
        this->myTrackerAccess = new TrackerAccess(this);
        connect(this->myTrackerAccess, SIGNAL(gotUserList()), this, SLOT(gotAllXMPPContacts()));
    }

    if(this->otrConnector == NULL)
        this->otrConnector = new OTRLConnector(this);
}

void ImControlThread::guiConnector(int _action, QString _value)
{
    Q_UNUSED(_value);

    if(_action == ACTION_NEED_GENKEY)
        emit otrPrivKeyMustBeCreated();
    else if(_action == ACTION_KEY_GENERATED)
        emit otrPrivateKeyGenerated();
    else if(_action == ACTION_ERROR_OCCURED)
        emit otrErrorOccured();
    else if(_action == ACTION_NEW_FINGERPRINT) {
        // Update contact list here, may be a new contact trying to contact us.
        this->myTrackerAccess->loadXMPPContacts(this->accountName);
    }
}

void ImControlThread::gotAllXMPPContacts()
{
    this->updateContactList();
}

QString ImControlThread::getAllXMPPAccounts()
{
    return this->allAccountNames.join("|");
}

int ImControlThread::getNumOfXMPPAccounts()
{
    return this->allAccountNames.size();
}

int ImControlThread::getNumOfFingerprints()
{
    return this->allKnownFingerprints.size();
}

QString ImControlThread::getFingerprint(int _num)
{
    return this->allKnownFingerprints.at(_num);
}

QStringList ImControlThread::getKnownOTRPartners()
{
    return this->knownOTRPartners;
}

bool ImControlThread::isFingerprintVerified(QString _account, QString _contact)
{
    return this->otrConnector->isVerified(_account, _contact);
}

QString ImControlThread::getReadableAccountName(QString _account)
{
    QString readableAccountName = _account.split("/").last();
    readableAccountName = readableAccountName.replace("_40", "@").replace("_2e", ".");

    if(readableAccountName.endsWith("0")) {
        readableAccountName.remove(QRegExp("0$"));
    }

    return readableAccountName;
}

QString ImControlThread::getXMPPAccount(int _num)
{
    QString accountPath = this->allAccountNames.at(_num);
    QString readableAccountName = getReadableAccountName(accountPath);

    return accountPath + "|" + readableAccountName;
}

QString ImControlThread::getFingerprintForAccount(QString _account)
{
    if(this->otrConnector != NULL) {
        return this->otrConnector->getFingerprintForAccount(_account);
    } else {
        return ("NOT INITIALIZED");
    }
}

void ImControlThread::verifyFingerprint(QString _account, QString _contact, QString _fingerprint, bool _verified)
{
    this->otrConnector->verifyFingerprint(_account, _contact, _fingerprint, _verified);
    this->updateContactList();
    this->meegoNotification(tr("Fingerprint changed"));
}

void ImControlThread::deleteFingerprint(QString _account, QString _contact, QString _fingerprint)
{
    this->otrConnector->deleteFingerprint(_account, _contact, _fingerprint);
    this->updateContactList();
    this->meegoNotification(tr("Fingerprint changed"));
}

bool ImControlThread::sendOTRMessage(QString _account, QString _contact, QString _message)
{
    qDebug() << "ImControlThread::sendOTRMessage(): encrypt message first...";

    if(_message == "")
        return false;

    // Check for resource extended contact and remember it.
    if(_contact.contains("/")) {
        qDebug() << "ImControlThread::sendOTRMessage(): contact has resource. Remember it: " << _contact;
        this->resourceOverwrites[_contact.split("/").at(0)] = _contact;
        _contact = _contact.split("/").at(0);
    } else {
        if(this->resourceOverwrites.contains(_contact))
            this->resourceOverwrites.remove(_contact);
    }

    QString encMsg = this->otrConnector->encryptMessage(_account, _contact, _message);

    if(encMsg != "") {
        return this->sendXMPPMessage(_contact, _account, ENCRYPT_SYMBOL + " " + _message, encMsg);
    }

    // Something went wrong. End session for this contact
    qWarning() << "ImControlThread::sendOTRMessage(): Unable to send message. End session.";
    this->otrConnector->endSession(this->accountName, _contact);

    return false;
}

bool ImControlThread::sendPlainTextMessage(QString _account, QString _contact, QString _message)
{
    qDebug() << "ImControlThread::sendPlainTextMessage()";

    if(_message == "")
        return false;

    // Check for resource extended contact and remember it.
    if(_contact.contains("/")) {
        qDebug() << "ImControlThread::sendPlainTextMessage(): contact has resource. Remember it: " << _contact;
        this->resourceOverwrites[_contact.split("/").at(0)] = _contact;
        _contact = _contact.split("/").at(0);
    } else {
        if(this->resourceOverwrites.contains(_contact))
            this->resourceOverwrites.remove(_contact);
    }

    // Set enc == dec this will do not start replacement in tracker
    return this->sendXMPPMessage(_contact, _account, _message, _message);
}

void ImControlThread::mctoolFinished(int _retVal)
{
    QString output = this->process_mctool->readAllStandardOutput().simplified();
    QString error = this->process_mctool->readAllStandardError().simplified();

    if(_retVal != 0) {
        qWarning() << "ImControlThread::mctoolFinished(): Error occured. RetVal=" << _retVal << ", StdErr: " << error;
        this->accountName = "ERROR";
        return;
    }

    QStringList tmpList = output.split(" ");

    this->allAccountNames.clear();

    // Allow only gabble accounts
    for(int i=0; i<tmpList.size(); i++) {
        if(tmpList.at(i).contains("gabble/")) {
            this->allAccountNames << "/org/freedesktop/Telepathy/Account/" + tmpList.at(i);
        }
    }

    qDebug() << "ImControlThread::mctoolFinished(): Accounts: " << this->allAccountNames;
    emit gotAccounts();
}

void ImControlThread::mctoolError(QProcess::ProcessError _pe)
{
    qWarning() << "ImControlThread::mctoolError(): Error occured: " << _pe;
}

void ImControlThread::initLater()
{
    // Do some init stuff here and inform QML
    emit initReady();
}

void ImControlThread::initialize()
{
    this->mctoolGetAccounts();
}

void ImControlThread::autoStart()
{
    QSettings settings;
    if(settings.value("SETTINGS_OTR_AUTORUN", "0").toString() == "1") {
        qDebug() << "ImControlThread::autoStart() - staring OTR because of AUTORUN setting.";

        QString account = settings.value("SETTINGS_OTR_ACCOUNT1", "UNKNOWN").toString();
        qDebug() << "ImControlThread::autoStart(): SETTINGS_OTR_ACCOUNT1 = " << account;

        if(account.contains("|"))
            this->startOTR(account.split("|").at(0));
        else
            qWarning() << "ImControlThread::autoStart() - Unable to autostart. Account: " << account;

    }
}

void ImControlThread::stopOTR(bool _onExit)
{
    if(!this->isActive)
        return;

    qDebug() << "ImControlThread::stopOTR(): stopping listeners";

    QDBusConnection::sessionBus().disconnect(QString(),
                                             QString(),
                                             "org.freedesktop.Telepathy.Channel.Interface.Messages",
                                             "MessageReceived",
                                             this,
                                             SLOT(telepathyMessageReceived(QDBusMessage)));

    QDBusConnection::sessionBus().disconnect(QString(),
                                             QString(),
                                             "org.freedesktop.Telepathy.Connection.Interface.SimplePresence" ,
                                             "PresencesChanged",
                                             this,
                                             SLOT(telepathyPresenceChanged(QDBusMessage)));

    this->isActive = false;

    // Get all active connections and end session!
    for(int i=0; i<this->knownOTRPartners.size(); i++) {
        qDebug() << "ImControlThread::stopOTR(): end session for: " << this->knownOTRPartners[i];
        this->otrConnector->endSession(this->accountName, this->knownOTRPartners[i]);
    }

    if(!_onExit) {
        QSettings settings;
        settings.setValue("SETTINGS_OTR_AUTORUN", "0");
    }

    this->meegoNotification(tr("OTR stopped!"));
    emit otrHasStopped();
}

bool ImControlThread::startOTR(QString _account)
{
    if(this->isActive) {
        qDebug() << "ImControlThread::startOTR(): Already active. Exit";
        return true;
    }

    qDebug() << "ImControlThread::startOTR(): START: Init listeners for account: " << _account;

    if(this->registerListeners()) {
        qDBusRegisterMetaType<KeyVarPair>();
        qDBusRegisterMetaType<KeyVarPairList>();
    } else {
        // Error handling
        qWarning() << "ImControlThread::startOTR(): Unable to register DBUS listener.";
        this->stopOTR();
        return false;
    }

    // Set account name
    this->otrConnector->setAccountName(_account);

    if(_account.startsWith("/org/freedesktop/Telepathy/Account/gabble/")) {
        // All ok, we got a formatted account name
        this->accountName = _account;
    } else {
        // Account isn't well formated, generate one
        this->accountName = _account.replace("@", "_40").replace(".", "_2e");

        for(int i=0; i<this->allAccountNames.size(); i++) {
            if(this->allAccountNames.at(i).contains(this->accountName)) {
                this->accountName = this->allAccountNames.at(i);
                break;
            }
        }

        qDebug() << "ImControlThread::startOTR(): corrected account: " << this->accountName;
    }

    // Checkif key exists
    if(!this->otrConnector->hasPrivKeyForAccount(this->accountName)) {
        qDebug() << "ImControlThread::startOTR(): No private key found. Generate it first. Stop.";
        emit otrPrivKeyMustBeCreated();
        this->stopOTR();
        return false;
    }

    // Init OTR keys
    this->otrConnector->setUpKeys();

    // Initialized.
    this->isActive = true;

    QSettings settings;
    settings.setValue("SETTINGS_OTR_AUTORUN", "1");

    QString readableAccountName = this->getReadableAccountName(this->accountName);
    settings.setValue("SETTINGS_OTR_ACCOUNT1", this->accountName + "|" + readableAccountName);

    this->meegoNotification(tr("OTR activated!"));
    this->allKnownFingerprints = this->otrConnector->getFingerprints();

    // Read all xmpp accounts
    this->myTrackerAccess->loadXMPPContacts(this->accountName);

    // Init emoji manager
    if(!this->emojiManager->initEmojis(EMOJI_DEF)) {
        qWarning() << "ImControlThread::startOTR(): Unable to initialize emojis.";
        this->useEmojis = false;
    } else
        this->useEmojis = true;

    emit otrIsRunning();

    return true;
}

void ImControlThread::createPrivateKey(QString _account)
{
    this->otrConnector->setAccountName(_account);

    if(this->otrConnector != NULL)
        this->otrConnector->setUpKeys();
    else
        emit otrErrorOccured();
}

void ImControlThread::mctoolGetAccounts()
{
    this->mctoolErrorCode = -1; // No error
    this->mctoolStdOutput = "";
    this->mctoolErrOutput = "No Errors";

    this->process_mctool->start(MCTOOL_BINARY + " list");
}

// Public interfaces //////////////////////////////////
bool ImControlThread::sendXMPPMessage(QString _receiver,
                                      QString _account,
                                      QString _contentOriginal,
                                      QString _contentEncrypted)
{
    // OTR init message, replace telepathy account name.
    if(_contentEncrypted.startsWith("?OTR?v")) {
        qDebug() << "ImControlThread::sendXMPPMessage(): Found ?OTR?v... init string. make some secial replacements";
        _contentOriginal = ENCRYPT_SYMBOL + " " + QObject::tr("[starting OTR session]");
        _contentEncrypted = "?OTR?v2? " + QObject::tr("I want to start a secure conversation. However, you do not have a plugin to support that.");
    }

    this->telepathySendDBusMessage(_receiver,
                                   _account,
                                   _contentEncrypted);

    if(_contentOriginal != _contentEncrypted) {
        qDebug() << "ImControlThread::sendXMPPMessage(): Start tracker replace for msg: " << _contentEncrypted.left(30) + "[...]";
        return this->myTrackerAccess->replaceMsgInTracker(_contentEncrypted, _contentOriginal);
    }

    return true;
}

bool ImControlThread::replaceMsgInTracker(QString _origMsg, QString _replacement)
{
    return this->myTrackerAccess->replaceMsgInTracker(_origMsg, _replacement);
}

void ImControlThread::addChatMessage(QString _contact, QString _message, bool _remote, bool _system, bool _sendEncrypted)
{
    // Check for HTML first
    _message.replace("<", "&lt;");

    // Remove resource
    if(_contact.contains("/"))
        _contact = _contact.split("/").at(0);

    if(this->useEmojis)
        _message = this->emojiManager->replaceEmojisInMsg(_message);

    ChatMessage* cm = new ChatMessage();
    cm->content = _message;
    cm->date = QDateTime::currentDateTime();
    cm->remote = _remote;
    cm->systemMessage = _system;
    cm->encrypted = _sendEncrypted;

    if(_sendEncrypted)
        cm->content = ENCRYPT_SYMBOL + " " + _message;

    this->chatHistory[_contact].append(cm);

    qDebug() << "ImControlThread::addChatMessage(): " << cm->toString();

    if(this->chatHistory[_contact].size() > 50) {
        qDebug() << "ImControlThread::addChatMessage(): more then 50 messages in history, delete oldest...";
        // hold only last 50 messages per contact
        delete this->chatHistory[_contact].takeFirst();
    }

    // Update pending message state
    if(!this->hasPendingMessages.value(_contact, false)) {
        this->hasPendingMessages[_contact] = true;
        qDebug() << "ImControlThread::addChatMessage(): Set new msg flag for: " << _contact << this->hasPendingMessages[_contact];
        this->updateContactList();
    } else {
        qDebug() << "ImControlThread::addChatMessage(): has already pending msgs or is sending one. Do not set new msg flag for: " << _contact;
    }

    emit otrUpdateChatHistory(_contact);
}

QString ImControlThread::getChatHistoryMessageFor(QString _contact, int _index)
{
    if(this->chatHistory[_contact].size() > _index)
        return this->chatHistory[_contact].at(_index)->toString();
    else
        return "";
}

int ImControlThread::getChatHistorySizeFor(QString _contact)
{
    // Update pending message state
    if(this->hasPendingMessages.contains(_contact) && this->hasPendingMessages.value(_contact, true)) {
        this->hasPendingMessages[_contact] = false;
        qDebug() << "ImControlThread::getChatHistorySizeFor(): unset new msg flag for contact: " << _contact;
        this->updateContactList();
    }

    return this->chatHistory[_contact].size();
}

QString ImControlThread::getNewestChatMessageFor(QString _contact)
{
    // Update pending message state
    if(this->hasPendingMessages.contains(_contact) && this->hasPendingMessages.value(_contact, true)) {
        this->hasPendingMessages[_contact] = false;
        qDebug() << "ImControlThread::getNewestChatMessageFor(): unset new msg flag for contact: " << _contact;
        this->updateContactList();
    }

    qDebug() << "ImControlThread::getNewestChatMessageFor(" << _contact << ")";

    if(this->chatHistory[_contact].size() > 0)
        return this->chatHistory[_contact].last()->toString();
    else
        return "";
}

bool ImControlThread::hasPendingMessageFor(QString _contact)
{
    return hasPendingMessages.value(_contact, false);
}

//////////////////////////////////////////////////////

bool ImControlThread::registerListeners()
{
    qDebug() << "ImControlThread::registerListeners()";

    // DBUS connection
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");

        return false;
    }

    qDebug() << "ImControlThread::initialize(): Go! - Init DBUS.";

    // Message received
    QDBusConnection::sessionBus().connect(QString(),
                                          QString(),
                                          "org.freedesktop.Telepathy.Channel.Interface.Messages",
                                          "MessageReceived",
                                          this,
                                          SLOT(telepathyMessageReceived(QDBusMessage)));

    // Presence change
    QDBusConnection::sessionBus().connect(QString(),
                                          QString(),
                                          "org.freedesktop.Telepathy.Connection.Interface.SimplePresence" ,
                                          "PresencesChanged",
                                          this,
                                          SLOT(telepathyPresenceChanged(QDBusMessage)));



    qDebug() << "ImControlThread::initialize(): DBUS initialized.";

    return true;
}

void ImControlThread::telepathyPresenceChanged(const QDBusMessage &reply)
{
    Q_UNUSED(reply);
    qDebug() << "ImControlThread::telepathyPresenceChanged() reload contacts later...";

    // Give tracker time for updating presence status
    this->updateContactsLaterTimer->start(4000);
}

void ImControlThread::updateXMPPContactsLater()
{
    this->myTrackerAccess->loadXMPPContacts(this->accountName);
}

QString ImControlThread::getDBUSConnectionStringForAccount(QString _account)
{
    // path includes also '_2f<resource>' and '/channel'.
    QString tmp = _account;

    if(tmp.endsWith("0")) {
        tmp.remove(QRegExp("0$"));
    }

    tmp.replace("/Account/", "/Connection/");

    return tmp;
}

void ImControlThread::telepathyMessageReceived(const QDBusMessage &reply)
{
    qDebug() << "ImControlThread::telepathyMessageReceived(const QDBusMessage &reply)";

    // Enter mutex
    this->mutexMsgReceived.lock();
    qDebug() << "ImControlThread::telepathyMessageReceived() MUTEX entered ------";

    this->debugVariantList(reply.arguments());

    // Now message is stored in this->lastMessage
    this->lastMessage.receivedMessagePath = reply.path();

    // Receive any message for this account

    // Process only messages for the active account!
    // compare path with this->accountName
    // path includes also '_2f<resource>' and '/channel'.
    QString tmp = this->getDBUSConnectionStringForAccount(this->accountName);

    // DEBUG
    qDebug() << "ImControlThread::telepathyMessageReceived() --> my account:      " << tmp;
    qDebug() << "ImControlThread::telepathyMessageReceived() --> msg for account: " << this->lastMessage.receivedMessagePath;

    if(this->lastMessage.receivedMessagePath.startsWith(tmp)) {
        qDebug() << "ImControlThread::telepathyMessageReceived() >>>>>>>>>>>> Got message for right account! <<<<<<<";

        if(this->lastMessage.receivedMessageContent.startsWith("?OTR")) {
            qDebug() << "ImControlThread::telepathyMessageReceived() >>>>>>>> Got OTR message! <<<<<<<";

            // Remember all encrypted conversations partners
            if(!this->knownOTRPartners.contains(this->lastMessage.receivedMessageSender)) {
                this->knownOTRPartners.append(this->lastMessage.receivedMessageSender);
                this->updateContactList();
            }

            // Do OTR decrypt stuff here.
            this->lastMessage.replacedMessageContent = this->otrConnector->decryptMessage(this->accountName,
                                                                                          this->lastMessage.receivedMessageSender,
                                                                                          this->lastMessage.receivedMessageContent);

            // Replace message in tracker
            if(this->lastMessage.replacedMessageContent != NULL && this->lastMessage.replacedMessageContent.size() > 0) {
                this->myTrackerAccess->replaceMsgInTracker(this->lastMessage.receivedMessageContent, this->lastMessage.replacedMessageContent);
            }

        } else {            
            qDebug() << "ImControlThread::telepathyMessageReceived() >>>>>> Got plaintext message. Account: " << tmp;

            // Just add to msg storage
            this->addChatMessage(this->lastMessage.receivedMessageSender, this->lastMessage.receivedMessageContent, true, false, false);
        }

    } else {
        qDebug() << "ImControlThread::telepathyMessageReceived(): Got message for wrong account. Ignore.";
    }

    // Leave mutex
    this->mutexMsgReceived.unlock();
    qDebug() << "ImControlThread::telepathyMessageReceived() MUTEX leave ------";
}

bool ImControlThread::telepathySendDBusMessage(QString _receiver, QString _account, QString _content)
{
    QString dbus_service = "org.freedesktop.Telepathy.ChannelDispatcher";
    QString dbus_path = "/org/freedesktop/Telepathy/ChannelDispatcher";
    QString dbus_interface = "org.freedesktop.Telepathy.ChannelDispatcher.Interface.Messages.DRAFT";

    QDBusMessage m = QDBusMessage::createMethodCall(dbus_service,
                                                    dbus_path,
                                                    dbus_interface,
                                                    "SendMessage");

    // Add ressource to contact name!
    if(this->resourceOverwrites.contains(_receiver)) {
        _receiver = this->resourceOverwrites[_receiver];
    }

    qDebug() << "ImControlThread::telepathySendDBusMessage(): " << _receiver << _content.left(30) + "[...]";

    QList<QVariant> args;

    // Path and targetID
    QDBusObjectPath path(_account);
    QString targetID = _receiver;
    args << qVariantFromValue(path);
    args << targetID;

    // Message Parts
    KeyVarPair message1;

    int type = 0;
    message1["message-type"] = qVariantFromValue(type);

    KeyVarPair message2;

    message2["content"] = qVariantFromValue(_content);
    message2["content-type"] = qVariantFromValue(QString("text/plain"));

    KeyVarPairList messageData;
    messageData << message1;
    messageData << message2;

    args << qVariantFromValue(messageData);

    // Flags
    uint flags = 0;
    args << flags;

    m.setArguments(args);

    qDebug() << "ImControlThread::telepathySendDBusMessage(): Calling DBUS...";
    QDBusMessage retVal = QDBusConnection::sessionBus().call(m);

    qDebug() << "ImControlThread::telepathySendDBusMessage(): Message sent: " << retVal;

    return true;
}

bool ImControlThread::meegoNotification(QString _message, bool _isUserMsg)
{
    QSettings settings;
    if(_isUserMsg && settings.value("SETTINGS_OTR_SHOW_NOTIFICATIONS", "0").toString() == "0") {
        // Do not show (decrypted) messages as notification
        return false;
    }

    QString dbus_service = "com.meego.core.MNotificationManager";
    QString dbus_path = "/notificationmanager";
    QString dbus_interface = "com.meego.core.MNotificationManager";

    qDebug() << "ImControlThread::meegoNotification(): " << _message;

    QDBusMessage m = QDBusMessage::createMethodCall(dbus_service,
                                                    dbus_path,
                                                    dbus_interface,
                                                    "addNotification");


    QList<QVariant> args;

    uint userID = 0;
    uint groupID = 0;
    QString eventType = "device";
    QString summary = "";
    QString msg = _message;
    QString action = "";
    QString imgURI = "/opt/CryptMee/splash/splash/CryptMeeOTR.png";
    uint count = 0;

    args << userID << groupID << eventType << summary << msg << action << imgURI << count;

    m.setArguments(args);

    QDBusMessage retVal = QDBusConnection::sessionBus().call(m);
    return true;
}

int ImControlThread::getNumOfAllContacts()
{
    return this->completeContactList.size();
}

QString ImControlThread::getContact(int _num)
{
    return this->completeContactList.at(_num);
}

void ImControlThread::updateContactList()
{
    qDebug() << "ImControlThread::updateContactList()";

    QString jid, lastDate;
    int collectionCnt = 0;

    this->completeContactList.clear();
    this->allKnownFingerprints = this->otrConnector->getFingerprints();

    for(int i=0; i<this->myTrackerAccess->getAllXMPPContacts().keys().size(); i++) {
        jid = this->myTrackerAccess->getAllXMPPContacts().keys().at(i);

        if(this->chatHistory.contains(jid) && !this->chatHistory[jid].isEmpty())
            lastDate = this->chatHistory[jid].last()->date.toString("yyyy-MM-dd &#124; hh:mm:ss") + " &#124; <i>" + this->chatHistory[jid].last()->content.left(8) + "...</i>";
        else
            lastDate = "---";

        this->completeContactList << "-|" + jid + "|offline|zunknown|" + QString::number(hasPendingMessages.value(jid, false)) + "|" + this->myTrackerAccess->getAllXMPPContacts().value(jid) + "|" + lastDate;

        for(int j=0; j<this->allKnownFingerprints.size(); j++) {
            if(this->allKnownFingerprints.at(j).split("|").at(1) == jid) {
                collectionCnt++;
                this->completeContactList[i] = this->allKnownFingerprints.at(j) + "|" + QString::number(hasPendingMessages.value(jid, false)) + "|" + this->myTrackerAccess->getAllXMPPContacts().value(jid) + "|" + lastDate;
                continue;
            }
        }
    }

    // Check if all fingerprints are in xmpp-contacts list
    if(collectionCnt != this->allKnownFingerprints.size()) {
        qWarning() << "ImControlThread::updateContactList(): not all fingerprints in list! Something went wrong!";
    }

    // Sort list
    this->sortContactListByOnlineState();

    emit otrUpdateFingerprints();
}

void ImControlThread::sortContactListByOnlineState()
{
    qSort(this->completeContactList.begin(), this->completeContactList.end(), lessThenComperator);
}

bool ImControlThread::lessThenComperator(QString _left, QString _right)
{
    QStringList leftList = _left.split("|");
    QStringList rightList = _right.split("|");

    if(leftList.size() != 9 || rightList.size() != 9) {
        qWarning() << "ImControlThread::lessThenComperator() wrong string format!";
        return _left < _right;
    }

    QString new1 = leftList.at(4);
    QString name1 = leftList.at(5);
    QString verify1 = leftList.at(3);
    QString onlineState1 = leftList.at(2);
    QString date1 = leftList.at(8);
    if(verify1 == "") verify1 = "y";

    QString new2 = rightList.at(4);
    QString name2 = rightList.at(5);
    QString verify2 = rightList.at(3);
    QString onlineState2 = rightList.at(2);
    QString date2 = rightList.at(8);
    if(verify2 == "") verify2 = "y";

    // Sort first by new message state
    if(new1 != new2) {
        return new1 > new2;
    }

    // Sort by last message date
    if(date1 != date2) {
        return date1 > date2;
    }

    // Sort by open session ('online') state
    if(onlineState1 != onlineState2) {
        return onlineState1 > onlineState2;
    }

    // Sort second by verified keys
    if(verify1 != verify2) {
        return verify1 < verify2;
    }

    // Sort by name
    return name1.toLower() < name2.toLower();
}

QString ImControlThread::makeLinksClickableInMsg(QString _msg)
{
    // replace images with a small preview
    if((_msg.startsWith("http://") || _msg.startsWith("https://") || _msg.startsWith(ENCRYPT_SYMBOL + " http://") || _msg.startsWith(ENCRYPT_SYMBOL + " https://")) &&
            (_msg.endsWith(".jpg") || _msg.endsWith(".JPG")) &&
            !_msg.contains(" ")) {
            _msg.replace(QRegExp("((?:https?|ftp)://\\S+)"), "<a href=\"\\1\"><img src=\"\\1\" width=\"200\" height=\"150\"></a>");
            qDebug() << "ImControlThread::makeLinksClickableInMsg(): create picture preview";

    } else {
        _msg.replace(QRegExp("((?:https?|ftp)://\\S+)"), "<a href=\"\\1\">\\1</a>");
    }
    return _msg;
}

int ImControlThread::getNumOfEmojis()
{
    return this->emojiManager->getNumOfEmojis();
}

QString ImControlThread::getEmojiPath(int _index)
{
    return this->emojiManager->getEmojiPath(_index);
}


////// DBUS helper methods ////////
// http://cep.xray.aps.anl.gov/software/qt4-x11-4.8.6-browser/d0/d78/qdbusutil_8cpp_source.html#l00318

bool ImControlThread::parseParameter(QString _param)
{
    QString delim = "\"=";

    if(_param.contains(delim)) {
        QString key, value;

        if(_param.split(delim).size() > 2) {
            qWarning() << "*** ImControlThread::parseParameter(): more then 2 parameters found in string. Could not parse values correctly";
        }

        key = _param.split(delim).at(0);
        key.replace("\"", "");
        key = key.trimmed();
        value = _param.split(delim).at(1);
        value.replace("\"", "");
        value = value.trimmed();

        if(key == "message-sender-id")
            this->lastMessage.receivedMessageSender = value;
        else if(key == "content-type")
            this->lastMessage.receivedMessageType = value;
        else if(key == "content")
            this->lastMessage.receivedMessageContent = value;

        return true;
    }

    return false;
}

QString ImControlThread::debugVariantList(const QVariantList &list)
{
    QString retVal;
    QVariantList::ConstIterator it = list.constBegin();
    QVariantList::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it) {
        retVal += ("Variant: " + this->argumentToString(*it));
    }

    return retVal;
}

QString ImControlThread::argumentToString(const QVariant &arg)
{
    QString out;
    ImControlThread::variantToString(arg, out);
    return out;
}

bool ImControlThread::variantToString(const QVariant &arg, QString &out)
{
    int argType = arg.userType();

    if (argType == QVariant::StringList) {
        out += QLatin1Char('{');
        QStringList list = arg.toStringList();
        foreach (QString item, list)
            out += QLatin1Char('\"') + item + QLatin1String("\", ");
        if (!list.isEmpty())
            out.chop(2);
        out += QLatin1Char('}');
    } else if (argType == QVariant::ByteArray) {
        out += QLatin1Char('{');
        QByteArray list = arg.toByteArray();
        for (int i = 0; i < list.count(); ++i) {
            out += QString::number(list.at(i));
            out += QLatin1String(", ");
        }
        if (!list.isEmpty())
            out.chop(2);
        out += QLatin1Char('}');
    } else if (argType == QVariant::List) {
        out += QLatin1Char('{');
        QList<QVariant> list = arg.toList();
        foreach (QVariant item, list) {
            if (!variantToString(item, out))
                return false;
            out += QLatin1String(", ");
        }
        if (!list.isEmpty())
            out.chop(2);
        out += QLatin1Char('}');
    } else if (argType == QMetaType::Char || argType == QMetaType::Short || argType == QMetaType::Int
               || argType == QMetaType::Long || argType == QMetaType::LongLong) {
        out += QString::number(arg.toLongLong());
    } else if (argType == QMetaType::UChar || argType == QMetaType::UShort || argType == QMetaType::UInt
               || argType == QMetaType::ULong || argType == QMetaType::ULongLong) {
        out += QString::number(arg.toULongLong());
    } else if (argType == QMetaType::Double) {
        out += QString::number(arg.toDouble());
    } else if (argType == QMetaType::Bool) {
        out += QLatin1String(arg.toBool() ? "true" : "false");
    } else if (argType == qMetaTypeId<QDBusArgument>()) {
        argToString(qvariant_cast<QDBusArgument>(arg), out);
    } else if (argType == qMetaTypeId<QDBusObjectPath>()) {
        const QString path = qvariant_cast<QDBusObjectPath>(arg).path();
        out += QLatin1String("[ObjectPath: ");
        out += path;
        out += QLatin1Char(']');
    } else if (argType == qMetaTypeId<QDBusSignature>()) {
        out += QLatin1String("[Signature: ") + qvariant_cast<QDBusSignature>(arg).signature();
        out += QLatin1Char(']');
    } else if (argType == qMetaTypeId<QDBusVariant>()) {
        const QVariant v = qvariant_cast<QDBusVariant>(arg).variant();
        if (!variantToString(v, out))
            return false;
    } else if (arg.canConvert(QVariant::String)) {
        out += QLatin1Char('\"') + arg.toString() + QLatin1Char('\"');
    } else {
        out += QLatin1Char('[');
        out += QLatin1String(arg.typeName());
        out += QLatin1Char(']');
    }

    return true;
}

bool ImControlThread::argToString(const QDBusArgument &busArg, QString &out)
{
    QString busSig = busArg.currentSignature();
    bool doIterate = false;
    QDBusArgument::ElementType elementType = busArg.currentType();

    if (elementType != QDBusArgument::BasicType && elementType != QDBusArgument::VariantType
            && elementType != QDBusArgument::MapEntryType)
        out += QLatin1String("[Argument: ") + busSig + QLatin1Char(' ');

    switch (elementType) {
    case QDBusArgument::BasicType:
    case QDBusArgument::VariantType:
        if (!variantToString(busArg.asVariant(), out))
            return false;
        break;
    case QDBusArgument::StructureType:
        busArg.beginStructure();
        doIterate = true;
        break;
    case QDBusArgument::ArrayType:
        busArg.beginArray();
        out += QLatin1Char('{');
        doIterate = true;
        break;
    case QDBusArgument::MapType:
        busArg.beginMap();
        out += QLatin1Char('{');
        doIterate = true;
        break;
    case QDBusArgument::MapEntryType: {
        busArg.beginMapEntry();
        QString mapValues;

        if (!variantToString(busArg.asVariant(), mapValues))
            return false;
        mapValues += QLatin1String("=");
        if (!argToString(busArg, mapValues))
            return false;
        busArg.endMapEntry();
        this->parseParameter(mapValues);
        out += mapValues; }
        break;
    case QDBusArgument::UnknownType:
    default:
        out += QLatin1String("<ERROR - Unknown Type>");
        return false;
    }
    if (doIterate && !busArg.atEnd()) {
        while (!busArg.atEnd()) {
            if (!argToString(busArg, out))
                return false;
            out += QLatin1String(", ");
        }
        out.chop(2);
    }
    switch (elementType) {
    case QDBusArgument::BasicType:
    case QDBusArgument::VariantType:
    case QDBusArgument::UnknownType:
    case QDBusArgument::MapEntryType:
        // nothing to do
        break;
    case QDBusArgument::StructureType:
        busArg.endStructure();
        break;
    case QDBusArgument::ArrayType:
        out += QLatin1Char('}');
        busArg.endArray();
        break;
    case QDBusArgument::MapType:
        out += QLatin1Char('}');
        busArg.endMap();
        break;
    }

    if (elementType != QDBusArgument::BasicType && elementType != QDBusArgument::VariantType
            && elementType != QDBusArgument::MapEntryType)
        out += QLatin1Char(']');

    return true;
}

