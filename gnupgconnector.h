#ifndef GNUPGCONNECTOR_H
#define GNUPGCONNECTOR_H

#include <QDeclarativeItem>
#include <QFile>
#include <QClipboard>
#include <QProcess>
#include <QVariantMap>

#include "keyreader.h"
#include "configuration.h"

#define GPG_IDLE 0
#define GPG_DECRYPT 1
#define GPG_ENCRYPT 2
#define GPG_KEYS 3
#define GPG_KEYS_GEN 4
#define GPG_SET_TRUST 5
#define GPG_SIGN 6
#define GPG_GET_VERSION 7
#define GPG_IMPORT 8
#define GPG_EXPORT 9
#define GPG_SEARCH 10
#define GPG_DELETE 11
#define GPG_SECRET_KEYS 12



class GnuPGConnector : public QDeclarativeItem
{
    Q_OBJECT
    //Q_PROPERTY(QStringList allPubKeys READ getKeys)

public:
    explicit GnuPGConnector(QDeclarativeItem *parent = 0);

    Q_INVOKABLE QString encrypt(QString _input, QString _recipient);
    Q_INVOKABLE QString decrypt(QString _input, QString _passphrase);
    Q_INVOKABLE QString showKeys();
    Q_INVOKABLE QString showSecretKeys();
    Q_INVOKABLE QString getData(bool _errors);
    Q_INVOKABLE QString getFromClipboard();
    Q_INVOKABLE void setToClipboard(QString _data);

    Q_INVOKABLE QString getKey(int _i, int _type = 0);
    Q_INVOKABLE QString getKeyByID(QString _id);
    Q_INVOKABLE QString getPrivateKeyIDs();

    Q_INVOKABLE int getNumOfPubKeys(int _type = 0);
    Q_INVOKABLE bool generateKeyPair(QString _name, QString _comment, QString _email, QString _passphrase);
    Q_INVOKABLE bool setOwnerTrust(QString _id, QString _trustLevel);
    Q_INVOKABLE bool checkGPGVersion(QString _path);
    Q_INVOKABLE QString getGPGVersionString();

    Q_INVOKABLE bool importKeysFromFile(QString _path);
    Q_INVOKABLE bool importKeysFromClipboard();
    Q_INVOKABLE bool searchKeysOnKeyserver(QString _keyword);
    Q_INVOKABLE bool importKeysFromKeyserver(QString _keys);
    Q_INVOKABLE bool deleteKey(QString _id);

    Q_INVOKABLE QString getHistory();
    Q_INVOKABLE bool saveHistory(QString _filename);

    // Settings interface
    Q_INVOKABLE void settingsSetValue(QString _key, QString _value);
    Q_INVOKABLE QString settingsGetValue(QString _key);
    Q_INVOKABLE void settingsReset();


private:
    QProcess* process_gpg;
    QString currentPassword;
    bool processIsRunning;
    QString gpgVersionString;

    QString gpgStdOutput;
    QString gpgErrOutput;

    QStringList gpgHistory;

    int errorCode;
    unsigned int currentState;

    bool writeToTmpFile(QString _content);
    QString readFromTmpFile(int _type);


    bool callGnuPG(QString _cmd, int _state);

    KeyReader* myKeyReader;    

    // Dynamic values - always synced with settings
    QString gpgBinaryPath;
    QString gpgKeyserverURL;
    QString localMailPath;
    QString localMailDB;
    QString useOwnKey;

private slots:
    void gpgFinished(int _retVal);
    void gpgError(QProcess::ProcessError _pe);

signals:
    void ready();
    void errorOccured();

};

#endif // GNUPGCONNECTOR_H
