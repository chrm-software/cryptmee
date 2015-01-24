#ifndef MAILREADER_H
#define MAILREADER_H

#include "mailobject.h"
#include "mailreaderthread.h"

#include <QDeclarativeItem>
#include <QList>
#include <QStringList>
#include <QThread>

class MailObject;

class MailReader : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit MailReader(QDeclarativeItem *parent = 0);

    Q_INVOKABLE void readMails(int _fromLastDays);
    Q_INVOKABLE QString getMail(int _i);
    Q_INVOKABLE int getNumOfMails();
    Q_INVOKABLE QString getContent(int _i);
    Q_INVOKABLE QString getHeader(int _i);
    Q_INVOKABLE QString parseMailContent(QString _mail);

    QString translateText(const char *_txt);

private:
    QList<MailObject*> allMails;
    MailReaderThread* myThread;
    QByteArray& quotedPrintableDecode(QString &input);

    void sortListByDate();
    bool readMailDB();

    QString mailDB;

private slots:
    void threadReady();

signals:
    void ready();
    void errorOccured();
};

#endif // MAILREADER_H
