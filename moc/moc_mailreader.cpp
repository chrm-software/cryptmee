/****************************************************************************
** Meta object code from reading C++ file 'mailreader.h'
**
** Created: Tue 29. Nov 23:48:47 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mailreader.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mailreader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MailReader[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      20,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      35,   11,   11,   11, 0x08,

 // methods: signature, parameters, type, tag, flags
      63,   49,   11,   11, 0x02,
      89,   86,   78,   11, 0x02,
     106,   11,  102,   11, 0x02,
     122,   86,   78,   11, 0x02,
     138,   86,   78,   11, 0x02,
     159,  153,   78,   11, 0x02,
     185,   11,  102,   11, 0x02,
     210,   86,   78,   11, 0x02,
     229,   11,   78,   11, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_MailReader[] = {
    "MailReader\0\0ready()\0errorOccured()\0"
    "threadReady()\0_fromLastDays\0readMails(int)\0"
    "QString\0_i\0getMail(int)\0int\0getNumOfMails()\0"
    "getContent(int)\0getHeader(int)\0_mail\0"
    "parseMailContent(QString)\0"
    "getAttachmentListCount()\0getAttachment(int)\0"
    "getMIMEMailContent()\0"
};

const QMetaObject MailReader::staticMetaObject = {
    { &QDeclarativeItem::staticMetaObject, qt_meta_stringdata_MailReader,
      qt_meta_data_MailReader, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MailReader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MailReader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MailReader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MailReader))
        return static_cast<void*>(const_cast< MailReader*>(this));
    return QDeclarativeItem::qt_metacast(_clname);
}

int MailReader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDeclarativeItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ready(); break;
        case 1: errorOccured(); break;
        case 2: threadReady(); break;
        case 3: readMails((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: { QString _r = getMail((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 5: { int _r = getNumOfMails();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { QString _r = getContent((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 7: { QString _r = getHeader((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 8: { QString _r = parseMailContent((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 9: { int _r = getAttachmentListCount();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 10: { QString _r = getAttachment((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 11: { QString _r = getMIMEMailContent();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void MailReader::ready()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MailReader::errorOccured()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
