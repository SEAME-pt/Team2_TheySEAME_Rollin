#include "KuksaLib.hpp"

kuksaLib::kuksaLib()
{
    channel = grpc::CreateChannel(_server, grpc::InsecureChannelCredentials());
    stub = VAL::NewStub(channel);
}

kuksaLib::~kuksaLib()
{
}

// Base setters
void kuksaLib::setThrottle(float t) { throttle.store(t); }
void kuksaLib::setSteering(float s) { steering.store(s); }
void kuksaLib::setGear(int g) { gear.store(g); }
void kuksaLib::setBattery(float b) { battery.store(b); }
void kuksaLib::setSpeed(float s) { speed.store(s); }
void kuksaLib::setBrake(int b) { brake.store(b); }
void kuksaLib::setDrivingMode(int dm) { drivingMode.store(dm); }

// LKA setters
void kuksaLib::setLkaEnabled(bool v) { lkaEnabled.store(v); }
void kuksaLib::setLkaActive(bool v) { lkaActive.store(v); }
void kuksaLib::setLkaTargetSteeringAngle(float v) { lkaTargetSteeringAngle.store(v); }
void kuksaLib::setLkaLaneOffset(float v) { lkaLaneOffset.store(v); }

// LDW setters
void kuksaLib::setLdwEnabled(bool v) { ldwEnabled.store(v); }
void kuksaLib::setLdwWarning(bool v) { ldwWarning.store(v); }

// CC setters
void kuksaLib::setCcEnabled(bool v) { ccEnabled.store(v); }
void kuksaLib::setCcActive(bool v) { ccActive.store(v); }
void kuksaLib::setCcTargetSpeed(float v) { ccTargetSpeed.store(v); }

// ACC setters
void kuksaLib::setAccEnabled(bool v) { accEnabled.store(v); }
void kuksaLib::setAccActive(bool v) { accActive.store(v); }
void kuksaLib::setAccTargetSpeed(float v) { accTargetSpeed.store(v); }
void kuksaLib::setAccTimeGap(float v) { accTimeGap.store(v); }
void kuksaLib::setAccLeadVehicleDistance(float v) { accLeadVehicleDistance.store(v); }

// BSD setters
void kuksaLib::setBsdEnabled(bool v) { bsdEnabled.store(v); }
void kuksaLib::setBsdLeftOccupied(bool v) { bsdLeftOccupied.store(v); }
void kuksaLib::setBsdRightOccupied(bool v) { bsdRightOccupied.store(v); }
void kuksaLib::setBsdWarning(bool v) { bsdWarning.store(v); }

// TSR setters
void kuksaLib::setTsrEnabled(bool v) { tsrEnabled.store(v); }
void kuksaLib::setTsrDetectedSpeedLimit(float v) { tsrDetectedSpeedLimit.store(v); }
void kuksaLib::setTsrDetectedSignType(int v) { tsrDetectedSignType.store(v); }

// PA setters
void kuksaLib::setPaEnabled(bool v) { paEnabled.store(v); }
void kuksaLib::setPaActive(bool v) { paActive.store(v); }
void kuksaLib::setPaObstacleDistanceFront(float v) { paObstacleDistanceFront.store(v); }
void kuksaLib::setPaObstacleDistanceRear(float v) { paObstacleDistanceRear.store(v); }

bool kuksaLib::subscribeFromKuksa()
{
    kuksa::val::v2::SubscribeRequest req;

    // Base signals
    req.add_signal_paths("Vehicle.Control.Throttle.Value");
    req.add_signal_paths("Vehicle.Control.Steering.Angle");
    req.add_signal_paths("Vehicle.Control.Gear.Value");
    req.add_signal_paths("Vehicle.Control.Brake.Value");
    req.add_signal_paths("Vehicle.Powertrain.Battery.StateOfCharge");
    req.add_signal_paths("Vehicle.Speed");
    req.add_signal_paths("Vehicle.Control.Mode.DrivingMode");

    // ADAS - LKA
    req.add_signal_paths("Vehicle.ADAS.LaneKeepAssist.Enabled");
    req.add_signal_paths("Vehicle.ADAS.LaneKeepAssist.Active");
    req.add_signal_paths("Vehicle.ADAS.LaneKeepAssist.TargetSteeringAngle");
    req.add_signal_paths("Vehicle.ADAS.LaneKeepAssist.LaneOffset");

    // ADAS - LDW
    req.add_signal_paths("Vehicle.ADAS.LaneDepartureWarning.Enabled");
    req.add_signal_paths("Vehicle.ADAS.LaneDepartureWarning.Warning");

    // ADAS - CC
    req.add_signal_paths("Vehicle.ADAS.CruiseControl.Enabled");
    req.add_signal_paths("Vehicle.ADAS.CruiseControl.Active");
    req.add_signal_paths("Vehicle.ADAS.CruiseControl.TargetSpeed");

    // ADAS - ACC
    req.add_signal_paths("Vehicle.ADAS.AdaptiveCruiseControl.Enabled");
    req.add_signal_paths("Vehicle.ADAS.AdaptiveCruiseControl.Active");
    req.add_signal_paths("Vehicle.ADAS.AdaptiveCruiseControl.TargetSpeed");
    req.add_signal_paths("Vehicle.ADAS.AdaptiveCruiseControl.TimeGap");
    req.add_signal_paths("Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleDistance");

    // ADAS - BSD
    req.add_signal_paths("Vehicle.ADAS.BlindSpotDetection.Enabled");
    req.add_signal_paths("Vehicle.ADAS.BlindSpotDetection.LeftOccupied");
    req.add_signal_paths("Vehicle.ADAS.BlindSpotDetection.RightOccupied");
    req.add_signal_paths("Vehicle.ADAS.BlindSpotDetection.Warning");

    // ADAS - TSR
    req.add_signal_paths("Vehicle.ADAS.TrafficSignRecognition.Enabled");
    req.add_signal_paths("Vehicle.ADAS.TrafficSignRecognition.DetectedSpeedLimit");
    req.add_signal_paths("Vehicle.ADAS.TrafficSignRecognition.DetectedSignType");

    // ADAS - Parking Assist
    req.add_signal_paths("Vehicle.ADAS.ParkingAssist.Enabled");
    req.add_signal_paths("Vehicle.ADAS.ParkingAssist.Active");
    req.add_signal_paths("Vehicle.ADAS.ParkingAssist.ObstacleDistanceFront");
    req.add_signal_paths("Vehicle.ADAS.ParkingAssist.ObstacleDistanceRear");

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

            const auto& value = datapoint.value();

            // Base signals
            if (path == "Vehicle.Control.Throttle.Value") {
                float v = 0.0f;
                if (valueToType(value, v)) setThrottle(v);
            } else if (path == "Vehicle.Control.Steering.Angle") {
                float v = 0.0f;
                if (valueToType(value, v)) setSteering(v);
            } else if (path == "Vehicle.Control.Gear.Value") {
                int v = 0;
                if (valueToType(value, v)) setGear(v);
            } else if (path == "Vehicle.Control.Brake.Value") {
                int v = 0;
                if (valueToType(value, v)) setBrake(v);
            } else if (path == "Vehicle.Powertrain.Battery.StateOfCharge") {
                float v = 0.0f;
                if (valueToType(value, v)) setBattery(v);
            } else if (path == "Vehicle.Speed") {
                float v = 0.0f;
                if (valueToType(value, v)) setSpeed(v);
            } else if (path == "Vehicle.Control.Mode.DrivingMode") {
                int v = 0;
                if (valueToType(value, v)) setDrivingMode(v);

            // LKA
            } else if (path == "Vehicle.ADAS.LaneKeepAssist.Enabled") {
                bool v = false;
                if (valueToType(value, v)) setLkaEnabled(v);
            } else if (path == "Vehicle.ADAS.LaneKeepAssist.Active") {
                bool v = false;
                if (valueToType(value, v)) setLkaActive(v);
            } else if (path == "Vehicle.ADAS.LaneKeepAssist.TargetSteeringAngle") {
                float v = 0.0f;
                if (valueToType(value, v)) setLkaTargetSteeringAngle(v);
            } else if (path == "Vehicle.ADAS.LaneKeepAssist.LaneOffset") {
                float v = 0.0f;
                if (valueToType(value, v)) setLkaLaneOffset(v);

            // LDW
            } else if (path == "Vehicle.ADAS.LaneDepartureWarning.Enabled") {
                bool v = false;
                if (valueToType(value, v)) setLdwEnabled(v);
            } else if (path == "Vehicle.ADAS.LaneDepartureWarning.Warning") {
                bool v = false;
                if (valueToType(value, v)) setLdwWarning(v);

            // CC
            } else if (path == "Vehicle.ADAS.CruiseControl.Enabled") {
                bool v = false;
                if (valueToType(value, v)) setCcEnabled(v);
            } else if (path == "Vehicle.ADAS.CruiseControl.Active") {
                bool v = false;
                if (valueToType(value, v)) setCcActive(v);
            } else if (path == "Vehicle.ADAS.CruiseControl.TargetSpeed") {
                float v = 0.0f;
                if (valueToType(value, v)) setCcTargetSpeed(v);

            // ACC
            } else if (path == "Vehicle.ADAS.AdaptiveCruiseControl.Enabled") {
                bool v = false;
                if (valueToType(value, v)) setAccEnabled(v);
            } else if (path == "Vehicle.ADAS.AdaptiveCruiseControl.Active") {
                bool v = false;
                if (valueToType(value, v)) setAccActive(v);
            } else if (path == "Vehicle.ADAS.AdaptiveCruiseControl.TargetSpeed") {
                float v = 0.0f;
                if (valueToType(value, v)) setAccTargetSpeed(v);
            } else if (path == "Vehicle.ADAS.AdaptiveCruiseControl.TimeGap") {
                float v = 0.0f;
                if (valueToType(value, v)) setAccTimeGap(v);
            } else if (path == "Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleDistance") {
                float v = 0.0f;
                if (valueToType(value, v)) setAccLeadVehicleDistance(v);

            // BSD
            } else if (path == "Vehicle.ADAS.BlindSpotDetection.Enabled") {
                bool v = false;
                if (valueToType(value, v)) setBsdEnabled(v);
            } else if (path == "Vehicle.ADAS.BlindSpotDetection.LeftOccupied") {
                bool v = false;
                if (valueToType(value, v)) setBsdLeftOccupied(v);
            } else if (path == "Vehicle.ADAS.BlindSpotDetection.RightOccupied") {
                bool v = false;
                if (valueToType(value, v)) setBsdRightOccupied(v);
            } else if (path == "Vehicle.ADAS.BlindSpotDetection.Warning") {
                bool v = false;
                if (valueToType(value, v)) setBsdWarning(v);

            // TSR
            } else if (path == "Vehicle.ADAS.TrafficSignRecognition.Enabled") {
                bool v = false;
                if (valueToType(value, v)) setTsrEnabled(v);
            } else if (path == "Vehicle.ADAS.TrafficSignRecognition.DetectedSpeedLimit") {
                float v = 0.0f;
                if (valueToType(value, v)) setTsrDetectedSpeedLimit(v);
            } else if (path == "Vehicle.ADAS.TrafficSignRecognition.DetectedSignType") {
                int v = 0;
                if (valueToType(value, v)) setTsrDetectedSignType(v);

            // Parking Assist
            } else if (path == "Vehicle.ADAS.ParkingAssist.Enabled") {
                bool v = false;
                if (valueToType(value, v)) setPaEnabled(v);
            } else if (path == "Vehicle.ADAS.ParkingAssist.Active") {
                bool v = false;
                if (valueToType(value, v)) setPaActive(v);
            } else if (path == "Vehicle.ADAS.ParkingAssist.ObstacleDistanceFront") {
                float v = 0.0f;
                if (valueToType(value, v)) setPaObstacleDistanceFront(v);
            } else if (path == "Vehicle.ADAS.ParkingAssist.ObstacleDistanceRear") {
                float v = 0.0f;
                if (valueToType(value, v)) setPaObstacleDistanceRear(v);
            }
        }
    }

    auto status = stream->Finish();
    if (!status.ok()) {
        std::cerr << "Kuksa stream finished with error: "
                  << status.error_code() << " - "
                  << status.error_message() << std::endl;
        return false;
    }

    return true;
}