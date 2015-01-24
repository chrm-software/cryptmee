/****************************************************************************
** Meta object code from reading C++ file 'mailreaderthread.h'
**
** Created: Fri 23. Jan 18:47:02 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mailreaderthread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mailreaderthread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MailReaderThread[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,
      32,   17,   17,   17, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_MailReaderThread[] = {
    "MailReaderThread\0\0readyThread()\0"
    "errorOccuredThread()\0"
};

const QMetaObject MailReaderThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_MailReaderThread,
      qt_meta_data_MailReaderThread, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MailReaderThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MailReaderThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MailReaderThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MailReaderThread))
        return static_cast<void*>(const_cast< MailReaderThread*>(this));
    return QThread::qt_metacast(_clname);
}

int MailReaderThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: readyThread(); break;
        case 1: errorOccuredThread(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void MailReaderThread::readyThread()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MailReaderThread::errorOccuredThread()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
