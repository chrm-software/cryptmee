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

    //QString output = "tru::1:1419856758:0:3:1:5 pub:u:2048:1:A6AC7F8669C9DE12:1419856555:::u:::scESC: uid:u::::1419856555::3CD6113D38E514E4CDF974FEA316A32E56E106AF::Christoph Mac <chrm@freenet.de>: sub:u:2048:1:97684DE47ABEC8D1:1419856555::::::e: pub:-:1024:17:DB353821E6687E7D:1052213151:::-:::scaESCA: uid:-::::1052213168::10FA90475FC9673B39E8AF48B848201AD990C01A::Maciejewski Christoph W711 <christoph.maciejewski@erl11.siemens.de>: uid:-::::1052213168::E075987391B6D3FD9FA4310034E1A2F3751DDBAB::Maciejewski Christoph W711 <christoph.maciejewski@siemens.com>: uid:-::::1052213168::8936167F938355E1C73F12AF6488DD5CEC696F97::Christoph (selfGen) Maciejewski <christoph.maciejewski@siemens.com>: sub:-:4096:1:4986758769A0AD4C:1052213151::::::esca:";

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

    tmpList = _fromGnuPG.split(QRegExp("\\([0-9]+\\)"));

    for(int i=1; i<tmpList.size(); i++) {
        KeyObject* tmp = new KeyObject(tmpList[i], true);
        this->lastSearchKeys << tmp;
    }

    return true;
}

QString KeyReader::getKeyAsHTMLString(int _key, bool _fullInfo, int _type) {
    QString retVal;

    if(_type == 0) {
        if(!_fullInfo) {
            retVal = this->allKeys[_key]->trustValue + "|" + this->allKeys[_key]->keyID + "|(Created: " + this->allKeys[_key]->date + ")" ;

            for(int i=0; i<this->allKeys[_key]->identities.size(); i++) {
                retVal += "<br>" + QString(this->allKeys[_key]->identities.at(i)).replace("<", "&lt;");
            }
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
