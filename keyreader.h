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
    bool parseGnuPGServerSearchOutput(QString _fromGnuPG);

    QString getKeyAsHTMLString(int _key, bool _fullInfo = false, int _type = 0);
    int getNumOfKeys(int _storeType = 0);
    KeyObject* getKeyByID(QString _id);


private:
    QString fromGnuPG;
    QList<KeyObject*> allKeys;
    QList<KeyObject*> lastSearchKeys;
};

#endif // KEYREADER_H
