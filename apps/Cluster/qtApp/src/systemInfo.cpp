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
}

bool systemInfo::getCruiseActive() const
{
    return _cruiseActive;
}

void systemInfo::setLeftCarDistance(int distance)
{
    if (_leftCarDistance == distance) return;
    _leftCarDistance = distance;
    emit leftCarDistanceUpdated(distance);
}

int systemInfo::getLeftCarDistance() const
{
    return _leftCarDistance;
}

bool systemInfo::start()
{
    if (_running) return true;
    _running = true;

    _thread = std::thread([this]() {
        this->updateFromKuksa();
    });

    _thread.detach();
    return true;
}

bool systemInfo::updateFromKuksa()
{
    auto channel = grpc::CreateChannel(_server.toStdString(), grpc::InsecureChannelCredentials());
    std::unique_ptr<VAL::Stub> stub = VAL::NewStub(channel);

    kuksa::val::v2::SubscribeRequest req;

    req.add_signal_paths("Vehicle.Speed");
    req.add_signal_paths("Vehicle.Powertrain.Battery.StateOfCharge");

    grpc::ClientContext ctx;
    auto stream = stub->Subscribe(&ctx, req);

    kuksa::val::v2::SubscribeResponse resp;
    while (stream->Read(&resp)) {

        const auto& entries = resp.entries();
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            const std::string& path = it->first;
            const auto& datapoint = it->second;

            if (!datapoint.has_value()) {
                qDebug() << "[READER]" << QString::fromStdString(path) << "= <no value>";
                continue;
            }

            int vInt = 0;
            if (!valueToInt(datapoint.value(), vInt)) {
                qWarning() << "[READER]" << QString::fromStdString(path) << "value type not int-compatible";
                continue;
            }

            if (path == "Vehicle.Speed") {
                setSpeed(vInt);
            } else if (path == "Vehicle.Powertrain.Battery.StateOfCharge") {
                setBattery(vInt);
            }
        }
    }

    auto status = stream->Finish();
    if (!status.ok()) {
        qWarning() << "gRPC Subscribe failed:" << QString::fromStdString(status.error_message());
        return false;
    }
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

