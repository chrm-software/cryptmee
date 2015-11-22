#ifndef TRACKERACCESS_H
#define TRACKERACCESS_H

#include <QObject>
#include <QProcess>
#include <QMutex>
#include "configuration.h"
#include "imcontrolthread.h"

#define TRACKER_IDLE 0
#define TRACKER_GETMSGNUMBER 1
#define TRACKER_DELETEMSGTXT 2
#define TRACKER_REPLACEMSG 3
#define TRACKER_GETCONTACTS 4
#define TRACKER_GETAVATARS 5

class ImControlMessage;

class TrackerAccess : public QObject
{
    Q_OBJECT
public:
    explicit TrackerAccess(QObject *parent = 0);
    ~TrackerAccess();

    bool replaceMsgInTracker(QString _origMsg, QString _replacement, int _retry = 0);
    bool loadXMPPContacts(QString _account);
    QHash<QString, QString> getAllXMPPContacts();

private:
    QProcess* process_tracker;
    QString trackerBinaryPath;
    QString trackerStdOutput;
    QString trackerErrOutput;
    QMutex mutexSafeListAccess;
    QString accountName; // Telepathie account path
    bool shouldReloadXMPPContactsLater;

    bool processIsRunning;
    int trackerErrorCode;
    unsigned int trackerCurrentState;

    bool callTracker(QString _cmd, int _state);

    QString trackerCurrentMsgID;
    QString trackerMsgReplacement;
    QString trackerMsgOriginal;
    int trackerMsgRetry;

    bool parseXMPPContacts(QString _trackerRetVal);

    QList<ImControlMessage*> receivedMessages;
    QHash<QString, QString> allXMPPContacts;

private slots:
    void trackerFinished(int _retVal);
    void trackerError(QProcess::ProcessError _pe);
    void trackerGetMsgNumber();
    
signals:
    void gotUserList();
    void errorWhileLoadingUserList();
    
public slots:
    
};

#endif // TRACKERACCESS_H
