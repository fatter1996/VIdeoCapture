/****************************************************************************
** Meta object code from reading C++ file 'VoiceDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../VoiceCall/VoiceDialog/VoiceDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VoiceDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VoiceDialog_t {
    QByteArrayData data[14];
    char stringdata0[153];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VoiceDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VoiceDialog_t qt_meta_stringdata_VoiceDialog = {
    {
QT_MOC_LITERAL(0, 0, 11), // "VoiceDialog"
QT_MOC_LITERAL(1, 12, 9), // "NewMember"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 10), // "MemberQuit"
QT_MOC_LITERAL(4, 34, 10), // "onResponse"
QT_MOC_LITERAL(5, 45, 4), // "data"
QT_MOC_LITERAL(6, 50, 14), // "onVoiceReceive"
QT_MOC_LITERAL(7, 65, 12), // "onSignalling"
QT_MOC_LITERAL(8, 78, 10), // "onAgreeBtn"
QT_MOC_LITERAL(9, 89, 13), // "onDisAgreeBtn"
QT_MOC_LITERAL(10, 103, 10), // "onCloseBtn"
QT_MOC_LITERAL(11, 114, 15), // "onNewConnection"
QT_MOC_LITERAL(12, 130, 15), // "onDisConnection"
QT_MOC_LITERAL(13, 146, 6) // "onCall"

    },
    "VoiceDialog\0NewMember\0\0MemberQuit\0"
    "onResponse\0data\0onVoiceReceive\0"
    "onSignalling\0onAgreeBtn\0onDisAgreeBtn\0"
    "onCloseBtn\0onNewConnection\0onDisConnection\0"
    "onCall"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VoiceDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   69,    2, 0x06 /* Public */,
       3,    2,   76,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   81,    2, 0x0a /* Public */,
       6,    1,   84,    2, 0x0a /* Public */,
       7,    1,   87,    2, 0x0a /* Public */,
       8,    0,   90,    2, 0x0a /* Public */,
       9,    0,   91,    2, 0x0a /* Public */,
      10,    1,   92,    2, 0x0a /* Public */,
      11,    3,   95,    2, 0x0a /* Public */,
      12,    2,  102,    2, 0x0a /* Public */,
      13,    2,  107,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UInt, QMetaType::QString,    2,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::QByteArray,    5,
    QMetaType::Void, QMetaType::QByteArray,    2,
    QMetaType::Void, QMetaType::QByteArray,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt, QMetaType::QString,    2,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,

       0        // eod
};

void VoiceDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VoiceDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->NewMember((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: _t->MemberQuit((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 2: _t->onResponse((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 3: _t->onVoiceReceive((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 4: _t->onSignalling((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 5: _t->onAgreeBtn(); break;
        case 6: _t->onDisAgreeBtn(); break;
        case 7: _t->onCloseBtn((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onNewConnection((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 9: _t->onDisConnection((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 10: _t->onCall((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VoiceDialog::*)(QString , uint , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VoiceDialog::NewMember)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VoiceDialog::*)(QString , uint );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VoiceDialog::MemberQuit)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VoiceDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_VoiceDialog.data,
    qt_meta_data_VoiceDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VoiceDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VoiceDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VoiceDialog.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int VoiceDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void VoiceDialog::NewMember(QString _t1, uint _t2, QString _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VoiceDialog::MemberQuit(QString _t1, uint _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
