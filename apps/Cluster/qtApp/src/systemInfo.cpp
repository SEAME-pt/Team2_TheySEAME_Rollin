#include "systemInfo.hpp"
#include <algorithm>

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

int systemInfo::metersToDisplayPercent(float meters)
{
    if (meters <= 0.f)
        return 0;
    // Match bridge LUT range (~8–80 m); keep some headroom past typical track distances.
    constexpr float kMaxMeters = 60.f;
    const float pct = (1.f - (meters / kMaxMeters)) * 100.f;
    return static_cast<int>(std::clamp(pct, 0.f, 100.f));
}

bool systemInfo::getLiveDetectionActive() const { return _liveDetectionActive.load(); }
bool systemInfo::getFrontCarVisible() const { return _frontCarVisible.load(); }
bool systemInfo::getLeftCarVisible() const { return _leftCarVisible.load(); }
bool systemInfo::getRightCarVisible() const { return _rightCarVisible.load(); }
int systemInfo::getFrontCarDistance() const { return _frontCarDistance.load(); }
int systemInfo::getLeftCarDistance() const { return _leftCarDistance.load(); }
int systemInfo::getRightCarDistance() const { return _rightCarDistance.load(); }
double systemInfo::getFrontCarOrientation() const { return _frontCarOrientation.load(); }
double systemInfo::getLeftCarOrientation() const { return _leftCarOrientation.load(); }
double systemInfo::getRightCarOrientation() const { return _rightCarOrientation.load(); }
double systemInfo::getFrontCarLateral() const { return _frontCarLateral.load(); }
double systemInfo::getLeftCarLateral() const { return _leftCarLateral.load(); }
double systemInfo::getRightCarLateral() const { return _rightCarLateral.load(); }

bool systemInfo::getLdwWarningActive() const { return _ldwWarningActive; }
bool systemInfo::getBsdWarningActive() const { return _bsdWarningActive; }
bool systemInfo::getAdasWarningVisible() const { return _adasWarningVisible; }
QString systemInfo::getAdasWarningMessage() const { return _adasWarningMessage; }

namespace {

QString buildAdasWarningMessage(bool ldw, bool bsd)
{
    if (ldw && bsd)
        return QStringLiteral("Lane Departure + Blind Spot");
    if (ldw)
        return QStringLiteral("Lane Departure");
    if (bsd)
        return QStringLiteral("Blind Spot");
    return QString();
}

} // namespace

void systemInfo::updateAdasWarnings()
{
    const bool ldw = _kuksa.getLdwWarning();
    const bool bsd = _kuksa.getBsdWarning();
    const bool visible = ldw || bsd;
    const QString message = buildAdasWarningMessage(ldw, bsd);
    const bool wasVisible = _adasWarningVisible;

    bool changed = false;
    if (_ldwWarningActive != ldw) {
        _ldwWarningActive = ldw;
        changed = true;
    }
    if (_bsdWarningActive != bsd) {
        _bsdWarningActive = bsd;
        changed = true;
    }
    if (_adasWarningVisible != visible) {
        _adasWarningVisible = visible;
        changed = true;
    }
    if (_adasWarningMessage != message) {
        _adasWarningMessage = message;
        changed = true;
    }

    if (!wasVisible && visible)
        emit adasWarningTriggered(message);
    else if (wasVisible && !visible)
        emit adasWarningCleared();

    if (changed)
        emit adasWarningUpdated();
}

void systemInfo::updateVehicleDetection()
{
    const float frontM = _kuksa.getAccLeadVehicleDistance();
    const float leftM = _kuksa.getBsdLeftDistance();
    const float rightM = _kuksa.getBsdRightDistance();

    const bool frontVisible = frontM > 0.5f;
    const bool leftVisible = _kuksa.getBsdLeftOccupied() && leftM > 0.5f;
    const bool rightVisible = _kuksa.getBsdRightOccupied() && rightM > 0.5f;
    const bool liveActive = frontVisible || leftVisible || rightVisible;

    const int frontPct = frontVisible ? metersToDisplayPercent(frontM) : 0;
    const int leftPct = leftVisible ? metersToDisplayPercent(leftM) : 0;
    const int rightPct = rightVisible ? metersToDisplayPercent(rightM) : 0;
    const double frontOrient = frontVisible ? static_cast<double>(_kuksa.getAccLeadVehicleOrientation()) : 180.0;
    const double leftOrient = leftVisible ? static_cast<double>(_kuksa.getBsdLeftVehicleOrientation()) : 180.0;
    const double rightOrient = rightVisible ? static_cast<double>(_kuksa.getBsdRightVehicleOrientation()) : 180.0;
    const double frontLat = frontVisible ? static_cast<double>(_kuksa.getAccLeadVehicleLateralOffset()) : 0.0;
    const double leftLat = leftVisible ? static_cast<double>(_kuksa.getBsdLeftVehicleLateralOffset()) : 0.0;
    const double rightLat = rightVisible ? static_cast<double>(_kuksa.getBsdRightVehicleLateralOffset()) : 0.0;

    bool changed = false;
    if (_liveDetectionActive != liveActive) { _liveDetectionActive = liveActive; changed = true; }
    if (_frontCarVisible != frontVisible) { _frontCarVisible = frontVisible; changed = true; }
    if (_leftCarVisible != leftVisible) { _leftCarVisible = leftVisible; changed = true; }
    if (_rightCarVisible != rightVisible) { _rightCarVisible = rightVisible; changed = true; }
    if (_frontCarDistance != frontPct) { _frontCarDistance = frontPct; changed = true; }
    if (_leftCarDistance != leftPct) { _leftCarDistance = leftPct; changed = true; }
    if (_rightCarDistance != rightPct) { _rightCarDistance = rightPct; changed = true; }
    if (_frontCarOrientation != frontOrient) { _frontCarOrientation = frontOrient; changed = true; }
    if (_leftCarOrientation != leftOrient) { _leftCarOrientation = leftOrient; changed = true; }
    if (_rightCarOrientation != rightOrient) { _rightCarOrientation = rightOrient; changed = true; }
    if (_frontCarLateral != frontLat) { _frontCarLateral = frontLat; changed = true; }
    if (_leftCarLateral != leftLat) { _leftCarLateral = leftLat; changed = true; }
    if (_rightCarLateral != rightLat) { _rightCarLateral = rightLat; changed = true; }

    if (changed)
        emit vehicleDetectionUpdated();
}

bool systemInfo::start()
{
    if (_running) return true;
    _running = true;

    _thread = std::thread([this]() {
        const bool skipSubscribe = qEnvironmentVariableIntValue("DISABLE_KUKSA_SUBSCRIBE") == 1;
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        std::unique_ptr<std::thread> subThread;
        if (!skipSubscribe) {
            subThread = std::make_unique<std::thread>([this]() {
                if (!_kuksa.subscribeFromKuksa()) {
                    qWarning() << "Failed to subscribe to Kuksa";
                }
            });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        while (_running) {
            setSpeed(static_cast<int>(_kuksa.getSpeed()));
            setBattery(static_cast<int>(_kuksa.getBattery()));
            setCruiseActive(_kuksa.getCcActive());
            setTargetSpeed(static_cast<int>(_kuksa.getCcTargetSpeed()));
            emit trafficSignUpdated(static_cast<int>(_kuksa.getTsrDetectedSignType()));
            emit speedLimitUpdated(static_cast<int>(_kuksa.getTsrDetectedSpeedLimit()));
            updateVehicleDetection();
            updateAdasWarnings();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

