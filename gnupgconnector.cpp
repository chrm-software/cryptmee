#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>

#include "gnupgconnector.h"

GnuPGConnector::GnuPGConnector(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    qDebug() << "GnuPGConnector!";

    this->myKeyReader = new KeyReader();
    this->myKeyReader->parseGnuPGOutput("");

    this->process_gpg = new QProcess(this);
    this->currentState = GPG_IDLE;

    this->gpgStdOutput = "";
    this->gpgErrOutput = "No Errors";

    connect(this->process_gpg, SIGNAL(finished(int)), this, SLOT(gpgFinished(int)));
    connect(this->process_gpg, SIGNAL(error(QProcess::ProcessError)), this, SLOT(gpgError(QProcess::ProcessError)));

    // Init Settings
    QSettings settings;
    if(!settings.contains("SETTINGS_RELEASE")) {
        // First start, fill values
        this->settingsReset();

        // TODO:
        // Do something after the first start...
    }

    //settings.remove("SETTINGS_RELEASE");
    this->gpgBinaryPath = settings.value("SETTINGS_GPGPATH", GPGBIN).toString();
    this->gpgKeyserverURL = settings.value("SETTINGS_GPGKEYSERVER", KEYSERVER).toString();
    this->localMailPath = settings.value("SETTINGS_MAILDIR", MAIL_PATH).toString();
    this->localMailDB = settings.value("SETTINGS_MAILDB", MAIL_DB).toString();

    // Initial check GnuPG version
    this->checkGPGVersion(this->gpgBinaryPath);
}

void GnuPGConnector::settingsReset()
{
    QSettings settings;
    settings.setValue("SETTINGS_RELEASE", RELEASE);
    settings.setValue("SETTINGS_GPGPATH", GPGBIN);
    settings.setValue("SETTINGS_GPGKEYSERVER", KEYSERVER);
    settings.setValue("SETTINGS_MAILDIR", MAIL_PATH);
    settings.setValue("SETTINGS_MAILDB", MAIL_DB);
}

void GnuPGConnector::settingsSetValue(QString _key, QString _value)
{
    QSettings settings;
    settings.setValue(_key, _value);

    // Update current values
    this->gpgBinaryPath = settings.value("SETTINGS_GPGPATH", GPGBIN).toString();
    this->gpgKeyserverURL = settings.value("SETTINGS_GPGKEYSERVER", KEYSERVER).toString();
    this->localMailPath = settings.value("SETTINGS_MAILDIR", MAIL_PATH).toString();
    this->localMailDB = settings.value("SETTINGS_MAILDB", MAIL_DB).toString();
}

QString GnuPGConnector::settingsGetValue(QString _key)
{
    QSettings settings;
    return settings.value(_key).toString();
}

QString GnuPGConnector::checkGPGVersion(QString _path)
{
    qDebug() << "GnuPGConnector::checkGPGVersion(" << _path << ")";

    this->errorCode = -1; // No error
    this->gpgStdOutput = "";
    this->gpgErrOutput = "No Errors";

    QString gpgIn = _path + QString(" --version");

    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_GET_VERSION;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return "";
}

bool GnuPGConnector::writeToTmpFile(QString _content)
{
    // Generate file
    QString outputFilename = TMPFILE;
    QFile outputFile(outputFilename);
    outputFile.open(QIODevice::WriteOnly);

    if(!outputFile.isOpen()){
        qDebug() << "*** Error, unable to open" << outputFilename << "for output";
        return false;
    }

    QTextStream outStream(&outputFile);
    outStream << _content;
    outputFile.close();

    return true;
}

QString GnuPGConnector::readFromTmpFile(int _type)
{
    QString outputFilename;
    if(_type == 0) {
        outputFilename = TMPFILE+".asc";
    } else {
        outputFilename = TMPFILE+".txt";
    }

    // Read from file

    QFile outputFile(outputFilename);
    outputFile.open(QIODevice::ReadOnly);

    if(!outputFile.isOpen()){
        qDebug() << "*** Error, unable to open" << outputFilename << "for input";
        return "*** Error occured ***";
    }

    QTextStream inStream(&outputFile);
    QString content = inStream.readAll();
    outputFile.close();

    qDebug() << "Content: " << content;

    return content;
}

QString GnuPGConnector::encrypt(QString _input, QString _recipient)
{
    qDebug() << "GnuPGConnector::encrypt(" << _input << ")";

    this->errorCode = -1; // No error
    this->gpgStdOutput = "";
    this->gpgErrOutput = "No Errors";

    if(!this->writeToTmpFile(_input))
        return "*** Error occured ***";

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --armor -r ") + _recipient + " -o " + TMPFILE + ".asc" + " --encrypt " + TMPFILE;

    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_ENCRYPT;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return "";
}

QString GnuPGConnector::decrypt(QString _input, QString _passphrase)
{
    qDebug() << "GnuPGConnector::decrypt(" << _input << "," << "***" << ")";

    this->errorCode = -1; // No error
    this->gpgStdOutput = "";
    this->gpgErrOutput = "No Errors";

    if(!this->writeToTmpFile(_input))
        return "*** Error occured ***";

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --passphrase \"") + _passphrase + "\" -o " + TMPFILE + ".txt" + " --decrypt " + TMPFILE;

    qDebug() << "Starting: " << QString(gpgIn).replace(_passphrase, "***");

    this->currentState = GPG_DECRYPT;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return "";
}

QString GnuPGConnector::showKeys()
{
    qDebug() << "GnuPGConnector::showKeys()";

    QString gpgIn = this->gpgBinaryPath + QString(" --charset utf-8 --display-charset utf-8 --list-public-keys --fixed-list-mode --with-colons --with-fingerprint");

    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_KEYS;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return "";
}

QString GnuPGConnector::getFromClipboard()
{
    QString clip =  QApplication::clipboard()->text(QClipboard::Clipboard);
    qDebug() << "GnuPGConnector::getFromClipboard() == " << clip;

    return clip;
}

void GnuPGConnector::setToClipboard(QString _data)
{
    qDebug() << "GnuPGConnector::setToClipboard(" << _data << ")";
    QApplication::clipboard()->setText(_data, QClipboard::Clipboard);
}

QString GnuPGConnector::getGPGVersionString()
{
    return this->gpgVersionString;
}

void GnuPGConnector::gpgFinished(int _retVal)
{    
    qDebug() << "GnuPGConnector::gpgFinished(" << _retVal << ")";
    this->processIsRunning = false;

    QString output = this->process_gpg->readAllStandardOutput().simplified();
    QString error = this->process_gpg->readAllStandardError().simplified();

    qDebug() << "GnuPGConnector::gpgFinished(): stdout: " << output;
    qDebug() << "GnuPGConnector::gpgFinished(): stderr: " << error;

    if(this->currentState == GPG_KEYS) {
        // Key listing finished: parse output
        this->myKeyReader->parseGnuPGOutput(output);

    } else if(this->currentState == GPG_KEYS_GEN) {
        // Nothing to do: just check retval

    } else if(this->currentState == GPG_GET_VERSION) {
        // Set GnuPG version
        this->gpgStdOutput = output;

        if(_retVal != 0)
            this->gpgVersionString = "-1";
        else
            this->gpgVersionString = output.left(35) + "...";

    } else if(this->currentState == GPG_ENCRYPT) {
        // Enrypted content from out file
        this->gpgStdOutput = this->readFromTmpFile(0);

    } else if(this->currentState == GPG_IMPORT) {
        // Import command output
        this->gpgStdOutput = output;

    } else if(this->currentState == GPG_SEARCH) {
        // Search results from stdout
        // TODO: parse output!
        this->gpgStdOutput = output;
        this->myKeyReader->parseGnuPGServerSearchOutput(output);

        // Hack: ignore errors
        _retVal = 0;

    } else if(this->currentState == GPG_SET_TRUST) {
        this->gpgStdOutput = output;

    } else {
        // Decrypted content form StdOut
        this->gpgStdOutput = this->readFromTmpFile(1);
    }


    // Remove tmp files
    QFile::remove(QString(TMPFILE) + ".asc");
    QFile::remove(QString(TMPFILE) + ".txt");
    QFile::remove(QString(TMPFILE));

    if(_retVal != 0) {
        this->gpgErrOutput = QString("[") + QString::number(_retVal) + "] " + error;
        emit errorOccured();
    } else {
        emit ready();
    }

    this->currentState = GPG_IDLE;
}

QString GnuPGConnector::getData(bool _errors)
{
    qDebug() << "GnuPGConnector::getData()";

    if(_errors) {
        if(this->errorCode == -1)
            return this->gpgErrOutput;
        else
            return "*** QProcess error occured ***\nCode: " + QString::number(this->errorCode);
    } else
        return this->gpgStdOutput;
}

void GnuPGConnector::gpgError(QProcess::ProcessError _pe)
{
    qDebug() << "GnuPGConnector::gpgError: " << _pe;
    this->processIsRunning = false;

    if(this->currentState == GPG_GET_VERSION)
        this->gpgVersionString = "-1";

    this->errorCode = _pe;
    this->currentState = GPG_IDLE;

    emit errorOccured();
}

QString GnuPGConnector::getKey(int _i, int _type)
{   
    return this->myKeyReader->getKeyAsHTMLString(_i, false, _type);
}

QString GnuPGConnector::getKeyByID(QString _id)
{
    if(this->myKeyReader->getKeyByID(_id) == NULL)
        return "";

    QString retVal;
    retVal = "<b>" + this->myKeyReader->getKeyByID(_id)->keyID + "</b>\n<br>";
    retVal += "<b><font color='blue'>" + this->myKeyReader->getKeyByID(_id)->fingerprint + "</font></b>\n<br>";
    retVal += this->myKeyReader->getKeyByID(_id)->date + "\n<br>";
    retVal += this->myKeyReader->getKeyByID(_id)->expires + "\n<br>";
    retVal += this->myKeyReader->getKeyByID(_id)->length + "\n<br>";
    retVal += this->myKeyReader->getKeyByID(_id)->trustValue + "\n<br>";
    retVal += (this->myKeyReader->getKeyByID(_id)->identities.join("\n")).replace("<", "&lt;").replace("\n", "\n<br>");
    return retVal;
}

int GnuPGConnector::getNumOfPubKeys(int _type)
{
    return this->myKeyReader->getNumOfKeys(_type);
}

bool GnuPGConnector::generateKeyPair(QString _name, QString _comment, QString _email, QString _passphrase)
{
    if(_name == "" || _email == "" || _passphrase == "")
        return false;

    // Generate file first
    QString genKeyFile;

    genKeyFile = "Key-Type: DSA\nKey-Length: 2048\nSubkey-Type: ELG-E\nSubkey-Length: 2048\nName-Real: ";
    genKeyFile += _name;
    genKeyFile += "\nName-Comment: " + _comment;
    genKeyFile += "\nName-Email: " + _email;
    genKeyFile += "\nExpire-Date: 0\nPassphrase: " + _passphrase;
    genKeyFile += "\n%commit";

    this->writeToTmpFile(genKeyFile);

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --gen-key ") + TMPFILE;
    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_KEYS_GEN;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return true;
}

bool GnuPGConnector::setOwnerTrust(QString _id, QString _trustLevel)
{
    qDebug() << "GnuPGConnector::setOwnerTrust(): ID: " << _id;

    if(_id == "" || _trustLevel == "")
        return false;

    if(this->myKeyReader->getKeyByID(_id) == NULL)
        return "";

    // Generate file first
    QString genTrustFile;

    genTrustFile = this->myKeyReader->getKeyByID(_id)->fingerprint + ":" + _trustLevel + ":\n";

    this->writeToTmpFile(genTrustFile);

    QString gpgIn = this->gpgBinaryPath + QString(" --import-ownertrust ") + TMPFILE;
    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_SET_TRUST;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return true;
}

bool GnuPGConnector::importKeysFromFile(QString _path)
{
    // Expand with prefix path MYDOCS_PATH

    qDebug() << "GnuPGConnector::importKeysFromFile(" << _path << ")";

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --import ") + MYDOCS_PATH +_path;

    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_IMPORT;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return true;
}

bool GnuPGConnector::importKeysFromClipboard()
{
    QString clipBoard = this->getFromClipboard();

    qDebug() << "GnuPGConnector::importKeysFromClipboard()";

    // Write to file
    this->writeToTmpFile(clipBoard);
    QString gpgIn = this->gpgBinaryPath + QString(" --batch --import ") + TMPFILE;

    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_IMPORT;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return true;
}

bool GnuPGConnector::searchKeysOnKeyserver(QString _keyword)
{
    qDebug() << "GnuPGConnector::searchKeysOnKeyserver(" << _keyword << ")";

    if(_keyword == "")
        return false;

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --keyserver " + this->gpgKeyserverURL + " --search-keys ") + _keyword;

    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_SEARCH;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return true;
}

bool GnuPGConnector::importKeysFromKeyserver(QString _keys)
{
    qDebug() << "GnuPGConnector::importKeysFromKeyserver(" << _keys << ")";

    QStringList keysToImport = _keys.split("|");
    keysToImport.removeDuplicates();

    if(keysToImport.size() == 0)
        return false;

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --keyserver " + this->gpgKeyserverURL + " --recv-keys ") + keysToImport.join(" ");

    qDebug() << "Starting: " << gpgIn;

    this->currentState = GPG_IMPORT;
    this->processIsRunning = true;
    this->process_gpg->start(gpgIn);

    return true;
}

// Sign (with ID):
// gpg --sign-key --batch 1096198193
//
// Owner trust:
// gpg --import-ownertrust $TEMP_FILE
// TMP_FILE: "B4D94345B0986AB5EE9DCD755DE249961B012345:3:"
// Fingerprint + Trust-Level
// 1 = Don't know
// 2 = I do NOT trust
// 3 = I trust marginally
// 4 = I trust fully
//
// Search and import:
// gpg --batch --keyserver key.adeti.org --search-keys "Xiao"
// gpg --batch --keyserver key.adeti.org --recv-keys 0ED59ADE

