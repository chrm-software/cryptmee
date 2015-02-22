#include "mailobject.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

MailObject::MailObject(QString _path, MailReader* _parent)
{
    this->fileName = _path;
    this->myMailreader = _parent;

    this->from = this->to = this->subject = "";
    this->isQuotedPrintable = false;
    this->encryptedContent = "";

    this->isSendedMail = false;

    qDebug() << "MailObject::MailObject: parsing " << _path;
    this->parseMail();
}

QString MailObject::tr(const char* _txt)
{
    return this->myMailreader->translateText(_txt);
}

void MailObject::parseMail()
{
    // Open file and read data
    QString fileToOpen = this->fileName;
    QString fileMailMultipartToOpen = "";

    if(this->fileName.contains("-part"))
    {
        qDebug() << "MailObject::parseMail(): found MIME multipart mail - open main file.";
        fileMailMultipartToOpen = this->fileName.replace(QRegExp("-part.*$"), "");

    }

    // Read complete mail, no multipart
    QFile file(fileToOpen);

    if (file.open(QIODevice::ReadOnly)) {
        QString line;
        QTextStream in(&file);

        if(fileMailMultipartToOpen == "") {
            // Inline mail - read content
            // Read header first
            while (!in.atEnd()) {
                line = in.readLine();

                if (line == "") {
                    break;
                }

                this->completeHeader << line;
            }

            // Read content of inline mail
            while (!in.atEnd()) {
                line = in.readLine();
                this->encryptedContent += "\n"+line;
            }

            file.close();

        } else {
            // Multipart mail.
            // Reading just content of MIME mail
            while (!in.atEnd()) {
                line = in.readLine();
                this->encryptedContent += "\n"+line;
            }
            file.close();
        }
    }

    if(fileMailMultipartToOpen != "") {
        // Read header from main MIME file

        QFile file(fileMailMultipartToOpen);

        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);

            while (!in.atEnd()) {
                line = in.readLine();

                if (line == "") {
                    break;
                }

                this->completeHeader << line;
            }

            file.close();
        }
    }


    // Parse header
    for(int i=0; i < this->completeHeader.size(); i++) {

        if(this->completeHeader[i].startsWith("From: ")) {
            this->from = this->completeHeader[i];
            this->from.replace("From: ", "");
            this->from.replace("\"", "");
            this->from.replace("<", "&lt;");
        }
        if(this->completeHeader[i].startsWith("To: ")) {
            this->to = this->completeHeader[i];
            this->to.replace("To: ", "");
        }
        if(this->completeHeader[i].startsWith("Subject: ")) {
            this->subject = this->completeHeader[i];
            this->subject.replace("Subject: ", "");
        }
        if(this->completeHeader[i].startsWith("Date: ")) {
            this->dateString = this->completeHeader[i];
            this->parseDate();
        }
        if(this->completeHeader[i].startsWith("Cc: ")) {
            this->cc = this->completeHeader[i];
            this->cc.replace("Cc: ", "");
        }

        if(this->completeHeader[i].contains("quoted-printable")) {
            qDebug() << "MailObject::parseMail(): quted-printable content, should be replaced!";
            this->isQuotedPrintable = true;
        }
    }

    // All header fields could also conatins encoded characters
    if(this->subject.contains("=?")) {
        this->subject = this->myMailreader->encodeMIMEString(this->subject);
    }

    if(this->from.contains("=?")) {
        this->from = this->myMailreader->encodeMIMEString(this->from);
    }
}

QString MailObject::getEncryptedContent()
{
    // Check quoted-printable first before passing to gpg!
    if(this->isQuotedPrintable) {
        this->encryptedContent = this->encryptedContent.replace("=3D", "=");
    }
    return this->encryptedContent.toUtf8();
}

QString MailObject::getShortHeader()
{
    qDebug() << "MailObject::getShortHeader()";

    if(this->isSendedMail)
        return QString(("<b><font size='+1'>")) + this->from + ("</font></b> (sent)") + "\n<br>" +
                ("<b> ") + this->subject + "</b><br>" +
                ("") + this->to + " " + this->cc + "\n<br>" +
                ("<font color='#006ea7'>") + this->dateTime.toString() + "</font>\n<br>";
    else
        return QString(("<b><font size='+1'>")) + this->from + ("</font></b>") + "\n<br>" +
                ("<b> ") + this->subject + "</b><br>" +
                ("") + this->to + " " + this->cc + "\n<br>" +
                ("<font color='#006ea7'>") + this->dateTime.toString() + "</font>\n<br>";
}

QString MailObject::getCompleteHeader()
{
    return this->completeHeader.join("\n");
}

QString MailObject::getDecryptedContent()
{
    return "DECRYPTED!";
}

bool MailObject::operator<(const MailObject * object) {

    qDebug() << "SORT: 1:" << this->dateTime.toMSecsSinceEpoch() << " 2: " << object->dateTime.toMSecsSinceEpoch();


    bool retVal = this->dateTime.toMSecsSinceEpoch() < object->dateTime.toMSecsSinceEpoch();

    qDebug() << "SORT: 1<2: " << retVal;

    return retVal;
}

bool MailObject::operator>(const MailObject * object) {

    qDebug() << "SORT: 1:" << this->dateTime.toMSecsSinceEpoch() << " 2: " << object->dateTime.toMSecsSinceEpoch();


    bool retVal = this->dateTime.toMSecsSinceEpoch() > object->dateTime.toMSecsSinceEpoch();

    qDebug() << "SORT: 1>2: " << retVal;

    return retVal;
}


bool MailObject::parseDate()
{
    // Parse Date
    QString dayremoved;
    QStringList datelist;
    dayremoved= this->dateString.section(",",1,1);
    datelist = dayremoved.split(" ");

    // Mail date is always en_US locale
    QStringList months;
    months << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";

    if(datelist.size() > 4) {
        QString day = datelist[1];
        QString month = datelist[2];
        QString year = datelist[3];
        QString time = datelist[4];

        // Use localized month name
        month = QDate::shortMonthName(months.indexOf(month) + 1);

        QString tmpDate = day + "-" + month + "-" + year + "-" + time;

        if(day.startsWith("0"))
            this->dateTime = QDateTime::fromString(tmpDate,"dd-MMM-yyyy-hh:mm:ss");
        else
            this->dateTime = QDateTime::fromString(tmpDate,"d-MMM-yyyy-hh:mm:ss");

        if(!this->dateTime.isValid()) {
            qDebug() << "MailObject::parseMail(): Unable to parse date  : [" << tmpDate << "]";
        } else {
            qDebug() << "MailObject::parseMail(): Successfull parse date: [" << tmpDate << "]";
        }
    }

    return true;
}
