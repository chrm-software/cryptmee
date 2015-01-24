#ifndef MAILREADERTHREAD_H
#define MAILREADERTHREAD_H

#include <QThread>
#include <QStringList>

class MailReaderThread : public QThread
{
    Q_OBJECT
public:
    explicit MailReaderThread(QString _path, QObject *parent);
    QStringList getAllMails();
    void run();
    int fromLastDays;
    
private:

    QStringList allPGPMails;
    QString path;

signals:
    void readyThread();
    void errorOccuredThread();
    
};

#endif // MAILREADERTHREAD_H
