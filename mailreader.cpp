#include <QDebug>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QTextCodec>
#include <QtAlgorithms>

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

    // Create tmp dir
    QDir tmpDir(TMP_DIR);
    if(!tmpDir.exists()) {
        tmpDir.mkpath(TMP_DIR);
    }

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

    retVal = this->allMails.at(_i)->from + "###" + this->allMails.at(_i)->subject;
    retVal += QString("###") + this->allMails.at(_i)->dateTime.toString();

    if(this->allMails.at(_i)->isSendedMail)
        retVal += QString("###SEND");
    else
        retVal += QString("###REC");

    return retVal;
}

int MailReader::getNumOfMails()
{
    return this->allMails.size();
}

QString MailReader::getContent(int _i)
{
    QString mail = this->allMails.at(_i)->getEncryptedContent();

    if(mail.contains("<pre class=\"k9mail\">")) {
        // Stupid send method from K9 mail.
        // If you send an attachment, K9 covers it in HTML.
        // No idea, if we can encrypt it. Just try.
        mail.replace(QRegExp("<br\\s*/>"), "\n");
        mail.replace("<pre class=\"k9mail\">", "");
        mail.replace("</pre>", "");
    }

    return mail;
}

QString MailReader::getHeader(int _i)
{
    return this->allMails.at(_i)->getShortHeader();
}

void MailReader::sortListByDate()
{
    qDebug() << "MailReader::sortListByDate()";
    qSort(this->allMails.begin(), this->allMails.end(), lessThenComperator);
}

bool MailReader::lessThenComperator(MailObject* _left, MailObject* _right)
{
    //qDebug() << "MailReader::lessThenComperator(): " << _left->dateTime.toMSecsSinceEpoch() ;
    return _left->dateTime.toMSecsSinceEpoch() > _right->dateTime.toMSecsSinceEpoch();
}

int MailReader::getAttachmentListCount()
{
    int cnt = this->attachments.size();
    qDebug() << "MailReader::getAttachmentListCount(): " << cnt;
    return cnt;
}

QString MailReader::getAttachment(int _i)
{
    QString retVal = this->attachments.at(_i);
    qDebug() << "MailReader::getAttachment(" << _i << "): " << retVal;
    return retVal;
}

QString MailReader::parseMIMEMultiparts(QString _content)
{
    qDebug() << "MailReader::parseMIMEMultiparts()";

    // Check for end line characters. RFC2822 requires CRLF, but after
    // decryption they can get lost...
    QStringList lines;

    if(_content.contains("\r\n"))
        lines = _content.split("\r\n");
    else
        lines = _content.split("\n");

    qDebug() << "MailReader::parseMIMEMultiparts(): #lines: " << lines.size();

    QString boundary;
    QString currentLine;
    QString tmp;
    int lastHeaderLine = 0;

    // read header
    for(int i=0; i<lines.size(); i++) {
        currentLine = lines.at(i);

        if(currentLine == "" && i > 1) {
            // Header ends here
            qDebug() << "MailReader::parseMIMEMultiparts(): Header ends at line " << i;
            lastHeaderLine = i;
            break;
        }

        // Check for line wrap...
        if(i < lines.size()-1 && lines.at(i+1).startsWith(" ")) {
            currentLine += lines.at(i+1);
            i++;
        }

        if(currentLine.contains(QRegExp("Content-Type:\\s+multipart/"))) {
            qDebug() << "MailReader::parseMIMEMultiparts(): Parsing another multipart...";

            // multipart content
            boundary = currentLine;
            boundary.replace(QRegExp("Content-Type: multipart/[^;]+;\\s*boundary=\"([^\"]*)\".*"), "\\1");

            QStringList mimeParts = _content.split(QRegExp("[^\"][-]*" + boundary));
            mimeParts.removeAt(0);

            qDebug() << "MailReader::parseMIMEMultiparts() - ist GPG/MIME. ==> Boundary: " << boundary << ". Number parts: " << mimeParts.size();

            for(int i=0; i<mimeParts.size(); i++) {
                tmp = mimeParts[i];
                tmp = this->parseMIMEObject(tmp);
                mimeParts[i] = tmp;
            }

            qDebug() << "MailReader::parseMIMEMultiparts() - ist GPG/MIME. ==> End of multipart ";

            // If the last one ist just the boundary finish: remove
            if(mimeParts.at(mimeParts.size()-1).contains(QRegExp("[-]*")))
                mimeParts.removeAt(mimeParts.size()-1);

            return mimeParts.join("\n------\n");
        }
    }

    qDebug() << "MailReader::parseMIMEMultiparts() - ERROR: Something went wrong. Could not separate MIME multiparts";
    return _content;

}

QString MailReader::parseMIMEObject(QString _part)
{
    qDebug() << "MailReader::parseMIMEObject()";

    // Check for end line characters. RFC2822 requires CRLF, but after
    // decryption they can get lost...
    QStringList lines;

    if(_part.contains("\r\n"))
        lines = _part.split("\r\n");
    else
        lines = _part.split("\n");

    qDebug() << "MailReader::parseMIMEObject(): #lines: " << lines.size();

    QString contentType, name, description, disposition, filename, transferEncoding;
    QString currentLine;
    QString tmp;
    int lastHeaderLine = 0;
    QString message = "";
    QString charset = "";

    int type = 0; // 0: text 1: image 2: else

    // read header
    for(int i=0; i<lines.size(); i++) {
        currentLine = lines.at(i);        

        if(currentLine == "" && i > 1) {
            // Header ends here
            qDebug() << "MailReader::parseMIMEObject(): Header ends at line " << i;
            lastHeaderLine = i;
            break;
        }

        // Check for line wrap...
        if(i < lines.size()-1 && lines.at(i+1).startsWith(" ")) {
            currentLine += lines.at(i+1);
            i++;
        }

        qDebug() << "MailReader::parseMIMEObject(): HEADER: " << currentLine;

        if(currentLine.startsWith(("Content-Type: "))) {
            if(currentLine.contains(QRegExp("Content-Type:\\s+multipart/"))) {
                // This is another multipart! Call recursive
                return this->parseMIMEMultiparts(_part);

            } else {

                // Read content type
                contentType = currentLine;
                contentType.replace(QRegExp("Content-Type: ([^;]*)\\s*[;]*\\s*([^=]*)[=]*[\"]*([^\"]*)[\"]*"), "\\1|\\2|\\3");

                tmp = contentType.split("|").at(0);

                // Check type
                if(tmp.contains("text/", Qt::CaseInsensitive)) {
                    type = 0;

                    if(contentType.split("|").size() != 3) {
                        qDebug() << "MailReader::parseMIMEObject(): [TXT] could not get all contentType Data!";

                    } else {
                        if( contentType.split("|").at(1).contains("charset", Qt::CaseInsensitive)) {
                            charset = contentType.split("|").at(2);
                            charset.replace("=", "");
                        }
                    }


                } else if(tmp.contains("image/", Qt::CaseInsensitive)) {
                    type = 1;

                    if(contentType.split("|").size() != 3) {
                        qDebug() << "MailReader::parseMIMEObject(): [IMG] could not get all contentType Data!";

                    } else {
                        if( contentType.split("|").at(1).contains("name", Qt::CaseInsensitive)) {
                            name = contentType.split("|").at(2);
                            name.replace("=", "");
                            name.replace("\"", "");
                            name.replace(" ", "_");
                        }
                    }

                    if(name == "")
                        name = tmp.replace("/", ".");

                } else {
                    type = 2;

                    if(contentType.split("|").size() != 3) {
                        qDebug() << "MailReader::parseMIMEObject(): [FILE] could not get all contentType Data!";

                    } else {
                        if( contentType.split("|").at(1).contains("name", Qt::CaseInsensitive)) {
                            name = contentType.split("|").at(2);
                            name.replace("=", "");
                            name.replace("\"", "");
                            name.replace(" ", "_");
                        }
                    }

                    if(name == "")
                        name = tmp.replace("/", ".");
                }
            }

        } else if(currentLine.startsWith(("Content-Description: "))) {
            // Read description
            description = currentLine;
            description.replace(QRegExp("Content-Description: (.*)"), "\\1");

        } else if(currentLine.startsWith(("Content-Disposition: "))) {
            // Read disposition
            disposition = currentLine;
            disposition.replace(QRegExp("Content-Disposition: (.*)"), "\\1");

            if(disposition.contains("attachment", Qt::CaseInsensitive) || disposition.contains("inline", Qt::CaseInsensitive)) {
                if(disposition.split("filename=", QString::KeepEmptyParts, Qt::CaseInsensitive).size() == 2)
                    filename = disposition.split("filename=", QString::KeepEmptyParts, Qt::CaseInsensitive).at(1);

            }

        } else if(currentLine.startsWith(("Content-Transfer-Encoding: "))) {
            // Read transfer encoding
            transferEncoding = currentLine;
            transferEncoding.replace(QRegExp("Content-Transfer-Encoding: (.*)"), "\\1");

        } else {
            // Nothing to do - unknow header value
        }
    }

    // read content
    for(int i=lastHeaderLine; i<lines.size(); i++) {
        message += (lines.at(i) + "\n");
    }

    // Fallback to default charset
    if(charset == "")
        charset = "utf-8";

    if(transferEncoding.contains("quoted-printable", Qt::CaseInsensitive))
        return quotedPrintableDecode(message, charset);
    else if(transferEncoding.contains("base64", Qt::CaseInsensitive)) {
        QString saveAsName = filename;
        if(saveAsName == "")
            saveAsName = name;

        return this->decodeBASE64(saveAsName.replace("\"", ""), message);
    } else
        return message;
}

QString MailReader::decodeBASE64(QString _filename, QString _mail, QString _codec)
{
    qDebug() << "MailReader::decodeBASE64(" << _filename << ", BASE64)";

    if(_filename != "") {
        if(this->attachments.contains(_filename)) {
            qDebug() << "MailReader::decodeBASE64(): already got this file. Ignore.";
            return _filename;
        }
        _mail.replace("\n", "");
        _mail.replace("\r", "");
    }

    QByteArray ba;
    ba.append(_mail);
    QByteArray fileData = QByteArray::fromBase64(ba);

    if(_filename != "") {
        QFile file(TMP_DIR + _filename);
        if (!file.open(QIODevice::WriteOnly))
        {
            return "ERROR";
        }

        this->attachments << _filename;
        file.write(fileData);
        file.close();

        return _filename;
    } else {
        // No filename means, this ist just a text part. TODO: make it clear!
        QString retVal;

        QByteArray byteArray = _codec.toUtf8();
        QTextCodec *codec = QTextCodec::codecForName(byteArray.constData());
        if(codec != 0) {
            return codec->toUnicode(fileData);
        }

        // Default is utf-8
        retVal = retVal.fromUtf8(fileData.data());
        return retVal;
    }
}

void MailReader::cleanUpAttachments()
{
    qDebug() << "MailReader::cleanUpAttachments()";
    this->attachments.clear();

    QDir dir(TMP_DIR);
    dir.setNameFilters(QStringList() << "*");
    dir.setFilter(QDir::Files);

    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }
}

QString MailReader::parseMailContent(QString _mail)
{
    // parse content. Split MIME objects, separate objects etc...
    this->cleanUpAttachments();

    if(_mail.contains("Content-Type: multipart/")) {
        qDebug() << "MailReader::parseMailContent(): Parsing MIME email...";

        return parseMIMEMultiparts(_mail);

    } else {
        qDebug() << "MailReader::parseMailContent(): Parsing INLINE email...";

        if(_mail.contains("Content-Transfer-Encoding: quoted-printable")) {
            // Remove quoted printable
            _mail.replace(QRegExp("Content-Transfer-Encoding: quoted-printable\\s*"), "");
            _mail = this->quotedPrintableDecode(_mail);
        }        
    }

    // Nothing to do for now for Inline mails:
    return _mail;
}

QString MailReader::encodeMIMEString(QString _input)
{
    // See: RFC 2047 http://tools.ietf.org/html/rfc2047.html
    // Some header chars could be encoded with Message Header Extensions for Non-ASCII Text
    // e.g.: =?ISO-8859-1 ?Q?Gr=FCsse?=
    // =? <charset> ? <encoding> ? <characters> ?=

    // QTextCodec *codec = QTextCodec::codecForName("ISO 8859-1");
    // QByteArray encodedString = codec->fromUnicode(name);

    if(_input.contains(QRegExp("=\\?[^\\?]+\\?[qQbB]+\\?.*\\?="))) {
        _input.replace(QRegExp("^(.*)=\\?([^\\?]+)\\?([qQbB]+)\\?(.*)\\?=(.*)"), "\\1###\\2###\\3###\\4###\\5");
        qDebug() << "MailReader::encodeMIMEString(): found RFC2047 string: " << _input;

        QString contentPre = _input.split("###").at(0);
        QString contentPost = _input.split("###").at(4);
        QString content = _input.split("###").at(3);
        QString charset = _input.split("###").at(1);
        QString encoding = _input.split("###").at(2);

        if(encoding == "q" || encoding == "Q") {
            // Encode quoted-printable
            _input = contentPre + this->quotedPrintableDecode(content, charset) + contentPost;
        } else {
            // Encode BASE64
            _input = contentPre + this->decodeBASE64("", content, charset) + contentPost;
        }
    }

    return _input;
}

QString MailReader::quotedPrintableDecode(QString &input, QString _codec)
{
    qDebug() << "MailReader::quotedPrintableDecode([DATA], " << _codec << ")";

    // Code from: http://www.softwareace.cn/?p=494
    // 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F
    const int hexVal[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

    QByteArray *output = new QByteArray();

    input.replace("=\n", "\n");

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

    if(_codec == "")
        return QTextCodec::codecForMib(106)->toUnicode(*output);
    else {
        QByteArray byteArray = _codec.toUtf8();
        QTextCodec *codec = QTextCodec::codecForName(byteArray.constData());
        if(codec != 0) {
            return codec->toUnicode(*output);
        }
    }

    return input;
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


