#include <QTimer>
#include <QDateTime>
//#include <QSparqlConnection>
#include "trackeraccess.h"

TrackerAccess::TrackerAccess(QObject *parent) :
    QObject(parent)
{
    this->processIsRunning = false;
    this->shouldReloadXMPPContactsLater = false;

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

bool TrackerAccess::loadXMPPContacts(QString _account)
{
    if(this->processIsRunning) {
        qWarning() << "TrackerAccess::loadXMPPContacts(): Another tracker process is running. Reload later.";
        this->shouldReloadXMPPContactsLater = true;
        return false;
    }

    this->processIsRunning = true;
    this->accountName = _account;
    this->shouldReloadXMPPContactsLater = false;

    qDebug() << "TrackerAccess::getXMPPContacts(" + _account + ")";
    this->callTracker(QString(TRACKER_BINARY) + \
                      QString(" -q \"SELECT nco:imPresence(?xmpp) nco:imNickname(?xmpp) nie:url(nco:photo(?contact)) ?name ?family ?xmpp  WHERE ") + \
                              "{ ?contact  a nco:PersonContact . OPTIONAL { ?contact nco:nameGiven ?name . ?contact nco:nameFamily ?family . } " + \
                              "?contact  nco:hasAffiliation [  nco:hasIMAddress ?xmpp ] . FILTER (fn:starts-with(?xmpp, 'telepathy:" + this->accountName + "'))}\"",
                      TRACKER_GETCONTACTS);    


    return true;
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
    this->mutexSafeListAccess.lock();

    ImControlMessage* msg;

    if(!this->receivedMessages.isEmpty()) {
        msg = this->receivedMessages.takeFirst();
        this->trackerMsgReplacement = msg->replacedMessageContent;
        this->trackerMsgOriginal = msg->receivedMessageContent;
        this->trackerMsgRetry = msg->retryCounter;

        // Set time to last 15sec. '2015-08-30T00:00:00Z'!
        int timeGap = 15;

        if(msg->retryCounter > 0)
            timeGap = 60;

        QDateTime dt = QDateTime::currentDateTimeUtc();
        dt.setMSecsSinceEpoch(dt.toMSecsSinceEpoch() - (1000 * timeGap));
        QString timeWindow = dt.toString("yyyy-MM-ddThh:mm:ssZ");

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
    qDebug() << "TrackerAccess::callTracker(" << _cmd << ", " << _state << ")";

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

    QString output = QString::fromUtf8(this->process_tracker->readAllStandardOutput().data());
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
            QString msgId = output.simplified();
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

        if(this->shouldReloadXMPPContactsLater) {
            qDebug() << "TrackerAccess::trackerFinished(): we should reload XMPP contacts now.";
            this->loadXMPPContacts(this->accountName);
        }

    } else if(this->trackerCurrentState == TRACKER_GETCONTACTS && _retVal == 0) {
        qDebug() << "TrackerAccess::trackerFinished(): Tracker job ended successfull for getting contacts:";
        this->trackerCurrentState = TRACKER_IDLE;
        this->trackerCurrentMsgID = "";
        this->processIsRunning = false;

        if(this->parseXMPPContacts(output))
            emit gotUserList();
        else
            emit errorWhileLoadingUserList();

    } else {
        qWarning() << "TrackerAccess::trackerFinished(): Can't handle message. State:" << this->trackerCurrentState << ", RetVal:" << _retVal;
        this->processIsRunning = false;
        this->trackerCurrentState = TRACKER_IDLE;
    }
}

void TrackerAccess::trackerError(QProcess::ProcessError _pe)
{
    qWarning() << "TrackerAccess::trackerError(): " << _pe;
    this->processIsRunning = false;
}

bool TrackerAccess::parseXMPPContacts(QString _trackerRetVal)
{
    if(!_trackerRetVal.contains(", "))
        return false;

    QStringList tmpList = _trackerRetVal.split("\n");
    QString contactNames, entry, presence, key, key2;
    QStringList line;

    qDebug() << "TrackerAccess::parseXMPPContacts() size:" << tmpList.size();

    // Parse one row which contains the following data:
    // < presence-status, nickname, photo, given-name, family-name, jid >

    for(int i=0; i<tmpList.size(); i++) {
        line = tmpList.at(i).split(", ");

        if(line.size() == 6) {

            contactNames = line.at(3) + " " + line.at(4); // name family-name
            key = line.at(5); // use jid as key

            // Replace empty name with nickname
            if(contactNames.startsWith("(null)"))
                contactNames = line.at(1);

            if(key.contains(this->accountName)) {
                key2 = key.replace("telepathy:", "").replace(this->accountName, "").replace("!", "");

                // Replace empty name with jid
                if(contactNames.startsWith("(null)"))
                    contactNames = key2;

                // Create entry: (name|presence|photo)
                // presence could be:
                /*  presence-status-unset
                    presence-status-offline
                    presence-status-available
                    presence-status-away
                    presence-status-extended-away
                    presence-status-hidden
                    presence-status-busy
                    presence-status-unknown
                    presence-status-error  */
                // photo is a local path like: "file:///home/user/.local/share//data/libqtcontacts-tracker/photos/4b6ce96fbc3235bc5ce1b8e63f710dc56002bdb5.png"
                presence = line.at(0);
                entry = contactNames.replace("|", "/") + "|" + presence.replace("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#", "").trimmed() + "|" + line.at(2);

                this->allXMPPContacts[key2] = entry;
            }
        }
    }

    //qDebug() << "parseXMPPContacts(): " << this->allXMPPContacts.keys();
    return true;
}

QHash<QString, QString> TrackerAccess::getAllXMPPContacts()
{
    return this->allXMPPContacts;
}

