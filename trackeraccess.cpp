#include <QTimer>
#include <QDateTime>
//#include <QSparqlConnection>
#include "trackeraccess.h"

TrackerAccess::TrackerAccess(QObject *parent) :
    QObject(parent)
{
    this->processIsRunning = false;

    this->process_tracker = new QProcess(this);
    this->trackerCurrentState = TRACKER_IDLE;

    this->trackerStdOutput = "";
    this->trackerErrOutput = "No Errors";

    connect(this->process_tracker, SIGNAL(finished(int)), this, SLOT(trackerFinished(int)));
    connect(this->process_tracker, SIGNAL(error(QProcess::ProcessError)), this, SLOT(trackerError(QProcess::ProcessError)));

    /* // TODO: alternative tracker access
    QSparqlConnection connection("QTRACKER_DIRECT");
    QSparqlQuery query("SELECT ?u ?from ?ta ?ptc ?d { ?u a nmo:IMMessage; nmo:from ?from; tracker:added ?ta; nie:plainTextContent ?ptc; nmo:sentDate ?d FILTER(?ta > '" + timeWindow + "') FILTER (?ptc = '" + msg->receivedMessageContent.replace("\n","\\n") + "') } ORDER BY ?ta");
    QSparqlResult* result = connection.exec(query);

    if (!result->hasError()) {
        QObject::connect(result, SIGNAL(finished()), this, SLOT(onTrackerFinished()));
        QObject::connect(result, SIGNAL(dataReady(int)), this, SLOT(onTrackerDataReady(int)));
    }*/

}

TrackerAccess::~TrackerAccess()
{
    qDebug() << "TrackerAccess::~TrackerAccess(): destroy object";
    delete this->process_tracker;
}


bool TrackerAccess::replaceMsgInTracker(QString _origMsg, QString _replacement, int _retry)
{
    ImControlMessage* msg = new ImControlMessage();
    msg->receivedMessageContent = _origMsg;
    msg->replacedMessageContent = _replacement;
    msg->retryCounter = _retry;

    this->mutexSafeListAccess.lock();
    this->receivedMessages << msg;
    this->mutexSafeListAccess.unlock();

    // Replace last message in tracker later
    QTimer::singleShot(1500, this, SLOT(trackerGetMsgNumber()));

    return true;
}

// Slot called after msg received
void TrackerAccess::trackerGetMsgNumber()
{
    // Check if runnig
    if(this->processIsRunning) {
        //qWarning() << "TrackerAccess::trackerGetMsgNumber(): Another tracker process is running. Restart later.";
        QTimer::singleShot(900, this, SLOT(trackerGetMsgNumber()));
        return;
    }

    this->processIsRunning = true;

    // Set time to last 10sec. '2015-08-30T00:00:00Z'!
    QDateTime dt = QDateTime::currentDateTimeUtc();
    dt.setMSecsSinceEpoch(dt.toMSecsSinceEpoch() - (1000*10));
    QString timeWindow = dt.toString("yyyy-MM-ddThh:mm:ssZ");

    this->mutexSafeListAccess.lock();

    ImControlMessage* msg;

    if(!this->receivedMessages.isEmpty()) {
        msg = this->receivedMessages.takeFirst();
        this->trackerMsgReplacement = msg->replacedMessageContent;
        this->trackerMsgOriginal = msg->receivedMessageContent;
        this->trackerMsgRetry = msg->retryCounter;

        if(msg->retryCounter > 1) {
            qWarning() << "TrackerAccess::trackerGetMsgNumber(): max retry counter reached. Give up. Not able to replace content: " << this->trackerMsgOriginal;
            delete msg;
            this->processIsRunning = false;
            this->mutexSafeListAccess.unlock();
            return;
        }

        qDebug() << "TrackerAccess::trackerGetMsgNumber(): processing message: " << this->trackerMsgOriginal.left(30) << ". Still in list: " << this->receivedMessages.size();

        this->callTracker(QString(TRACKER_BINARY) +
                          QString(" -q \"SELECT ?u ?from ?ta ?ptc ?d { ?u a nmo:IMMessage; nmo:from ?from; tracker:added ?ta; nie:plainTextContent ?ptc; nmo:sentDate ?d FILTER(?ta > '" + timeWindow + "') FILTER (?ptc = '" + msg->receivedMessageContent.replace("\n","\\n").replace("'", "\\'") + "') } ORDER BY ?ta\""),
                          TRACKER_GETMSGNUMBER);
        delete msg;
    } else {
        qWarning() << "TrackerAccess::trackerGetMsgNumber(): Can't get last received message. Do nothing";
        this->processIsRunning = false;
        this->mutexSafeListAccess.unlock();
        return;
    }

    this->mutexSafeListAccess.unlock();
}


// Call gpg with given command
bool TrackerAccess::callTracker(QString _cmd, int _state)
{
    //qDebug() << "TrackerAccess::callTracker(" << _cmd << ", " << _state << ")";

    if(_cmd.size() < 5) {
        this->processIsRunning = false;
        return false;
    }

    this->trackerErrorCode = -1; // No error
    this->trackerStdOutput = "";
    this->trackerErrOutput = "No Errors";

    this->trackerCurrentState = _state;
    this->process_tracker->start(_cmd);

    return true;
}

void TrackerAccess::trackerFinished(int _retVal)
{
    //qDebug() << "TrackerAccess::trackerFinished(" << _retVal << ")";

    QString output = this->process_tracker->readAllStandardOutput().simplified();
    QString error = this->process_tracker->readAllStandardError().simplified();

    if(_retVal != 0) {
        this->trackerCurrentState = TRACKER_IDLE;
        qWarning() << "Error occured. RetVal=" << _retVal << ", StdErr: " << error;
        this->processIsRunning = false;
        return;
    }

    if(this->trackerCurrentState == TRACKER_GETMSGNUMBER && _retVal == 0) {
        if(output.contains("message:")) {
            // Clear Message now
            QString msgId = output;
            msgId = msgId.split(",").at(0);
            msgId.replace("Results: ", "");
            msgId.trimmed();

            this->trackerCurrentMsgID = msgId;

            if(this->trackerMsgReplacement.startsWith(ENCRYPT_SYMBOL)) {
                //qDebug() << "TrackerAccess::trackerFinished(): Calling DELETE TEXT now for msgID:" << this->trackerCurrentMsgID;
                // human readable message: just delete text
                this->callTracker(QString(TRACKER_BINARY) +
                                  QString(" -qu \"DELETE { <" + msgId + "> nie:plainTextContent ?t } WHERE  { <" + msgId + "> nie:plainTextContent ?t }\""),
                                  TRACKER_DELETEMSGTXT);
            } else {
                //qDebug() << "TrackerAccess::trackerFinished(): Calling DELETE complete now for msgID:" << this->trackerCurrentMsgID;
                // OTR internal message: remove message completly
                this->callTracker(QString(TRACKER_BINARY) +
                                  QString(" -qu \"DELETE { <" + msgId + "> a rdfs:Resource }\""),
                                  TRACKER_DELETEMSGTXT);
            }
        } else {
            qWarning() << "TrackerAccess::trackerFinished(): SELECT statement doesn't return a messageID. Replacement should be: [" << this->trackerMsgReplacement << "]";
            qWarning() << "TrackerAccess::trackerFinished(): trying once again. Retry: " << this->trackerMsgRetry;
            this->replaceMsgInTracker(this->trackerMsgOriginal, this->trackerMsgReplacement, ++this->trackerMsgRetry);
            this->processIsRunning = false;
        }

    } else if(this->trackerCurrentState == TRACKER_DELETEMSGTXT && _retVal == 0) {        
        if(this->trackerMsgReplacement.startsWith(ENCRYPT_SYMBOL)) {
            //qDebug() << "TrackerAccess::trackerFinished(): Calling INSERT now for msg:" << this->trackerCurrentMsgID;
            this->callTracker(QString(TRACKER_BINARY) +
                              QString(" -qu \"INSERT { <" + this->trackerCurrentMsgID + "> nie:plainTextContent '" + this->trackerMsgReplacement.replace("\n", "\\n").replace("'", "\\'") + "' }\""),
                              TRACKER_REPLACEMSG);
        } else {
            // No need for insert. Internal message already deleted completly
            this->trackerCurrentState = TRACKER_REPLACEMSG;
            this->trackerFinished(0);
        }

    } else if(this->trackerCurrentState == TRACKER_REPLACEMSG && _retVal == 0) {
        qDebug() << "TrackerAccess::trackerFinished(): Tracker job ended successfull for msgID:" << this->trackerCurrentMsgID;
        this->trackerCurrentState = TRACKER_IDLE;
        this->trackerCurrentMsgID = "";
        this->processIsRunning = false;

    } else {
        qWarning() << "TrackerAccess::trackerFinished(): Can't handle message. State:" << this->trackerCurrentState << ", RetVal:" << _retVal;
        this->processIsRunning = false;
        this->trackerCurrentState = TRACKER_IDLE;
    }
}

void TrackerAccess::trackerError(QProcess::ProcessError _pe)
{
    qDebug() << "TrackerAccess::trackerError(): " << _pe;
    this->processIsRunning = false;
}

