#include "KuksaLib.hpp"

kuksaLib::kuksaLib()
{
    channel = grpc::CreateChannel(_server, grpc::InsecureChannelCredentials());
    stub = VAL::NewStub(channel);
}

kuksaLib::~kuksaLib()
{
}

void kuksaLib::setThrottle(float t) {
    throttle.store(t);
}

void kuksaLib::setSteering(float s) {
    steering.store(s);
}

void kuksaLib::setGear(int g) {
    gear.store(g);
}

void kuksaLib::setBattery(float b) {
    battery.store(b);
}

void kuksaLib::setSpeed(float s) {
    speed.store(s);
}

void kuksaLib::setDrivingMode(int dm) {
    drivingMode.store(dm);
}

bool kuksaLib::subscribeFromKuksa()
{
    kuksa::val::v2::SubscribeRequest req;


    req.add_signal_paths("Vehicle.Control.Throttle.Value");
    req.add_signal_paths("Vehicle.Control.Steering.Angle");
    req.add_signal_paths("Vehicle.Control.Gear.Value");
    req.add_signal_paths("Vehicle.Powertrain.Battery.StateOfCharge");
    req.add_signal_paths("Vehicle.Speed");
    req.add_signal_paths("Vehicle.Control.Mode.DrivingMode");
    grpc::ClientContext ctx;
    auto stream = stub->Subscribe(&ctx, req);

    kuksa::val::v2::SubscribeResponse resp;
    while (stream->Read(&resp)) {

        const auto& entries = resp.entries();
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            const std::string& path = it->first;
            const auto& datapoint = it->second;

            if (!datapoint.has_value()) {
                continue;
            }

            float value = 0;
            if (!valueToType(datapoint.value(), value)) {
                continue;
            }

            if (path == "Vehicle.Control.Throttle.Value") {
                setThrottle(value);
            } else if (path == "Vehicle.Control.Steering.Angle") {
                setSteering(value);
            }
            else if (path == "Vehicle.Control.Gear.Value") {
                setGear(value);
            }
            else if (path == "Vehicle.Powertrain.Battery.StateOfCharge") {
                setBattery(value);
            }
            else if (path == "Vehicle.Speed") {
                setSpeed(value);
            }
            else if (path == "Vehicle.Control.Mode.DrivingMode") {
                setDrivingMode(value);
            }
        }
    }

    auto status = stream->Finish();
    if (!status.ok()) {
        std::cerr << "Kuksa stream finished with error: " << status.error_code() << " - " << status.error_message() << std::endl;
        return false;
    }
    return true;
}
