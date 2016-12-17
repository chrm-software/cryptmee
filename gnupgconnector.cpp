#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDateTime>
#include "gnupgconnector.h"

GnuPGConnector::GnuPGConnector(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    qDebug() << "GnuPGConnector!";
    this->gpgHistory.append("=== CryptMee started");

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
    }

    //settings.remove("SETTINGS_RELEASE");
    this->gpgBinaryPath = settings.value("SETTINGS_GPGPATH", GPGBIN).toString();
    this->gpgKeyserverURL = settings.value("SETTINGS_GPGKEYSERVER", KEYSERVER).toString();
    this->localMailPath = settings.value("SETTINGS_MAILDIR", MAIL_PATH).toString();
    this->localMailDB = settings.value("SETTINGS_MAILDB", MAIL_DB).toString();
    this->useOwnKey = settings.value("SETTINGS_USEOWNKEY", "0").toString();

    // Initial check GnuPG version
    this->checkGPGVersion(this->gpgBinaryPath);
}

QString GnuPGConnector::getHistory()
{
    return this->gpgHistory.join("\n");
}

void GnuPGConnector::clearHistory()
{
    this->gpgHistory.clear();
    this->gpgHistory.append("[CLEARED BY USER]");
}

bool GnuPGConnector::saveHistory(QString _filename)
{
    QString outputFilename = _filename;
    QFile outputFile(outputFilename);
    outputFile.open(QIODevice::WriteOnly);

    if(!outputFile.isOpen()){
        qDebug() << "*** Error, unable to open" << outputFilename << "for log output";
        return false;
    }

    QTextStream outStream(&outputFile);
    outStream << this->getHistory();
    outputFile.close();

    return true;
}

void GnuPGConnector::settingsReset()
{
    QSettings settings;
    settings.setValue("SETTINGS_RELEASE", RELEASE);
    settings.setValue("SETTINGS_GPGPATH", GPGBIN);
    settings.setValue("SETTINGS_GPGKEYSERVER", KEYSERVER);
    settings.setValue("SETTINGS_MAILDIR", MAIL_PATH);
    settings.setValue("SETTINGS_MAILDB", MAIL_DB);
    settings.setValue("SETTINGS_USEOWNKEY", "0");
    settings.setValue("SETTINGS_OTR_ACCOUNT1", "|");
    settings.setValue("SETTINGS_OTR_AUTORUN", "0");
    settings.setValue("SETTINGS_OTR_SHOW_NOTIFICATIONS", "0");
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
    this->useOwnKey = settings.value("SETTINGS_USEOWNKEY", "0").toString();
}

QString GnuPGConnector::settingsGetValue(QString _key)
{
    QSettings settings;
    return settings.value(_key).toString();
}

// Call gpg with given command
bool GnuPGConnector::callGnuPG(QString _cmd, int _state)
{
    qDebug() << "GnuPGConnector::callGnuPG(" << _cmd << ", " << _state << ")";

    if(_cmd.size() < 5)
        return false;

    this->errorCode = -1; // No error
    this->gpgStdOutput = "";
    this->gpgErrOutput = "No Errors";

    this->currentState = _state;
    this->processIsRunning = true;
    this->process_gpg->start(_cmd);

    // Remove passwords from log
    if(_state == GPG_DECRYPT || _state == GPG_SIGN || _state == GPG_DECRYPT_FILE ) {
        qDebug() << "GnuPGConnector::callGnuPG() - try to replace passphrase...";
        _cmd.replace(QRegExp("--passphrase \"[^\"]*\" "), "--passphrase \"#######\" ");
    }

    this->gpgHistory.append("\n=== Starting new gpg command at " + QDateTime::currentDateTime().toString());
    this->gpgHistory.append(_cmd);

    return true;
}

bool GnuPGConnector::checkGPGVersion(QString _path)
{
    qDebug() << "GnuPGConnector::checkGPGVersion(" << _path << ")";

    QString gpgIn = _path + QString(" --version");
    this->callGnuPG(gpgIn, GPG_GET_VERSION);

    return true;
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

    if(!this->writeToTmpFile(_input))
        return "*** Error occured ***";

    if(this->useOwnKey == "1")
        _recipient += " " + this->myKeyReader->getAllPrivateKeyIDs(true);

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --armor -r ") + _recipient + " -o " + TMPFILE + ".asc" + " --encrypt " + TMPFILE;
    this->callGnuPG(gpgIn, GPG_ENCRYPT);

    return "";
}

QString GnuPGConnector::decrypt(QString _input, QString _passphrase)
{
    qDebug() << "GnuPGConnector::decrypt(" << _input << "," << "***" << ")";

    if(!this->writeToTmpFile(_input))
        return "*** Error occured ***";

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --charset utf-8 --display-charset utf-8 --passphrase \"") + _passphrase + "\" -o " + TMPFILE + ".txt" + " --decrypt " + TMPFILE;
    this->callGnuPG(gpgIn, GPG_DECRYPT);

    return "";
}

QString GnuPGConnector::encryptFile(QString _filename, QString _recipient)
{
    qDebug() << "GnuPGConnector::encryptFile(" << _filename << ")";

    if(_filename.startsWith("file://"))
        _filename.replace("file://", "");

    if(!_filename.startsWith("/"))
        return "";

    this->filenameInProgress = TMP_DIR + _filename.split("/").last() + ".gpg";

    if(this->useOwnKey == "1")
        _recipient += " " + this->myKeyReader->getAllPrivateKeyIDs(true);

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust -r ") + _recipient + " -o " + this->filenameInProgress + " --encrypt " + _filename;
    this->callGnuPG(gpgIn, GPG_ENCRYPT_FILE);

    return "";
}

QString GnuPGConnector::decryptFile(QString _filename, QString _passphrase)
{
    qDebug() << "GnuPGConnector::decryptFile(" << _filename << "," << "***" << ")";
    QString filenameOrig = _filename.replace(".gpg", "");
    this->filenameInProgress = filenameOrig;

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --charset utf-8 --display-charset utf-8 --passphrase \"") + _passphrase + "\" -o " + filenameOrig + " --decrypt " + _filename;
    this->callGnuPG(gpgIn, GPG_DECRYPT_FILE);

    return "";

}

QString GnuPGConnector::showKeys()
{
    qDebug() << "GnuPGConnector::showKeys()";

    QString gpgIn = this->gpgBinaryPath + QString(" --charset utf-8 --display-charset utf-8 --list-public-keys --fixed-list-mode --with-colons --with-fingerprint");
    this->callGnuPG(gpgIn, GPG_KEYS);

    return "";
}

QString GnuPGConnector::showSecretKeys()
{
    qDebug() << "GnuPGConnector::showSecretKeys()";

    QString gpgIn = this->gpgBinaryPath + QString(" --charset utf-8 --display-charset utf-8 --list-secret-keys --fixed-list-mode --with-colons --with-fingerprint");
    this->callGnuPG(gpgIn, GPG_SECRET_KEYS);

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

    qDebug() << "GnuPGConnector::gpgFinished(): State finished: " << this->currentState;
    qDebug() << "GnuPGConnector::gpgFinished(): stdout: " << output.left(100) << "...";
    qDebug() << "GnuPGConnector::gpgFinished(): stderr: " << error;

    if(this->currentState == GPG_KEYS) {
        // Key listing finished: parse output
        this->myKeyReader->parseGnuPGOutput(output);        

    } else if(this->currentState == GPG_SECRET_KEYS) {
        // All keys are now available. Compare secret keys with public keys
        this->myKeyReader->parseGnuPGPrivateKeysOutput(output);

    } else if(this->currentState == GPG_KEYS_GEN) {
        // Nothing to do: just check retval

    } else if(this->currentState == GPG_GET_VERSION) {
        // Set GnuPG version
        this->gpgStdOutput = output;

        if(_retVal != 0)
            this->gpgVersionString = "-1";
        else
            this->gpgVersionString = output.left(55) + "...";

    } else if(this->currentState == GPG_ENCRYPT) {
        // Enrypted content from out file
        this->gpgStdOutput = this->readFromTmpFile(0);

    } else if(this->currentState == GPG_IMPORT) {
        // Import command output
        this->gpgStdOutput = output;

    } else if(this->currentState == GPG_DELETE) {
        // Delete command output
        this->gpgStdOutput = output;

    } else if(this->currentState == GPG_SEARCH) {        
        this->gpgStdOutput = output;
        this->myKeyReader->parseGnuPGServerSearchOutput(output);

        // Hack: ignore errors
        _retVal = 0;

    } else if(this->currentState == GPG_SET_TRUST) {
        this->gpgStdOutput = output;

    } else if(this->currentState == GPG_SIGN) {
        this->gpgStdOutput = output;

    } else if(this->currentState == GPG_EXPORT) {
        this->gpgStdOutput = output;

    } else if(this->currentState == GPG_ENCRYPT_FILE) {
        this->gpgStdOutput = output;

    } else {
        // Decrypted content form StdOut
        this->gpgStdOutput = this->readFromTmpFile(1);
    }


    // Remove tmp files
    QFile::remove(QString(TMPFILE) + ".asc");
    QFile::remove(QString(TMPFILE) + ".txt");
    QFile::remove(QString(TMPFILE));

    if(output.length() > 200)
        this->gpgHistory.append("stdout: " + output.left(150) + " [...] " + output.right(50));
    else
        this->gpgHistory.append("stdout: " + output);

    this->gpgHistory.append("stderr: " + error);

    this->currentState = GPG_IDLE;

    if(_retVal != 0) {
        this->gpgErrOutput = QString("[") + QString::number(_retVal) + "] " + error;
        emit errorOccured();
    } else {
        emit ready();
    }    
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

    this->gpgHistory << ("QProcess-Error: [" + QString::number(_pe) + "]");

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
    retVal += "Created: <b>" + this->myKeyReader->getKeyByID(_id)->date + "</b>\n<br>";
    retVal += "Expires: <b>" + this->myKeyReader->getKeyByID(_id)->expires + "</b>\n<br>";
    retVal += "Length: <b>" + this->myKeyReader->getKeyByID(_id)->length + "</b>, Trust: <b>[";
    retVal += this->myKeyReader->getKeyByID(_id)->trustValue + "]</b>\n<br>";
    retVal += (this->myKeyReader->getKeyByID(_id)->identities.join("\n")).replace("<", "&lt;").replace("\n", "\n<br>");
    return retVal;
}

int GnuPGConnector::getNumOfPubKeys(int _type)
{
    return this->myKeyReader->getNumOfKeys(_type);
}

QString GnuPGConnector::getPrivateKeyIDs(bool _asCommandLine)
{
    return this->myKeyReader->getAllPrivateKeyIDs(_asCommandLine);
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

    return this->callGnuPG(gpgIn, GPG_KEYS_GEN);
}

bool GnuPGConnector::setOwnerTrust(QString _id, QString _trustLevel)
{
    qDebug() << "GnuPGConnector::setOwnerTrust(): ID: " << _id;

    if(_id == "" || _trustLevel == "")
        return false;

    if(this->myKeyReader->getKeyByID(_id) == NULL)
        return false;

    // Generate file first
    QString genTrustFile;

    genTrustFile = this->myKeyReader->getKeyByID(_id)->fingerprint + ":" + _trustLevel + ":\n";

    this->writeToTmpFile(genTrustFile);

    QString gpgIn = this->gpgBinaryPath + QString(" --import-ownertrust ") + TMPFILE;

    return this->callGnuPG(gpgIn, GPG_SET_TRUST);
}

bool GnuPGConnector::importKeysFromFile(QString _path)
{
    // Expand with prefix path MYDOCS_PATH

    qDebug() << "GnuPGConnector::importKeysFromFile(" << _path << ")";

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --import ") + MYDOCS_PATH +_path;

    return this->callGnuPG(gpgIn, GPG_IMPORT);
}

bool GnuPGConnector::importKeysFromClipboard()
{
    QString clipBoard = this->getFromClipboard();

    qDebug() << "GnuPGConnector::importKeysFromClipboard()";

    // Write to file
    this->writeToTmpFile(clipBoard);
    QString gpgIn = this->gpgBinaryPath + QString(" --batch --import ") + TMPFILE;

    return this->callGnuPG(gpgIn, GPG_IMPORT);
}

bool GnuPGConnector::searchKeysOnKeyserver(QString _keyword)
{
    qDebug() << "GnuPGConnector::searchKeysOnKeyserver(" << _keyword << ")";

    if(_keyword == "")
        return false;

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --keyserver " + this->gpgKeyserverURL + " --search-keys ") + _keyword;

    return this->callGnuPG(gpgIn, GPG_SEARCH);
}

bool GnuPGConnector::importKeysFromKeyserver(QString _keys)
{
    qDebug() << "GnuPGConnector::importKeysFromKeyserver(" << _keys << ")";

    QStringList keysToImport = _keys.split("|");
    keysToImport.removeDuplicates();

    if(keysToImport.size() == 0)
        return false;

    QString gpgIn = this->gpgBinaryPath + QString(" --batch --keyserver " + this->gpgKeyserverURL + " --recv-keys ") + keysToImport.join(" ");

    return this->callGnuPG(gpgIn, GPG_IMPORT);
}

bool GnuPGConnector::deleteKey(QString _id)
{
    qDebug() << "GnuPGConnector::deleteKey(" << _id << ")";

    QStringList keyData = _id.split("|");
    QString gpgIn;

    if(keyData.size() == 2) {
        if(keyData.at(1) == "1") {
            // This is a private Key
            gpgIn = this->gpgBinaryPath + QString(" --batch --yes --delete-secret-key ") + this->myKeyReader->getKeyByID(keyData.at(0))->fingerprint;
            return this->callGnuPG(gpgIn, GPG_DELETE);
        } else {
            gpgIn = this->gpgBinaryPath + QString(" --batch --yes --delete-key ") + keyData.at(0);
            return this->callGnuPG(gpgIn, GPG_DELETE);
        }
    }

    gpgIn = this->gpgBinaryPath + QString(" --batch --yes --delete-key ") + _id;
    return this->callGnuPG(gpgIn, GPG_DELETE);
}

bool GnuPGConnector::signKey(QString _id, QString _passwd, QString _privateKey)
{
    qDebug() << "GnuPGConnector::signKey(" << _id << ", " << _privateKey << ")";
    QString gpgIn;

    gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --passphrase \"") + _passwd + "\" -u " + _privateKey + " --sign-key " + _id;
    return this->callGnuPG(gpgIn, GPG_SIGN);
}

bool GnuPGConnector::exportKeys(int _mode, QString _path)
{
    qDebug() << "GnuPGConnector::exportKeys(" << _mode << ")";
    QString gpgIn;

    if(_mode == 1) {
        gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --export --armor --output ") + MYDOCS_PATH + _path;
    } else {
        gpgIn = this->gpgBinaryPath + QString(" --batch --no-tty --yes --always-trust --export-secret-keys --armor --output ") + MYDOCS_PATH + _path;
    }

    return this->callGnuPG(gpgIn, GPG_EXPORT);
}

QString GnuPGConnector::getLastEncryptedFilename()
{
    return this->filenameInProgress;
}




