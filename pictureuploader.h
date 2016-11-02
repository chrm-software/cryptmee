#ifndef PICTUREUPLOADER_H
#define PICTUREUPLOADER_H

#include <QNetworkReply>

class PictureUploader : public QObject
{
    Q_OBJECT
public:
    explicit PictureUploader(QObject *parent = 0);
    Q_INVOKABLE bool uploadFile(QString _localPath);

private:
    QByteArray buildUploadString();
    bool processFile();
    QString destFilePath;
    QString destFileURL;
    QString authName, authPass;
    QString uploaderURL;
    QString boundary;
    QString fileInputTagName;

private slots:
    void printScriptReply(QNetworkReply* _reply);
    void onAuthenticationRequestSlot(QNetworkReply* _reply, QAuthenticator* _authenticator);
    void uploadProgress(qint64 _sent, qint64 _total);

signals:
    void uploadCompleted(QString _downloadURL);
    void errorOccured(QString _message);
    void uploadStep(int _step, int _total);
};

#endif // PICTUREUPLOADER_H
