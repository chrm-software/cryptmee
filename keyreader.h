#ifndef KEYREADER_H
#define KEYREADER_H

#include <QHash>
#include <QStringList>
#include "keyobject.h"

class KeyReader
{
public:
    KeyReader();
    bool parseGnuPGOutput(QString _fromGnuPG);
    QString getKeyAsHTMLString(int _key, bool _fullInfo = false);
    int getNumOfKeys();
    KeyObject* getKeyByID(QString _id);


private:
    QString fromGnuPG;
    QList<KeyObject*> allKeys;
};

#endif // KEYREADER_H
