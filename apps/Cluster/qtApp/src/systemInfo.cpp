#include "systemInfo.hpp"
#include <algorithm>
#include <cstdlib>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

systemInfo::systemInfo(QObject *parent)
    : QObject(parent)
{
    
}

systemInfo::~systemInfo()
{
    _running = false;
    if (_mqttClient)
        _mqttClient->stop();
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
    constexpr float kMaxMeters = 30.f;
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

bool systemInfo::getLdwWarningActive() const { return _ldwWarningActive; }
bool systemInfo::getBsdWarningActive() const { return _bsdWarningActive; }
bool systemInfo::getAdasWarningVisible() const { return _adasWarningVisible; }
QString systemInfo::getAdasWarningMessage() const { return _adasWarningMessage; }
bool systemInfo::getAdasWarningTimed() const { return _adasWarningTimed; }
int systemInfo::getAdasWarningRemainingMs() const { return _adasWarningRemainingMs; }
int systemInfo::getAdasWarningEpoch() const { return _adasWarningEpoch; }

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

QString hazardTypeToMessage(const QString &type)
{
    if (type == QStringLiteral("stopped_car"))
        return QStringLiteral("Stopped Car");
    if (type == QStringLiteral("stopped_obstacles"))
        return QStringLiteral("Object On Track");
    if (type == QStringLiteral("two_stopped_cars"))
        return QStringLiteral("Two Stopped Cars");
    if (type.isEmpty())
        return QString();

    QString out = type;
    out.replace('_', ' ');
    if (!out.isEmpty())
        out[0] = out[0].toUpper();
    return out;
}

QString topicToMobilityMessage(const std::string &topic)
{
    const QString qtopic = QString::fromStdString(topic);
    const int slash = qtopic.lastIndexOf('/');
    const QString leaf = slash >= 0 ? qtopic.mid(slash + 1) : qtopic;
    if (leaf.isEmpty())
        return QString();
    return hazardTypeToMessage(leaf);
}

QString parseMobilityPayload(const std::string &topic, const std::string &body, int *outMarkerId)
{
    if (outMarkerId)
        *outMarkerId = -1;

    const QByteArray raw = QByteArray::fromStdString(body);
    if (!raw.trimmed().isEmpty()) {
        QJsonParseError err{};
        const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("marker_id"))) {
                const int markerId = obj.value(QStringLiteral("marker_id")).toInt(-1);
                if (outMarkerId && markerId >= 0)
                    *outMarkerId = markerId;
            }
            const QString message = obj.value(QStringLiteral("message")).toString();
            if (!message.isEmpty())
                return message;
            const QString description = obj.value(QStringLiteral("description")).toString();
            if (!description.isEmpty())
                return description;
            const QString type = obj.value(QStringLiteral("type")).toString();
            if (!type.isEmpty()) {
                QString out = hazardTypeToMessage(type);
                if (obj.contains(QStringLiteral("marker_id"))) {
                    const int markerId = obj.value(QStringLiteral("marker_id")).toInt(-1);
                    if (markerId >= 0)
                        out += QStringLiteral(" (marker %1)").arg(markerId);
                }
                return out;
            }
        }
        if (!raw.trimmed().isEmpty())
            return QString::fromUtf8(raw.trimmed());
    }
    return topicToMobilityMessage(topic);
}

/** Circular distance on ArUco DICT_4X4_50 (ids 0..49). */
int markerDistance(int a, int b)
{
    if (a < 0 || b < 0)
        return 999;
    const int span = 50;
    const int d = std::abs(a - b);
    return std::min(d, span - d);
}

} // namespace

void systemInfo::handleMobilityMqttMessage(const std::string &topic, const std::string &body)
{
    int hazardMarkerId = -1;
    const QString message = parseMobilityPayload(topic, body, &hazardMarkerId);
    if (message.isEmpty())
        return;

    std::lock_guard<std::mutex> lock(_mobilityMutex);
    _pendingHazardValid = true;
    _pendingHazardMarkerId = hazardMarkerId;
    _pendingHazardMessage = message;
    _pendingHazardExpires = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(kPendingHazardTtlMs);
    // Do not show yet — wait until ego ArUco marker is close.
    _wasNearHazard = false;
    _mobilityHazardActive = false;

    qInfo() << "Mobility hazard pending at marker" << hazardMarkerId
            << "ego marker" << _kuksa.getMobilityMarkerId()
            << "msg" << message;
}

void systemInfo::startMobilityMqtt()
{
    if (qEnvironmentVariableIntValue("DISABLE_MQTT") == 1)
        return;

    QByteArray hostEnv = qgetenv("MQTT_HOST");
    if (hostEnv.isEmpty())
        hostEnv = qgetenv("MQTT_BROKER_HOST");
    // Default matches CarControlMQTT / feature/490 hazard publisher broker on fleet Pi.
    const std::string host = hostEnv.isEmpty() ? std::string("10.21.100.3") : hostEnv.constData();
    const int port = qEnvironmentVariableIsSet("MQTT_PORT") ? qgetenv("MQTT_PORT").toInt() : 1883;
    const QByteArray clientEnv = qgetenv("MQTT_CLIENT_ID");
    const std::string clientId = clientEnv.isEmpty() ? std::string("cluster-qtAppExec") : clientEnv.constData();

    _mqttClient = std::make_unique<MqttHazardClient>();
    _mqttClient->start(host, port, clientId, [this](const std::string &topic, const std::string &body) {
        handleMobilityMqttMessage(topic, body);
    });
    qInfo() << "Mobility MQTT subscriber started on" << host.c_str() << port;
}

void systemInfo::updateAdasWarnings()
{
    const bool ldw = _kuksa.getLdwWarning();
    const bool bsd = _kuksa.getBsdWarning();
    const int egoMarker = _kuksa.getMobilityMarkerId();
    const int proximity = qEnvironmentVariableIsSet("HAZARD_MARKER_PROXIMITY")
        ? qgetenv("HAZARD_MARKER_PROXIMITY").toInt()
        : kDefaultMarkerProximity;

    bool mobilityActive = false;
    QString mobilityMessage;
    {
        std::lock_guard<std::mutex> lock(_mobilityMutex);
        const auto now = std::chrono::steady_clock::now();

        if (_pendingHazardValid && now >= _pendingHazardExpires) {
            _pendingHazardValid = false;
            _pendingHazardMessage.clear();
            _pendingHazardMarkerId = -1;
            _wasNearHazard = false;
            _mobilityHazardActive = false;
        }

        if (_pendingHazardValid) {
            const bool near = (_pendingHazardMarkerId < 0)
                ? true // no marker in MQTT → treat as global alert
                : (egoMarker >= 0 && markerDistance(egoMarker, _pendingHazardMarkerId) <= proximity);

            if (near) {
                if (!_wasNearHazard) {
                    // Just entered proximity — start popup countdown.
                    _mobilityHazardActive = true;
                    _mobilityHazardMessage = _pendingHazardMessage;
                    _mobilityHazardUntil = now + std::chrono::milliseconds(kMobilityHazardDurationMs);
                    ++_adasWarningEpoch;
                    _wasNearHazard = true;
                    qInfo() << "Mobility popup: ego marker" << egoMarker
                            << "near hazard marker" << _pendingHazardMarkerId;
                }
            } else {
                // Still far from crash — keep pending, hide popup.
                if (_wasNearHazard) {
                    qInfo() << "Mobility popup cleared: ego" << egoMarker
                            << "left hazard marker" << _pendingHazardMarkerId;
                }
                _wasNearHazard = false;
                _mobilityHazardActive = false;
            }
        }

        if (_mobilityHazardActive && now < _mobilityHazardUntil) {
            mobilityActive = true;
            mobilityMessage = _mobilityHazardMessage;
        } else if (_mobilityHazardActive) {
            // Countdown finished while still near — clear display but keep pending
            // until we leave the area (so we don't immediately re-trigger).
            _mobilityHazardActive = false;
            _mobilityHazardMessage.clear();
        }
    }

    const bool visible = ldw || bsd || mobilityActive;
    const bool timed = mobilityActive;
    int remainingMs = 0;
    if (mobilityActive) {
        remainingMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
            _mobilityHazardUntil - std::chrono::steady_clock::now()).count());
        if (remainingMs < 0)
            remainingMs = 0;
    }

    QString message;
    if (mobilityActive && (ldw || bsd)) {
        message = mobilityMessage + QStringLiteral(" + ") + buildAdasWarningMessage(ldw, bsd);
    } else if (mobilityActive) {
        message = mobilityMessage;
    } else {
        message = buildAdasWarningMessage(ldw, bsd);
    }

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
    if (_adasWarningTimed != timed) {
        _adasWarningTimed = timed;
        changed = true;
    }
    if (_adasWarningRemainingMs != remainingMs) {
        _adasWarningRemainingMs = remainingMs;
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

    bool changed = false;
    if (_liveDetectionActive != liveActive) { _liveDetectionActive = liveActive; changed = true; }
    if (_frontCarVisible != frontVisible) { _frontCarVisible = frontVisible; changed = true; }
    if (_leftCarVisible != leftVisible) { _leftCarVisible = leftVisible; changed = true; }
    if (_rightCarVisible != rightVisible) { _rightCarVisible = rightVisible; changed = true; }
    if (_frontCarDistance != frontPct) { _frontCarDistance = frontPct; changed = true; }
    if (_leftCarDistance != leftPct) { _leftCarDistance = leftPct; changed = true; }
    if (_rightCarDistance != rightPct) { _rightCarDistance = rightPct; changed = true; }

    if (changed)
        emit vehicleDetectionUpdated();
}

bool systemInfo::start()
{
    if (_running) return true;
    _running = true;
    startMobilityMqtt();

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

