#include "keyobject.h"
#include <QDebug>
#include <QStringList>
#include <QDateTime>

KeyObject::KeyObject(QString _fromGnuPG, bool _searchResult = false)
{
    this->trustValue = "o";

    this->fromGnuPG = _fromGnuPG;

    if(!_searchResult) {
        QStringList uids = this->fromGnuPG.split("uid:");

        if(uids.at(0).contains("fpr:")) {
            this->fingerprint = uids.at(0).split(QRegExp("\\s+fpr:")).at(1);

            if(this->fingerprint.contains(":")) {
                this->fingerprint = this->fingerprint.split(":").at(8);
            }
        }

        this->keyID = uids.at(0).split(":").at(3);
        this->trustValue = uids.at(0).split(":").at(0);
        this->length = uids.at(0).split(":").at(1);


        QDateTime dt;
        dt.setTime_t(QString(uids.at(0).split(":").at(4)).toInt());
        this->date = dt.toString(Qt::SystemLocaleShortDate);

        QDateTime dt_exp;
        dt_exp.setTime_t(QString(uids.at(0).split(":").at(5)).toInt());
        this->expires = dt_exp.toString(Qt::SystemLocaleShortDate);

        uids.removeFirst();

        // Take last uid and separate sub-keys
        QStringList subKeys = uids.at(uids.size()-1).split("sub:");

        // First part is the corrected last uid
        uids[uids.size()-1] = subKeys[0];

        for(int i=0; i<uids.size(); i++) {
            uids[i] = uids.at(i).split(":").at(8);
        }

        // copy
        this->identities = uids;

    } else {
        // Parsing search results
        _fromGnuPG.replace(QRegExp(">\\s+"), ">|||");
        this->identities = _fromGnuPG.split("|||");

        // Last one are key data
        QString dataPart = this->identities.last();
        this->identities.removeOne(this->identities.last());

        dataPart.replace(QRegExp(".*\\s+([A-Z0-9]+),\\s+.*:\\s+([\\S]+)\\s*$"), "\\1,\\2");

        qDebug() << "###>" << dataPart;

        this->keyID = dataPart.split(",").at(0);
        this->date = dataPart.split(",").at(1);
    }

    qDebug() << "KeyObject: ID: " << this->keyID;
    qDebug() << "KeyObject: FPR: " << this->fingerprint;
    qDebug() << "KeyObject: Date: " << this->date;
    qDebug() << "KeyObject: UIDs: " << this->identities;
    qDebug() << "KeyObject: Trust: " << this->trustValue;
}
