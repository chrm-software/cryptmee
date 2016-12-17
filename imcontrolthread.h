#ifndef IMCONTROLTHREAD_H
#define IMCONTROLTHREAD_H

#include <QDeclarativeItem>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDateTime>
#include <QList>
#include <QProcess>
#include <QMutex>
#include <QDeclarativeImageProvider>
#include <QBitmap>
#include <QPainter>

#include "otrlconnector.h"
#include "trackeraccess.h"
#include "emojimanager.h"

#define ACTION_NEED_GENKEY 0
#define ACTION_KEY_GENERATED 1
#define ACTION_ERROR_OCCURED 2
#define ACTION_NEW_FINGERPRINT 3

class OTRLConnector;
class TrackerAccess;

class ImControlMessage
{
public:
    QString receivedMessageContent;
    QString receivedMessageType;
    QString receivedMessageSender;
    QString receivedMessagePath;
    QString replacedMessageContent;
    int retryCounter;
    bool encrypted;

    QString toString() {
        return QString("From: ") + receivedMessageSender + ", Content: " + receivedMessageContent + ", Replacement: " + replacedMessageContent + ", Encrypted: " + encrypted;
    }
};

class ChatMessage
{
public:
    QString content;
    bool remote;
    QDateTime date;
    bool systemMessage;
    bool encrypted;

    QString toString() {
        return content.replace("|", "&#124;") + "|" + QString::number(remote) + "|" + date.toString("hh:mm:ss") + "|" + QString::number(systemMessage) + "|" + QString::number(encrypted);
    }
};

class AvatarImageProvider : public QDeclarativeImageProvider
{
public:
    AvatarImageProvider()
        : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
    {
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        QString pathName = id;
        qDebug() << "AvatarImageProvider::requestPixmap(): " << pathName << requestedSize << size;

        QPixmap squircleMaskPath("/usr/share/themes/blanco/meegotouch/images/theme/basement/meegotouch-avatar/meegotouch-avatar-placeholder-background.png");
        squircleMaskPath = squircleMaskPath.scaled(squircleMaskPath.width()-2, squircleMaskPath.height()-2);
        QPixmap squircleBorder("/usr/share/themes/blanco/meegotouch/images/theme/basement/meegotouch-avatar/meegotouch-avatar-frame-small.png");

        if(pathName.startsWith("file://")) {
            QPixmap avatar(pathName.replace("file://", ""));
            avatar = avatar.scaled(squircleMaskPath.size());
            avatar.setMask(squircleMaskPath.mask());

            QPainter painter(&avatar);
            //painter.setBackgroundMode(Qt::TransparentMode);
            //painter.setRenderHint(QPainter::Antialiasing);
            painter.drawPixmap(0, 0, squircleBorder);

            //painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            //painter.drawPixmap(0, 0, squircleMaskPath.width(), squircleMaskPath.height(), squircleMaskPath);

            return avatar;
        }

        return squircleMaskPath;
    }
};

class ImControlThread : public QDeclarativeItem
{
    Q_OBJECT
public:
    explicit ImControlThread(QDeclarativeItem *parent = 0);
    ~ImControlThread();

    // QML interface ///////////////////////////////////
    Q_INVOKABLE void initialize();

    Q_INVOKABLE QString getAllXMPPAccounts();
    Q_INVOKABLE int getNumOfXMPPAccounts();
    Q_INVOKABLE QString getXMPPAccount(int _num);

    Q_INVOKABLE int getNumOfFingerprints();
    Q_INVOKABLE QString getFingerprint(int _num);

    Q_INVOKABLE int getNumOfAllContacts();
    Q_INVOKABLE QString getContact(int _num);

    Q_INVOKABLE bool startOTR(QString _account);
    Q_INVOKABLE void stopOTR(bool _onExit = false);

    Q_INVOKABLE void createPrivateKey(QString _account);
    Q_INVOKABLE QString getFingerprintForAccount(QString _account);
    Q_INVOKABLE bool sendOTRMessage(QString _account, QString _contact, QString _message);
    Q_INVOKABLE bool sendPlainTextMessage(QString _account, QString _contact, QString _message);
    Q_INVOKABLE QString getLibOTRVersion();

    Q_INVOKABLE QString getChatHistoryMessageFor(QString _contact, int _index);
    Q_INVOKABLE int getChatHistorySizeFor(QString _contact);
    Q_INVOKABLE QString getNewestChatMessageFor(QString _contact);
    Q_INVOKABLE void addChatMessage(QString _contact, QString _message, bool _remote, bool _system, bool _sendEncrypted = true);
    Q_INVOKABLE void clearMessageHistoryFor(QString _contact);
    Q_INVOKABLE bool clearUnreadMsgFlag(QString _contact);

    Q_INVOKABLE void verifyFingerprint(QString _account, QString _contact, QString _fingerprint, bool _verified);
    Q_INVOKABLE void deleteFingerprint(QString _account, QString _contact, QString _fingerprint);
    Q_INVOKABLE bool isFingerprintVerified(QString _account, QString _contact);

    Q_INVOKABLE QString makeLinksClickableInMsg(QString _msg);

    Q_INVOKABLE int getNumOfEmojis();
    Q_INVOKABLE QString getEmojiPath(int _index);
    ////////////////////////////////////////////////////

    // public interface
    bool sendXMPPMessage(QString _receiver, QString _account, QString _contentOriginal, QString _contentEncrypted);
    bool replaceMsgInTracker(QString _origMsg, QString _replacement);
    bool meegoNotification(QString _message, bool _isUserMsg = false);
    void guiConnector(int _action, QString _value);
    QStringList getKnownOTRPartners();
    QString getReadableAccountName(QString _account);   

    
public slots:

signals:
    void readyThread();
    void errorOccuredThread();

private:
    QMutex mutexMsgReceived;
    bool isActive;
    void initializeObjects();
    bool registerListeners();
    void unregisterListeners();

    QStringList knownOTRPartners;
    QHash<QString, QString> resourceOverwrites;
    QHash<QString, QList<ChatMessage*> > chatHistory;
    QHash<QString, bool> hasPendingMessages;

    QString accountName;
    QStringList allAccountNames;
    QStringList allKnownFingerprints;

    QString debugVariantList(const QVariantList &list);
    QString argumentToString(const QVariant &arg);

    bool parseParameter(QString _param);


    bool variantToString(const QVariant &arg, QString &out);
    bool argToString(const QDBusArgument &busArg, QString &out);
    bool parsingUnreadFlag;

    void foundMessageInTracker(QString _message);
    void clearedMessageTextInTracker(QString _message);
    void updatedMessageTextInTracker(QString _message);

    bool telepathySendDBusMessage(QString _receiver, QString _account, QString _content);
    QString getDBUSConnectionStringForAccount(QString _account);

    ImControlMessage lastMessage;
    TrackerAccess* myTrackerAccess;
    OTRLConnector* otrConnector;

    QProcess* process_mctool;
    QString mctoolStdOutput;
    QString mctoolErrOutput;
    int mctoolErrorCode;

    QStringList completeContactList;
    void updateContactList();
    QTimer *updateContactsLaterTimer;

    void sortContactListByOnlineState();
    static bool lessThenComperator(QString _left, QString _right);

    EmojiManager *emojiManager;
    bool useEmojis;

private slots:
    void telepathyMessageReceived(const QDBusMessage &reply);
    void telepathyPresenceChanged(const QDBusMessage &reply);
    void telepathyConversationOpened(const QDBusMessage &reply);

    void mctoolFinished(int _retVal);
    void mctoolError(QProcess::ProcessError _pe);
    void mctoolGetAccounts();

    void initLater();
    void autoStart();

    void gotAllXMPPContacts();
    void updateXMPPContactsLater();

signals:
    void gotAccounts();
    void initReady();
    void otrErrorOccured();
    void otrPrivKeyMustBeCreated();
    void otrPrivateKeyGenerated();
    void otrIsRunning();
    void otrHasStopped();
    void otrUpdateFingerprints();
    void otrUpdateChatHistory(QString _contactName);
};

#endif // IMCONTROLTHREAD_H
