/****************************************************************************
** Meta object code from reading C++ file 'window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../tesselate/window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Window_t {
    QByteArrayData data[25];
    char stringdata0[248];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Window_t qt_meta_stringdata_Window = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Window"
QT_MOC_LITERAL(1, 7, 12), // "repaintAllGL"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 8), // "saveFile"
QT_MOC_LITERAL(4, 30, 6), // "saveAs"
QT_MOC_LITERAL(5, 37, 9), // "showModel"
QT_MOC_LITERAL(6, 47, 4), // "show"
QT_MOC_LITERAL(7, 52, 7), // "showLat"
QT_MOC_LITERAL(8, 60, 16), // "showParamOptions"
QT_MOC_LITERAL(9, 77, 14), // "lineEditChange"
QT_MOC_LITERAL(10, 92, 12), // "sliderChange"
QT_MOC_LITERAL(11, 105, 5), // "value"
QT_MOC_LITERAL(12, 111, 8), // "voxPress"
QT_MOC_LITERAL(13, 120, 10), // "marchPress"
QT_MOC_LITERAL(14, 131, 11), // "smoothPress"
QT_MOC_LITERAL(15, 143, 8), // "defPress"
QT_MOC_LITERAL(16, 152, 9), // "loadPress"
QT_MOC_LITERAL(17, 162, 13), // "loadGridPress"
QT_MOC_LITERAL(18, 176, 11), // "shrinkPress"
QT_MOC_LITERAL(19, 188, 8), // "demoMode"
QT_MOC_LITERAL(20, 197, 13), // "loadModelDemo"
QT_MOC_LITERAL(21, 211, 6), // "string"
QT_MOC_LITERAL(22, 218, 8), // "filename"
QT_MOC_LITERAL(23, 227, 10), // "txtDisplay"
QT_MOC_LITERAL(24, 238, 9) // "resetView"

    },
    "Window\0repaintAllGL\0\0saveFile\0saveAs\0"
    "showModel\0show\0showLat\0showParamOptions\0"
    "lineEditChange\0sliderChange\0value\0"
    "voxPress\0marchPress\0smoothPress\0"
    "defPress\0loadPress\0loadGridPress\0"
    "shrinkPress\0demoMode\0loadModelDemo\0"
    "string\0filename\0txtDisplay\0resetView"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Window[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  104,    2, 0x0a /* Public */,
       3,    0,  105,    2, 0x0a /* Public */,
       4,    0,  106,    2, 0x0a /* Public */,
       5,    1,  107,    2, 0x0a /* Public */,
       7,    1,  110,    2, 0x0a /* Public */,
       8,    0,  113,    2, 0x0a /* Public */,
       9,    0,  114,    2, 0x0a /* Public */,
      10,    1,  115,    2, 0x0a /* Public */,
      12,    0,  118,    2, 0x0a /* Public */,
      13,    0,  119,    2, 0x0a /* Public */,
      14,    0,  120,    2, 0x0a /* Public */,
      15,    0,  121,    2, 0x0a /* Public */,
      16,    0,  122,    2, 0x0a /* Public */,
      17,    0,  123,    2, 0x0a /* Public */,
      18,    0,  124,    2, 0x0a /* Public */,
      19,    0,  125,    2, 0x0a /* Public */,
      20,    2,  126,    2, 0x0a /* Public */,
      24,    0,  131,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 21, 0x80000000 | 21,   22,   23,
    QMetaType::Void,

       0        // eod
};

void Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->repaintAllGL(); break;
        case 1: _t->saveFile(); break;
        case 2: _t->saveAs(); break;
        case 3: _t->showModel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->showLat((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->showParamOptions(); break;
        case 6: _t->lineEditChange(); break;
        case 7: _t->sliderChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->voxPress(); break;
        case 9: _t->marchPress(); break;
        case 10: _t->smoothPress(); break;
        case 11: _t->defPress(); break;
        case 12: _t->loadPress(); break;
        case 13: _t->loadGridPress(); break;
        case 14: _t->shrinkPress(); break;
        case 15: _t->demoMode(); break;
        case 16: _t->loadModelDemo((*reinterpret_cast< string(*)>(_a[1])),(*reinterpret_cast< string(*)>(_a[2]))); break;
        case 17: _t->resetView(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Window::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_Window.data,
    qt_meta_data_Window,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Window.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 18;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
