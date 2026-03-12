/****************************************************************************
** Meta object code from reading C++ file 'ui.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_UIState_t {
    QByteArrayData data[11];
    char stringdata0[96];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UIState_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UIState_t qt_meta_stringdata_UIState = {
    {
QT_MOC_LITERAL(0, 0, 7), // "UIState"
QT_MOC_LITERAL(1, 8, 8), // "uiUpdate"
QT_MOC_LITERAL(2, 17, 0), // ""
QT_MOC_LITERAL(3, 18, 1), // "s"
QT_MOC_LITERAL(4, 20, 16), // "FrogPilotUIState"
QT_MOC_LITERAL(5, 37, 2), // "fs"
QT_MOC_LITERAL(6, 40, 17), // "offroadTransition"
QT_MOC_LITERAL(7, 58, 7), // "offroad"
QT_MOC_LITERAL(8, 66, 14), // "engagedChanged"
QT_MOC_LITERAL(9, 81, 7), // "engaged"
QT_MOC_LITERAL(10, 89, 6) // "update"

    },
    "UIState\0uiUpdate\0\0s\0FrogPilotUIState\0"
    "fs\0offroadTransition\0offroad\0"
    "engagedChanged\0engaged\0update"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UIState[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,
       6,    1,   39,    2, 0x06 /* Public */,
       8,    1,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   45,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 0, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::Bool,    9,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void UIState::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UIState *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->uiUpdate((*reinterpret_cast< const UIState(*)>(_a[1])),(*reinterpret_cast< const FrogPilotUIState(*)>(_a[2]))); break;
        case 1: _t->offroadTransition((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->engagedChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->update(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UIState::*)(const UIState & , const FrogPilotUIState & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIState::uiUpdate)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (UIState::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIState::offroadTransition)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (UIState::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIState::engagedChanged)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject UIState::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_UIState.data,
    qt_meta_data_UIState,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UIState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UIState::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UIState.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int UIState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void UIState::uiUpdate(const UIState & _t1, const FrogPilotUIState & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void UIState::offroadTransition(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void UIState::engagedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_Device_t {
    QByteArrayData data[13];
    char stringdata0[134];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Device_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Device_t qt_meta_stringdata_Device = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Device"
QT_MOC_LITERAL(1, 7, 19), // "displayPowerChanged"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 2), // "on"
QT_MOC_LITERAL(4, 31, 18), // "interactiveTimeout"
QT_MOC_LITERAL(5, 50, 23), // "resetInteractiveTimeout"
QT_MOC_LITERAL(6, 74, 7), // "timeout"
QT_MOC_LITERAL(7, 82, 14), // "timeout_onroad"
QT_MOC_LITERAL(8, 97, 6), // "update"
QT_MOC_LITERAL(9, 104, 7), // "UIState"
QT_MOC_LITERAL(10, 112, 1), // "s"
QT_MOC_LITERAL(11, 114, 16), // "FrogPilotUIState"
QT_MOC_LITERAL(12, 131, 2) // "fs"

    },
    "Device\0displayPowerChanged\0\0on\0"
    "interactiveTimeout\0resetInteractiveTimeout\0"
    "timeout\0timeout_onroad\0update\0UIState\0"
    "s\0FrogPilotUIState\0fs"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Device[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    0,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    2,   48,    2, 0x0a /* Public */,
       5,    1,   53,    2, 0x2a /* Public | MethodCloned */,
       5,    0,   56,    2, 0x2a /* Public | MethodCloned */,
       8,    2,   57,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    7,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9, 0x80000000 | 11,   10,   12,

       0        // eod
};

void Device::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Device *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->displayPowerChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->interactiveTimeout(); break;
        case 2: _t->resetInteractiveTimeout((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->resetInteractiveTimeout((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->resetInteractiveTimeout(); break;
        case 5: _t->update((*reinterpret_cast< const UIState(*)>(_a[1])),(*reinterpret_cast< const FrogPilotUIState(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Device::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::displayPowerChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::interactiveTimeout)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Device::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_Device.data,
    qt_meta_data_Device,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Device::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Device::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Device.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Device::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void Device::displayPowerChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Device::interactiveTimeout()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
