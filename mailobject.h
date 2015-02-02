#ifndef MAILOBJECT_H
#define MAILOBJECT_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QObject>

#include "mailreader.h"

class MailReader;

class MailObject
{

public:
    MailObject(QString _path, MailReader *_parent);
    QString getEncryptedContent();
    QString getShortHeader();
    QString getCompleteHeader();
    QString from, to, dateString, subject;
    QDateTime dateTime;
    QString getDecryptedContent();
    bool isSendedMail;
    bool operator<(const MailObject * object);
    bool operator>(const MailObject * object);

private:
    void parseMail();
    QString tr(const char *_txt);

    QString fileName;
    QStringList completeHeader;
    QString encryptedContent;

    bool isQuotedPrintable;
    bool parseDate();

    MailReader* myMailreader;
};

#endif // MAILOBJECT_H
