#include "pictureuploader.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QFile>
#include <QDebug>
#include <QImage>
#include <QSettings>

PictureUploader::PictureUploader(QObject *parent) : QObject(parent)
{
    qDebug() << "PictureUploader::PictureUploader() initialized";
}

bool PictureUploader::uploadFile(QString _localPath)
{
    this->destFilePath = _localPath;

    QSettings settings;

    this->authName = settings.value("UPLOAD_SERVICE_AUTH_USER").toString();
    this->authPass = settings.value("UPLOAD_SERVICE_AUTH_PASS").toString();
    this->uploaderURL = settings.value("UPLOAD_SERVICE_URL").toString();

    if(!this->uploaderURL.startsWith("http", Qt::CaseInsensitive)) {
        emit errorOccured("Wrong upload server URL");
        return false;
    }

    this->destFilePath = _localPath.replace("file://", "");
    qDebug() << "uploadFile() -> " << this->destFilePath;
    qDebug() << "uploadFile() -> " << this->uploaderURL;

    this->fileInputTagName = "myfile";
    this->boundary = "12345678901234567890abcde";
    return this->processFile();
}

bool PictureUploader::processFile(){

    QByteArray postData;
    postData = buildUploadString();

    if(postData.size() == 0) {
        return false;
    }

    QUrl mResultsURL = QUrl(this->uploaderURL);
    QNetworkAccessManager* mNetworkManager = new QNetworkAccessManager(this);

    QNetworkRequest request(mResultsURL);
    request.setRawHeader(QString("Content-Type").toAscii(),QString("multipart/form-data; boundary=" + this->boundary).toAscii());
    request.setRawHeader(QString("Content-Length").toAscii(), QString::number(postData.length()).toAscii());

    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(printScriptReply(QNetworkReply*)));
    connect(mNetworkManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(onAuthenticationRequestSlot(QNetworkReply*,QAuthenticator*)));

    QNetworkReply* reply = mNetworkManager->post(request, postData);
    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgress(qint64, qint64)));

    return true;
}

void PictureUploader::onAuthenticationRequestSlot(QNetworkReply *_reply, QAuthenticator *_authenticator)
{
    Q_UNUSED(_reply);    

    qDebug() << "Authentication is required. Use: " << this->authName << ":" << this->authPass;
    _authenticator->setUser(this->authName);
    _authenticator->setPassword(this->authPass);
}

QByteArray PictureUploader::buildUploadString()
{
    QByteArray data(QString("--" + this->boundary + "\r\n").toAscii());
    data.append(QString("--" + this->boundary + "\r\n").toAscii());
    data.append("Content-Disposition: form-data; name=\"" + this->fileInputTagName + "\"; filename=\"");
    data.append("image.jpg");
    data.append("\"\r\n");
    data.append("Content-Type: image/jpeg\r\n\r\n");

    QFile file(this->destFilePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "QFile Error: File not found!";
        emit errorOccured("Could not open selected file");
        return QByteArray();
    } else {
        qDebug() << "Orig file found, proceed as planned";
    }

    file.close();

    // Resize image
    QImage img1(this->destFilePath);
    QImage img2 = img1.scaled(1024, 1024, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    img2.save("/tmp/tmp_cryptmee_img.jpg", "JPG", 80);

    QFile file2("/tmp/tmp_cryptmee_img.jpg");

    if (!file2.open(QIODevice::ReadOnly)) {
        qDebug() << "QFile Error: Resized file not found!";
        emit errorOccured("Could not open resized file");
        return QByteArray();
    } else {
        qDebug() << "Resized file found, proceed as planned";
    }

    data.append(file2.readAll());
    data.append("\r\n");
    data.append("--" + this->boundary + "--\r\n");

    file2.close();
    file2.remove();
    return data;
}

void PictureUploader::printScriptReply(QNetworkReply* _reply)
{
    if(_reply->error() == QNetworkReply::NoError) {
        qDebug() << "Header: " << _reply->rawHeaderList();
        qDebug() << "File URL: " << _reply->rawHeader("Download-url");
        qDebug() << "Max size: " << _reply->rawHeader("Download-maxsize");
        emit uploadCompleted(QString(_reply->rawHeader("Download-url")));
    } else {
        qDebug() << "Reply error: " << _reply->errorString();
        qDebug() << "Max size: " << _reply->rawHeader("Download-maxsize");
        emit errorOccured(_reply->errorString());
    }
}

void PictureUploader::uploadProgress(qint64 _sent, qint64 _total)
{
    qDebug() << "uploadProgress(): " << _sent << " bytes of " << _total;
    emit uploadStep(_sent/1000, _total/1000);
}

