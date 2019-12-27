/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Fri Mar 14 09:58:34 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      25,   11,   11,   11, 0x0a,
      42,   11,   11,   11, 0x0a,
      57,   11,   11,   11, 0x0a,
      71,   11,   11,   11, 0x0a,
      83,   11,   11,   11, 0x0a,
      99,   11,   11,   11, 0x0a,
     113,   11,   11,   11, 0x0a,
     126,   11,   11,   11, 0x0a,
     144,   11,   11,   11, 0x0a,
     160,   11,   11,   11, 0x0a,
     174,   11,   11,   11, 0x0a,
     182,   11,   11,   11, 0x0a,
     193,   11,   11,   11, 0x0a,
     216,  205,   11,   11, 0x0a,
     244,  237,   11,   11, 0x0a,
     263,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0select_all()\0reflash_config()\0"
    "unselect_all()\0save_config()\0read_item()\0"
    "read_item_ini()\0load_config()\0"
    "write_item()\0readOutput_byte()\0"
    "select_choose()\0init_config()\0clear()\0"
    "read_all()\0write_all()\0set_status\0"
    "set_pushbottom(bool)\0result\0"
    "print_result(bool)\0merge_ddr_size_speed()\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->select_all(); break;
        case 1: _t->reflash_config(); break;
        case 2: _t->unselect_all(); break;
        case 3: _t->save_config(); break;
        case 4: _t->read_item(); break;
        case 5: _t->read_item_ini(); break;
        case 6: _t->load_config(); break;
        case 7: _t->write_item(); break;
        case 8: _t->readOutput_byte(); break;
        case 9: _t->select_choose(); break;
        case 10: _t->init_config(); break;
        case 11: _t->clear(); break;
        case 12: _t->read_all(); break;
        case 13: _t->write_all(); break;
        case 14: _t->set_pushbottom((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->print_result((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->merge_ddr_size_speed(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
