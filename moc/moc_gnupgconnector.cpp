/****************************************************************************
** Meta object code from reading C++ file 'gnupgconnector.h'
**
** Created: Thu 3. Dec 16:12:55 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../gnupgconnector.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gnupgconnector.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GnuPGConnector[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      34,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      24,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      47,   39,   15,   15, 0x08,
      68,   64,   15,   15, 0x08,

 // methods: signature, parameters, type, tag, flags
     127,  109,  101,   15, 0x02,
     171,  152,  101,   15, 0x02,
     196,   15,  101,   15, 0x02,
     207,   15,  101,   15, 0x02,
     232,  224,  101,   15, 0x02,
     246,   15,  101,   15, 0x02,
     271,  265,   15,   15, 0x02,
     304,  295,  101,   15, 0x02,
     323,  320,  101,   15, 0x22,
     339,  335,  101,   15, 0x02,
     374,  359,  101,   15, 0x02,
     397,   15,  101,   15, 0x22,
     426,  420,  416,   15, 0x02,
     447,   15,  416,   15, 0x22,
     504,  470,  465,   15, 0x02,
     569,  553,  465,   15, 0x02,
     606,  600,  465,   15, 0x02,
     631,   15,  101,   15, 0x02,
     653,  600,  465,   15, 0x02,
     681,   15,  465,   15, 0x02,
     716,  707,  465,   15, 0x02,
     753,  747,  465,   15, 0x02,
     786,  335,  465,   15, 0x02,
     829,  805,  465,   15, 0x02,
     874,  862,  465,   15, 0x02,
     898,   15,  101,   15, 0x02,
     921,  911,  465,   15, 0x02,
     954,  942,   15,   15, 0x02,
     993,  988,  101,   15, 0x02,
    1019,   15,   15,   15, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_GnuPGConnector[] = {
    "GnuPGConnector\0\0ready()\0errorOccured()\0"
    "_retVal\0gpgFinished(int)\0_pe\0"
    "gpgError(QProcess::ProcessError)\0"
    "QString\0_input,_recipient\0"
    "encrypt(QString,QString)\0_input,_passphrase\0"
    "decrypt(QString,QString)\0showKeys()\0"
    "showSecretKeys()\0_errors\0getData(bool)\0"
    "getFromClipboard()\0_data\0"
    "setToClipboard(QString)\0_i,_type\0"
    "getKey(int,int)\0_i\0getKey(int)\0_id\0"
    "getKeyByID(QString)\0_asCommandLine\0"
    "getPrivateKeyIDs(bool)\0getPrivateKeyIDs()\0"
    "int\0_type\0getNumOfPubKeys(int)\0"
    "getNumOfPubKeys()\0bool\0"
    "_name,_comment,_email,_passphrase\0"
    "generateKeyPair(QString,QString,QString,QString)\0"
    "_id,_trustLevel\0setOwnerTrust(QString,QString)\0"
    "_path\0checkGPGVersion(QString)\0"
    "getGPGVersionString()\0importKeysFromFile(QString)\0"
    "importKeysFromClipboard()\0_keyword\0"
    "searchKeysOnKeyserver(QString)\0_keys\0"
    "importKeysFromKeyserver(QString)\0"
    "deleteKey(QString)\0_id,_passwd,_privateKey\0"
    "signKey(QString,QString,QString)\0"
    "_mode,_path\0exportKeys(int,QString)\0"
    "getHistory()\0_filename\0saveHistory(QString)\0"
    "_key,_value\0settingsSetValue(QString,QString)\0"
    "_key\0settingsGetValue(QString)\0"
    "settingsReset()\0"
};

const QMetaObject GnuPGConnector::staticMetaObject = {
    { &QDeclarativeItem::staticMetaObject, qt_meta_stringdata_GnuPGConnector,
      qt_meta_data_GnuPGConnector, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GnuPGConnector::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GnuPGConnector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GnuPGConnector::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GnuPGConnector))
        return static_cast<void*>(const_cast< GnuPGConnector*>(this));
    return QDeclarativeItem::qt_metacast(_clname);
}

int GnuPGConnector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDeclarativeItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ready(); break;
        case 1: errorOccured(); break;
        case 2: gpgFinished((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: gpgError((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        case 4: { QString _r = encrypt((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 5: { QString _r = decrypt((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 6: { QString _r = showKeys();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 7: { QString _r = showSecretKeys();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 8: { QString _r = getData((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 9: { QString _r = getFromClipboard();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 10: setToClipboard((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 11: { QString _r = getKey((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 12: { QString _r = getKey((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 13: { QString _r = getKeyByID((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 14: { QString _r = getPrivateKeyIDs((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 15: { QString _r = getPrivateKeyIDs();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 16: { int _r = getNumOfPubKeys((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 17: { int _r = getNumOfPubKeys();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 18: { bool _r = generateKeyPair((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 19: { bool _r = setOwnerTrust((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 20: { bool _r = checkGPGVersion((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 21: { QString _r = getGPGVersionString();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 22: { bool _r = importKeysFromFile((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 23: { bool _r = importKeysFromClipboard();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 24: { bool _r = searchKeysOnKeyserver((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 25: { bool _r = importKeysFromKeyserver((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 26: { bool _r = deleteKey((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 27: { bool _r = signKey((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 28: { bool _r = exportKeys((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 29: { QString _r = getHistory();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 30: { bool _r = saveHistory((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 31: settingsSetValue((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 32: { QString _r = settingsGetValue((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 33: settingsReset(); break;
        default: ;
        }
        _id -= 34;
    }
    return _id;
}

// SIGNAL 0
void GnuPGConnector::ready()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void GnuPGConnector::errorOccured()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
