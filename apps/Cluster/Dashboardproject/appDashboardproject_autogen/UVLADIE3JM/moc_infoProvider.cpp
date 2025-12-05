/****************************************************************************
** Meta object code from reading C++ file 'infoProvider.hpp'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/infoProvider.hpp"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'infoProvider.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSinfoProviderENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSinfoProviderENDCLASS = QtMocHelpers::stringData(
    "infoProvider",
    "weatherInfoChanged",
    "",
    "temperatureChanged",
    "localTimeChanged",
    "currentDateChanged",
    "fetchWeatherData",
    "onWeatherDataReceived",
    "QNetworkReply*",
    "reply",
    "weatherInfo",
    "temperature",
    "localTime",
    "currentDate"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSinfoProviderENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       4,   58, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   50,    2, 0x06,    5 /* Public */,
       3,    0,   51,    2, 0x06,    6 /* Public */,
       4,    0,   52,    2, 0x06,    7 /* Public */,
       5,    0,   53,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   54,    2, 0x0a,    9 /* Public */,
       7,    1,   55,    2, 0x0a,   10 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,

 // properties: name, type, flags
      10, QMetaType::QString, 0x00015001, uint(0), 0,
      11, QMetaType::Int, 0x00015001, uint(1), 0,
      12, QMetaType::QString, 0x00015001, uint(2), 0,
      13, QMetaType::QString, 0x00015001, uint(3), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject infoProvider::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSinfoProviderENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSinfoProviderENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSinfoProviderENDCLASS_t,
        // property 'weatherInfo'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'temperature'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'localTime'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'currentDate'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<infoProvider, std::true_type>,
        // method 'weatherInfoChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'temperatureChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'localTimeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'currentDateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fetchWeatherData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onWeatherDataReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QNetworkReply *, std::false_type>
    >,
    nullptr
} };

void infoProvider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<infoProvider *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->weatherInfoChanged(); break;
        case 1: _t->temperatureChanged(); break;
        case 2: _t->localTimeChanged(); break;
        case 3: _t->currentDateChanged(); break;
        case 4: _t->fetchWeatherData(); break;
        case 5: _t->onWeatherDataReceived((*reinterpret_cast< std::add_pointer_t<QNetworkReply*>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QNetworkReply* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (infoProvider::*)();
            if (_t _q_method = &infoProvider::weatherInfoChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (infoProvider::*)();
            if (_t _q_method = &infoProvider::temperatureChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (infoProvider::*)();
            if (_t _q_method = &infoProvider::localTimeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (infoProvider::*)();
            if (_t _q_method = &infoProvider::currentDateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<infoProvider *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->getWeatherInfo(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->getTemperature(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->getLocalTime(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->getCurrentDate(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *infoProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *infoProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSinfoProviderENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int infoProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void infoProvider::weatherInfoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void infoProvider::temperatureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void infoProvider::localTimeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void infoProvider::currentDateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
