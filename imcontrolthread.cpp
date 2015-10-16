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

    this->initializeObjects();

    // Autostart
    QTimer::singleShot(2500, this, SLOT(autoStart()));
}

ImControlThread::~ImControlThread()
{
    qDebug() << "ImControlThread::~ImControlThread(): destroy object";

    this->stopOTR(true);

    if(this->otrConnector != NULL)
        delete this->otrConnector;

    if(this->myTrackerAccess != NULL)
        delete this->myTrackerAccess;
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

    if(this->myTrackerAccess == NULL)
        this->myTrackerAccess = new TrackerAccess(this);

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
        this->allKnownFingerprints = this->otrConnector->getFingerprints();
        emit otrUpdateFingerprints();
    }
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

bool ImControlThread::sendOTRMessage(QString _account, QString _contact, QString _message)
{
    qDebug() << "ImControlThread::sendOTRMessage(): encrypt message first...";

    QString encMsg = this->otrConnector->encryptMessage(_account, _contact, _message);

    if(_message == "")
        return false;

    // Check for resource extended contact and remember it.
    if(_contact.contains("/")) {
        qDebug() << "ImControlThread::sendOTRMessage(): contact has resource. Remember it: " << _contact;
        this->resourceOverwrites[_contact.split("/").at(0)] = _contact;
    } else {
        if(this->resourceOverwrites.contains(_contact))
            this->resourceOverwrites.remove(_contact);
    }

    if(encMsg != "") {
        return this->sendXMPPMessage(_contact, _account, ENCRYPT_SYMBOL + " " + _message, encMsg);
    }

    return false;
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
                                             "org.freedesktop.Telepathy.Channel.Interface.Messages" ,
                                             "MessageSent",
                                             this,
                                             SLOT(telepathyMessageSent(QDBusMessage)));

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
    this->telepathySendDBusMessage(_receiver,
                                   _account,
                                   _contentEncrypted);

    if(_contentOriginal != _contentEncrypted)
        return this->myTrackerAccess->replaceMsgInTracker(_contentEncrypted, _contentOriginal);

    return true;
}

bool ImControlThread::replaceMsgInTracker(QString _origMsg, QString _replacement)
{
    return this->myTrackerAccess->replaceMsgInTracker(_origMsg, _replacement);
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

    // Message sent
    QDBusConnection::sessionBus().connect(QString(),
                                          QString(),
                                          "org.freedesktop.Telepathy.Channel.Interface.Messages" ,
                                          "MessageSent",
                                          this,
                                          SLOT(telepathyMessageSent(QDBusMessage)));


    qDebug() << "ImControlThread::initialize(): DBUS initialized.";

    return true;
}

void ImControlThread::telepathyMessageSent(const QDBusMessage &reply)
{
    qDebug() << "ImControlThread::telepathyMessageSent() ....";

    QList<QVariant> args = reply.arguments();

    qDebug() << "--> List size:" << args.size(); // 3
    /*
    [Argument: aa{sv} {
        [Argument: a{sv}
            {"message-type" = [Variant(int): 0],
             "message-token" = [Variant(QString): "c7ec240b-aabd-42cb-abfa-fafbfcab4fe1"],
             "message-sent" = [Variant(qulonglong): 1442501592],
             "message-sender" = [Variant(uint): 1],
             "message-sender-id" = [Variant(QString): "chrm@XXX.de"]
        }],
        [Argument: a{sv}
            {"content" = [Variant(QString): "?OTR ..."], "content-type" = [Variant(QString): "text/plain"]
        }]}
     ]
     */
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

    if(this->lastMessage.receivedMessageContent.startsWith("?OTR")) {

        // Process only messages for the active account!
        // compare path with this->accountName
        // path includes also '_2f<resource>' and '/channel'.
        QString tmp = this->accountName;
        if(tmp.endsWith("0")) {
            tmp.remove(QRegExp("0$"));
        }
        tmp.replace("/Account/", "/Connection/");

        // DEBUG
        qDebug() << "ImControlThread::telepathyMessageReceived() --> my account:      " << tmp;
        qDebug() << "ImControlThread::telepathyMessageReceived() --> msg for account: " << this->lastMessage.receivedMessagePath;

        if(this->lastMessage.receivedMessagePath.startsWith(tmp)) {
            qDebug() << "ImControlThread::telepathyMessageReceived() >>>>>>>> Got OTR message for right account! <<<<<<<";

            // Remember all encrypted conversations partners
            if(!this->knownOTRPartners.contains(this->lastMessage.receivedMessageSender))
                this->knownOTRPartners.append(this->lastMessage.receivedMessageSender);

            if(this->resourceOverwrites.contains(this->lastMessage.receivedMessageSender)) {
                // Replace with resource extension.
                this->lastMessage.receivedMessageSender = this->resourceOverwrites[this->lastMessage.receivedMessageSender];
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
            qDebug() << "ImControlThread::telepathyMessageReceived() Got OTR message for wrong account. Do nothing. Account: " << tmp;
        }

    } else {
        qDebug() << "ImControlThread::telepathyMessageReceived(): No OTR init string found. Do nothing";
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

    qDebug() << "ImControlThread::telepathySendDBusMessage(): " << _content.left(30) << "[...]";

    QDBusMessage m = QDBusMessage::createMethodCall(dbus_service,
                                                    dbus_path,
                                                    dbus_interface,
                                                    "SendMessage");


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

    qDebug() << "ImControlThread::telepathySendDBusMessage(): Sending message...";
    QDBusMessage retVal = QDBusConnection::sessionBus().call(m);

    qDebug() << "ImControlThread::telepathySendDBusMessage(): Message sent: " << retVal;

    return true;
}

bool ImControlThread::meegoNotification(QString _message)
{
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

