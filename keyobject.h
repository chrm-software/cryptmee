#ifndef KEYOBJECT_H
#define KEYOBJECT_H

#include <QStringList>

class KeyObject
{
public:
    KeyObject(QString _fromGnuPG, bool _searchResult);

    QString fromGnuPG;
    QString keyID;
    QStringList identities;
    QString date;
    QString trustValue;
    QString length;
    QString expires;
    QString fingerprint;
    QString isSecretKey;

};

#endif // KEYOBJECT_H
