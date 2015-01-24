#include <QDebug>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include "mailreader.h"
#include "configuration.h"

MailReader::MailReader(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    QString path = MAIL_PATH;
    this->mailDB = MAIL_DB;

    this->myThread = new MailReaderThread(path, this);
    connect(this->myThread, SIGNAL(readyThread()), this, SLOT(threadReady()));
    connect(this->myThread, SIGNAL(errorOccuredThread()), this, SLOT(threadReady()));

    qDebug() << "MailReader::MailReader: LOCALE: " << QLocale::system().country();
}

QString MailReader::translateText(const char* _txt) {
    // Helper function for non-QObject classes
    return tr(_txt);
}

void MailReader::threadReady()
{
    QStringList allPGPMails = this->myThread->getAllMails();

    // Clear List
    for(int i=0; i<this->allMails.size(); i++) {
        delete this->allMails.at(i);
    }

    this->allMails.clear();

    for(int i=0; i<allPGPMails.size(); i++) {
        MailObject* mo = new MailObject(allPGPMails[i], this);
        this->allMails << mo;
    }

    this->sortListByDate();

    // DEBUG;
    this->readMailDB();

    emit ready();
}

void MailReader::readMails(int _fromLastDays)
{
    qDebug() << "MailReader::readMail()";

    this->myThread->fromLastDays = _fromLastDays;
    this->myThread->start();
}

QString MailReader::getMail(int _i)
{
    QString retVal;

    retVal = this->allMails.at(_i)->from + "|" + this->allMails.at(_i)->subject;
    retVal += QString("|") + this->allMails.at(_i)->dateTime.toString();

    if(this->allMails.at(_i)->isSendedMail)
        retVal += QString("|SEND");
    else
        retVal += QString("|REC");

    return retVal;
}

int MailReader::getNumOfMails()
{
    return this->allMails.size();
}

QString MailReader::getContent(int _i)
{
    return this->allMails.at(_i)->getEncryptedContent();
}

QString MailReader::getHeader(int _i)
{
    return this->allMails.at(_i)->getShortHeader();
}

void MailReader::sortListByDate()
{
    qSort(this->allMails.begin(), this->allMails.end(), qGreater<MailObject*>());
}

QString MailReader::parseMailContent(QString _mail)
{
    // parse content. Split MIME objects, separate objects etc...

    if(_mail.contains("Content-Type: multipart/mixed")) {
        // multipart content
        QString boundary = _mail;
        boundary.replace(QRegExp("Content-Type: multipart/mixed;\\s*boundary=\"([^\"]*)\".*"), "\\1");

        QStringList mimeParts = _mail.split(QRegExp("[^\"][-]*" + boundary));
        mimeParts.removeAt(0);
        QString tmp;

        qDebug() << "MailReader::parseMailContent() - ist GPG/MIME. Boundary: " << boundary;

        for(int i=0; i<mimeParts.size(); i++) {
            tmp = mimeParts[i];

            if(tmp.contains("Content-Type: text/")) {
                tmp.replace(QRegExp("Content-Type: text/.*\n(.*)"), "\\1");

                if(tmp.contains("Content-Transfer-Encoding: quoted-printable")) {
                    // Hast quoted-printable. Replace first
                    tmp.replace(QRegExp("Content-Transfer-Encoding: quoted-printable\\s*"), "");
                    tmp = this->quotedPrintableDecode(tmp);
                }
            } else if(tmp.contains("Content-Type: image/")) {
                tmp = "[IMAGE] " + tmp;

            } else if(tmp.contains("Content-Type: application/")) {
                tmp = "[APPLICATION] " + tmp;
            }

            mimeParts[i] = tmp;
        }

        // If the last one ist just the boundary finish: remove
        if(mimeParts.at(mimeParts.size()-1).contains(QRegExp("[-]*")))
            mimeParts.removeAt(mimeParts.size()-1);

        return mimeParts.join("\n ---------------------- \n");

    } else {
        if(_mail.contains("Content-Transfer-Encoding: quoted-printable")) {
            // Remove quoted printable
            _mail.replace(QRegExp("Content-Transfer-Encoding: quoted-printable\\s*"), "");
            _mail = this->quotedPrintableDecode(_mail);
        }
    }

    // Nothing to do for now for Inline mails:
    return _mail;
}

QByteArray& MailReader::quotedPrintableDecode(QString &input)
{
    // Code from: http://www.softwareace.cn/?p=494
    // 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F
    const int hexVal[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

    QByteArray *output = new QByteArray();

    for (int i = 0; i < input.length(); ++i)
    {
        if (input.at(i).toAscii() == '=')
        {
            output->append((hexVal[input.at(++i).toAscii() - '0'] << 4) + hexVal[input.at(++i).toAscii() - '0']);
        }
        else
        {
            output->append(input.at(i).toAscii());
        }
    }
    return *output;
}

bool MailReader::readMailDB()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(this->mailDB);
    if(db.open()) {
        qDebug() << "MailReader::readMailDB(): DB opened successfull.";
        QSqlQuery query;
        if(query.exec("SELECT stamp,subject,sender FROM mailmessages WHERE receivedstamp is null")) {

            QString date;
            QString subject;
            QString sender;

            while (query.next()) {
                //0: date; 1: Subject; 2: Sender
                date = query.value(0).toString();
                subject = query.value(1).toString();
                sender = query.value(2).toString();

                qDebug() << "MailReader::readMailDB(): Subject from DB: " << subject << ", Sender: " << sender;

                // Find mail in List
                for(int i=0; i<this->allMails.size(); i++) {
                    if(this->allMails.at(i)->from == sender) {
                        if(this->allMails.at(i)->subject == subject) {
                            qDebug() << "MailReader::readMailDB(): Found Mail! Add as sended at: " << i;

                            // TODO: Check also date!
                            this->allMails.at(i)->isSendedMail = true;
                            continue;
                        }
                    }
                }
            }

            db.close();
            return true;

        } else {
            qDebug() << "MailReader::readMailDB(): *** Could not exec query: " << query.lastError().text();
            db.close();
            return false;
        }
    } else {
        qDebug() << "MailReader::readMailDB(): *** Error: could not open DB!" << db.lastError().text();
        return false;
    }
}


