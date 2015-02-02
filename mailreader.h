#ifndef MAILREADER_H
#define MAILREADER_H

#include "mailobject.h"
#include "mailreaderthread.h"

#include <QDeclarativeItem>
#include <QList>
#include <QStringList>
#include <QStringListModel>
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
    Q_INVOKABLE int getAttachmentListCount();
    Q_INVOKABLE QString getAttachment(int _i);


    QString translateText(const char *_txt);
    QString quotedPrintableDecode(QString &input, QString _codec = "");
    QString encodeMIMEString(QString _input);

private:
    QList<MailObject*> allMails;
    MailReaderThread* myThread;

    static bool lessThenComperator(MailObject* _left, MailObject* _right);
    void sortListByDate();

    bool readMailDB();
    QString parseMIMEMultiparts(QString _content);
    QString parseMIMEObject(QString _part);
    QString decodeBASE64(QString _filename, QString _mail, QString _codec = "utf-8");
    void cleanUpAttachments();

    QStringList attachments;
    QString mailDB;

private slots:
    void threadReady();

signals:
    void ready();
    void errorOccured();
};

#endif // MAILREADER_H
