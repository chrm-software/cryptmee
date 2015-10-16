#ifndef IMCONTROLTHREAD_H
#define IMCONTROLTHREAD_H

#include <QDeclarativeItem>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QList>
#include <QProcess>
#include <QMutex>

#include "otrlconnector.h"
#include "trackeraccess.h"

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

    QString toString() {
        return QString("From: ") + receivedMessageSender + ", Content: " + receivedMessageContent + ", Replacement: " + replacedMessageContent;
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
    Q_INVOKABLE bool startOTR(QString _account);
    Q_INVOKABLE void stopOTR(bool _onExit = false);
    Q_INVOKABLE void createPrivateKey(QString _account);
    Q_INVOKABLE QString getFingerprintForAccount(QString _account);
    Q_INVOKABLE bool sendOTRMessage(QString _account, QString _contact, QString _message);
    Q_INVOKABLE QString getLibOTRVersion();
    ////////////////////////////////////////////////////

    // public interface
    bool sendXMPPMessage(QString _receiver, QString _account, QString _contentOriginal, QString _contentEncrypted);
    bool replaceMsgInTracker(QString _origMsg, QString _replacement);
    bool meegoNotification(QString _message);
    void guiConnector(int _action, QString _value);

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

    QString accountName;
    QStringList allAccountNames;
    QStringList allKnownFingerprints;

    QString debugVariantList(const QVariantList &list);
    QString argumentToString(const QVariant &arg);

    bool parseParameter(QString _param);

    bool variantToString(const QVariant &arg, QString &out);
    bool argToString(const QDBusArgument &busArg, QString &out);

    void foundMessageInTracker(QString _message);
    void clearedMessageTextInTracker(QString _message);
    void updatedMessageTextInTracker(QString _message);

    bool telepathySendDBusMessage(QString _receiver, QString _account, QString _content);

    ImControlMessage lastMessage;
    TrackerAccess* myTrackerAccess;
    OTRLConnector* otrConnector;

    QProcess* process_mctool;
    QString mctoolStdOutput;
    QString mctoolErrOutput;
    int mctoolErrorCode;

private slots:
    void telepathyMessageReceived(const QDBusMessage &reply);
    void telepathyMessageSent(const QDBusMessage &reply);    

    void mctoolFinished(int _retVal);
    void mctoolError(QProcess::ProcessError _pe);
    void mctoolGetAccounts();

    void initLater();
    void autoStart();

signals:
    void gotAccounts();
    void initReady();
    void otrErrorOccured();
    void otrPrivKeyMustBeCreated();
    void otrPrivateKeyGenerated();
    void otrIsRunning();
    void otrHasStopped();
    void otrUpdateFingerprints();
};

#endif // IMCONTROLTHREAD_H
