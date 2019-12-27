/****************************************************************************
** Meta object code from reading C++ file 'ftpget.h'
**
** Created: Thu Apr 10 18:46:10 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ftpget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ftpget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_FtpGet[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      21,   15,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FtpGet[] = {
    "FtpGet\0\0done()\0error\0ftpDone(bool)\0"
};

const QMetaObject FtpGet::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FtpGet,
      qt_meta_data_FtpGet, 0 }
};

const QMetaObject *FtpGet::metaObject() const
{
    return &staticMetaObject;
}

void *FtpGet::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FtpGet))
	return static_cast<void*>(const_cast< FtpGet*>(this));
    return QObject::qt_metacast(_clname);
}

int FtpGet::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: done(); break;
        case 1: ftpDone((*reinterpret_cast< bool(*)>(_a[1]))); break;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void FtpGet::done()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
