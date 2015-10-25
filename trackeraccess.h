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

class ImControlMessage;

class TrackerAccess : public QObject
{
    Q_OBJECT
public:
    explicit TrackerAccess(QObject *parent = 0);
    ~TrackerAccess();

    bool replaceMsgInTracker(QString _origMsg, QString _replacement, int _retry = 0);

private:
    QProcess* process_tracker;
    QString trackerBinaryPath;
    QString trackerStdOutput;
    QString trackerErrOutput;
    QMutex mutexSafeListAccess;

    bool processIsRunning;
    int trackerErrorCode;
    unsigned int trackerCurrentState;

    bool callTracker(QString _cmd, int _state);

    QString trackerCurrentMsgID;
    QString trackerMsgReplacement;
    QString trackerMsgOriginal;
    int trackerMsgRetry;

    QList<ImControlMessage*> receivedMessages;

private slots:
    void trackerFinished(int _retVal);
    void trackerError(QProcess::ProcessError _pe);
    void trackerGetMsgNumber();
    
signals:
    
public slots:
    
};

#endif // TRACKERACCESS_H
