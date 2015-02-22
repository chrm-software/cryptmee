#include <QDebug>
#include "keyreader.h"

KeyReader::KeyReader()
{    
}

bool KeyReader::parseGnuPGOutput(QString _fromGnuPG)
{
    // Parse content;
    QStringList tmpList;

    // Clear List
    for(int i=0; i<this->allKeys.size(); i++) {
        delete this->allKeys.at(i);
    }

    this->allKeys.clear();

    QString output = _fromGnuPG;

    if(output.length() > 10) {
        tmpList = output.split(QRegExp("\\s+pub:"));
        tmpList.removeAt(0);

    } else {
        // Too short. Ignore.
        return false;
    }

    for(int i=0; i<tmpList.size(); i++) {
        // create KeyObjects
        KeyObject* tmp = new KeyObject(tmpList[i], false);
        this->allKeys << tmp;
    }

    return true;
}

bool KeyReader::parseGnuPGPrivateKeysOutput(QString _fromGnuPG)
{
    // Parse content;
    QStringList tmpList;
    qDebug() << "KeyReader::parseGnuPGPrivateKeysOutput(..)";

    // Clear List
    for(int i=0; i<this->allSecretKeys.size(); i++) {
        delete this->allSecretKeys.at(i);
    }

    this->allSecretKeys.clear();

    QString output = _fromGnuPG;

    if(output.length() > 10) {
        tmpList = output.split(QRegExp("\\s*sec:"));
        tmpList.removeAt(0);

    } else {
        // Too short. Ignore.
        return false;
    }

    for(int i=0; i<tmpList.size(); i++) {
        // create KeyObjects
        KeyObject* tmp = new KeyObject(tmpList[i], false);
        this->allSecretKeys << tmp;
    }

    // Check public key list for secret keys
    QString pubID;
    for(int i=0; i<this->allKeys.size(); i++) {
        pubID = this->allKeys.at(i)->keyID;
        for(int j=0; j<this->allSecretKeys.size(); j++) {
            if(this->allSecretKeys.at(j)->keyID == pubID) {
                // Found one
                this->allKeys.at(i)->isSecretKey = "1";
            }
        }
    }

    return true;
}

bool KeyReader::parseGnuPGServerSearchOutput(QString _fromGnuPG)
{
    // Parse content;
    QStringList tmpList;
    qDebug() << "KeyReader::parseGnuPGServerSearchOutput()";

    // Clear List
    for(int i=0; i<this->lastSearchKeys.size(); i++) {
        delete this->lastSearchKeys.at(i);
    }

    this->lastSearchKeys.clear();

    tmpList = _fromGnuPG.split(QRegExp("\\([0-9]{1,2}\\)"));

    for(int i=1; i<tmpList.size(); i++) {
        KeyObject* tmp = new KeyObject(tmpList[i], true);
        this->lastSearchKeys << tmp;
    }

    return true;
}

QString KeyReader::getKeyAsHTMLString(int _key, bool _fullInfo, int _type) {
    QString retVal;

    if(_type == 0) {
        // Local keys
        if(!_fullInfo) {
            retVal = this->allKeys[_key]->trustValue + "|" + this->allKeys[_key]->keyID + "|(Created: " + this->allKeys[_key]->date + ")" ;

            for(int i=0; i<this->allKeys[_key]->identities.size(); i++) {
                retVal += "<br>" + QString(this->allKeys[_key]->identities.at(i)).replace("<", "&lt;");
            }

            retVal += "|" + this->allKeys[_key]->isSecretKey;

        } else {
            retVal = "ID: " + this->allKeys[_key]->keyID + "<br>";
            retVal += "Created: " + this->allKeys[_key]->date + "<br>" + "Expires: " + this->allKeys[_key]->expires + "<br>";
            retVal += "Length: " + this->allKeys[_key]->length;

            for(int i=0; i<this->allKeys[_key]->identities.size(); i++) {
                retVal += "<br>" + QString(this->allKeys[_key]->identities.at(i)).replace("<", "&lt;");
            }
        }
    } else if(_type == 1) {
        // Search keys
        retVal = this->lastSearchKeys[_key]->keyID + "|(Created: " + this->lastSearchKeys[_key]->date + ")" ;

        for(int i=0; i<this->lastSearchKeys[_key]->identities.size(); i++) {
            retVal += "<br>" + QString(this->lastSearchKeys[_key]->identities.at(i)).replace("<", "&lt;");
        }
    }

    return retVal;
}

int KeyReader::getNumOfKeys(int _storeType)
{
    if(_storeType == 0)
        return this->allKeys.size();
    else if(_storeType == 1)
        return this->lastSearchKeys.size();
    else if(_storeType == 2)
        return this->allSecretKeys.size();

    return 0;
}

KeyObject* KeyReader::getKeyByID(QString _id)
{
    for(int i=0; i<this->allKeys.size(); i++) {
        if(this->allKeys.at(i)->keyID == _id)
            return this->allKeys.at(i);
    }

    return NULL;
}

QString KeyReader::getAllPrivateKeyIDs(bool _asCommandLine)
{
    QString retVal;
    QString param = " ";

    if(_asCommandLine)
        param = " -r ";

    for(int i=0; i<this->allSecretKeys.size(); i++) {
       retVal += param + this->allSecretKeys.at(i)->keyID;
    }

    return retVal;
}
