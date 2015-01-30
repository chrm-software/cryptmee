#include "mailreaderthread.h"
#include <QDirIterator>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

MailReaderThread::MailReaderThread(QString _path, QObject *parent) :
    QThread(parent)
{
    this->path = _path;
    this->fromLastDays = 31;
}

void MailReaderThread::run()
{
    QDirIterator it(this->path, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
    this->allPGPMails.clear();

    int lineCnt = 0;

    while (it.hasNext()) {
        QFile file(it.next());

        // Check only newer files
        if(it.fileInfo().created().daysTo(QDateTime::currentDateTime()) > this->fromLastDays) {
            qDebug() << "MailReaderThread::run(): File too old: " << it.fileInfo().created().daysTo(QDateTime::currentDateTime()) << ". " << this->fromLastDays;
            continue;
        }

        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);
            lineCnt = 0;

            while (!in.atEnd()) {
                // Break after 500 lines
                lineCnt++;
                if(lineCnt > 500)
                    break;

                line = in.readLine();

                if (line.contains("-BEGIN PGP MESSAGE-")) {
                     this->allPGPMails << file.fileName();
                    break;
                }
            }

            file.close();

        } else {
            qDebug() << "*** Can't open file: " << file.fileName();
            emit errorOccuredThread();
        }
    }

    emit readyThread();
}

QStringList MailReaderThread::getAllMails()
{
    return this->allPGPMails;
}
