#include "systemInfo.hpp"

systemInfo::systemInfo(QObject *parent)
    : QObject(parent)
{
    
}

systemInfo::~systemInfo()
{
    _running = false;
    if (_thread.joinable()) {
        _thread.join();
    }
}

void systemInfo::setBattery(int battery)
{
    if (_battery == battery) return;
        _battery = battery;
    emit batteryUpdated(battery);
}

void systemInfo::setSpeed(int speed)
{
    if (_speed == speed) return;
        _speed = speed;
    emit speedUpdated(speed);
}

int systemInfo::getBattery() const
{
    return _battery;
}

int systemInfo::getSpeed() const
{
    return _speed;
}

void systemInfo::setCruiseActive(bool active)
{
    if (_cruiseActive == active) return;
    _cruiseActive = active;
    emit cruiseActiveUpdated(active);
    emit targetSpeedUpdated(_targetSpeed);
}

bool systemInfo::getCruiseActive() const
{
    return _cruiseActive;
}

void systemInfo::setTargetSpeed(int speed)
{
    if (_targetSpeed == speed) return;
    _targetSpeed = speed;
    emit targetSpeedUpdated(speed);
}

int systemInfo::getTargetSpeed() const
{
    return _targetSpeed;
}

QString systemInfo::getTargetSpeedDisplay() const
{
    if (!_cruiseActive) return "---";
    return QString::number(_targetSpeed) + " hm/h";
}

bool systemInfo::start()
{
    if (_running) return true;
    _running = true;

    _thread = std::thread([this]() {
        const QString platform = qEnvironmentVariable("QT_QPA_PLATFORM");
        const bool skipSubscribe = qEnvironmentVariableIntValue("DISABLE_KUKSA_SUBSCRIBE") == 1;

        std::unique_ptr<std::thread> subThread;
        if (!skipSubscribe) {
            subThread = std::make_unique<std::thread>([this]() {
                if (!_kuksa.subscribeFromKuksa()) {
                    qWarning() << "Failed to subscribe to Kuksa";
                }
            });
        } else {
            qWarning() << "Skipping Kuksa subscribe (DISABLE_KUKSA_SUBSCRIBE=1). Platform:" << platform;
        }

        while (_running) {
            setSpeed(static_cast<int>(_kuksa.getSpeed()));
            setBattery(static_cast<int>(_kuksa.getBattery()));
            setCruiseActive(_kuksa.getCcActive());
            setTargetSpeed(static_cast<int>(_kuksa.getCcTargetSpeed()));

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        if (subThread && subThread->joinable()) {
            subThread->join();
        }
    });

    return true;
}

bool systemInfo::valueToInt(const kuksa::val::v2::Value& v, int& out)
{
    using V = kuksa::val::v2::Value;

    switch (v.typed_value_case()) {
    case V::kInt32:  out = v.int32(); return true;
    case V::kInt64:  out = static_cast<int>(v.int64()); return true;
    case V::kUint32: out = static_cast<int>(v.uint32()); return true;
    case V::kUint64: out = static_cast<int>(v.uint64()); return true;

    case V::kFloat:  out = static_cast<int>(std::lround(v.float_())); return true;
    case V::kDouble: out = static_cast<int>(std::lround(v.double_())); return true;

    case V::kBool:   out = v.bool_() ? 1 : 0; return true;

    case V::TYPED_VALUE_NOT_SET:
    default:
        return false;
    }
}

