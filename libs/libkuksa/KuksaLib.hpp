#pragma once

#include <cmath>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <atomic>
#include <string>
#include <memory>
#include <type_traits>
#include <iostream>

#include "val.grpc.pb.h"
#include "types.pb.h"

using kuksa::val::v2::VAL;

class kuksaLib
{
private:
    // Base vehicle signals
    std::atomic<float> throttle{0.0f};
    std::atomic<float> steering{0.0f};
    std::atomic<int> gear{0};
    std::atomic<float> battery{0.0f};
    std::atomic<float> speed{0.0f};
    std::atomic<int> brake{0};
    std::atomic<int> drivingMode{0};

    // ADAS - Lane Keep Assist
    std::atomic<bool> lkaEnabled{false};
    std::atomic<bool> lkaActive{false};
    std::atomic<float> lkaTargetSteeringAngle{0.0f};
    std::atomic<float> lkaLaneOffset{0.0f};

    // ADAS - Lane Departure Warning
    std::atomic<bool> ldwEnabled{false};
    std::atomic<bool> ldwWarning{false};

    // ADAS - Cruise Control
    std::atomic<bool> ccEnabled{false};
    std::atomic<bool> ccActive{false};
    std::atomic<float> ccTargetSpeed{0.0f};

    // ADAS - Adaptive Cruise Control
    std::atomic<bool> accEnabled{false};
    std::atomic<bool> accActive{false};
    std::atomic<float> accTargetSpeed{0.0f};
    std::atomic<float> accTimeGap{0.0f};
    std::atomic<float> accLeadVehicleDistance{0.0f};

    // ADAS - Blind Spot Detection
    std::atomic<bool> bsdEnabled{false};
    std::atomic<bool> bsdLeftOccupied{false};
    std::atomic<bool> bsdRightOccupied{false};
    std::atomic<bool> bsdWarning{false};

    // ADAS - Traffic Sign Recognition
    std::atomic<bool> tsrEnabled{false};
    std::atomic<float> tsrDetectedSpeedLimit{0.0f};
    std::atomic<int> tsrDetectedSignType{0};

    // ADAS - Parking Assist
    std::atomic<bool> paEnabled{false};
    std::atomic<bool> paActive{false};
    std::atomic<float> paObstacleDistanceFront{0.0f};
    std::atomic<float> paObstacleDistanceRear{0.0f};

    std::string _server = "10.21.221.17:55555";
    std::thread _thread;
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<VAL::Stub> stub;

    // Internal setters
    void setThrottle(float t);
    void setSteering(float s);
    void setGear(int g);
    void setBattery(float b);
    void setSpeed(float s);
    void setBrake(int b);
    void setDrivingMode(int dm);

    void setLkaEnabled(bool v);
    void setLkaActive(bool v);
    void setLkaTargetSteeringAngle(float v);
    void setLkaLaneOffset(float v);

    void setLdwEnabled(bool v);
    void setLdwWarning(bool v);

    void setCcEnabled(bool v);
    void setCcActive(bool v);
    void setCcTargetSpeed(float v);

    void setAccEnabled(bool v);
    void setAccActive(bool v);
    void setAccTargetSpeed(float v);
    void setAccTimeGap(float v);
    void setAccLeadVehicleDistance(float v);

    void setBsdEnabled(bool v);
    void setBsdLeftOccupied(bool v);
    void setBsdRightOccupied(bool v);
    void setBsdWarning(bool v);

    void setTsrEnabled(bool v);
    void setTsrDetectedSpeedLimit(float v);
    void setTsrDetectedSignType(int v);

    void setPaEnabled(bool v);
    void setPaActive(bool v);
    void setPaObstacleDistanceFront(float v);
    void setPaObstacleDistanceRear(float v);

public:
    kuksaLib();
    ~kuksaLib();

	void asyncSub();

    bool subscribeFromKuksa();

    template<typename T>
    bool sendValueToKuksa(const std::string& path, const T& value);

    template<typename T>
    bool valueToType(const kuksa::val::v2::Value& v, T& out);

    // Base getters
    float getThrottle() const { return throttle.load(); }
    float getSteering() const { return steering.load(); }
    int getGear() const { return gear.load(); }
    float getBattery() const { return battery.load(); }
    float getSpeed() const { return speed.load(); }
    int getBrake() const { return brake.load(); }
    int getDrivingMode() const { return drivingMode.load(); }

    // ADAS getters
    bool getLkaEnabled() const { return lkaEnabled.load(); }
    bool getLkaActive() const { return lkaActive.load(); }
    float getLkaTargetSteeringAngle() const { return lkaTargetSteeringAngle.load(); }
    float getLkaLaneOffset() const { return lkaLaneOffset.load(); }

    bool getLdwEnabled() const { return ldwEnabled.load(); }
    bool getLdwWarning() const { return ldwWarning.load(); }

    bool getCcEnabled() const { return ccEnabled.load(); }
    bool getCcActive() const { return ccActive.load(); }
    float getCcTargetSpeed() const { return ccTargetSpeed.load(); }

    bool getAccEnabled() const { return accEnabled.load(); }
    bool getAccActive() const { return accActive.load(); }
    float getAccTargetSpeed() const { return accTargetSpeed.load(); }
    float getAccTimeGap() const { return accTimeGap.load(); }
    float getAccLeadVehicleDistance() const { return accLeadVehicleDistance.load(); }

    bool getBsdEnabled() const { return bsdEnabled.load(); }
    bool getBsdLeftOccupied() const { return bsdLeftOccupied.load(); }
    bool getBsdRightOccupied() const { return bsdRightOccupied.load(); }
    bool getBsdWarning() const { return bsdWarning.load(); }

    bool getTsrEnabled() const { return tsrEnabled.load(); }
    float getTsrDetectedSpeedLimit() const { return tsrDetectedSpeedLimit.load(); }
    int getTsrDetectedSignType() const { return tsrDetectedSignType.load(); }

    bool getPaEnabled() const { return paEnabled.load(); }
    bool getPaActive() const { return paActive.load(); }
    float getPaObstacleDistanceFront() const { return paObstacleDistanceFront.load(); }
    float getPaObstacleDistanceRear() const { return paObstacleDistanceRear.load(); }
};

template<typename T>
bool kuksaLib::sendValueToKuksa(const std::string& path, const T& value)
{
    kuksa::val::v2::PublishValueRequest req;

    auto* sid = req.mutable_signal_id();
    sid->set_path(path);

    auto* dp = req.mutable_data_point();
    auto* val = dp->mutable_value();

    if constexpr (std::is_same_v<T, float>) {
        val->set_float_(value);
    } else if constexpr (std::is_same_v<T, double>) {
        val->set_double_(value);
    } else if constexpr (std::is_same_v<T, int>) {
        val->set_int32(value);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        val->set_int32(value);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        val->set_uint32(value);
    } else if constexpr (std::is_same_v<T, bool>) {
        val->set_bool_(value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        val->set_string(value);
    } else {
        std::cerr << "Unsupported type for sendValueToKuksa: " << path << "\n";
        return false;
    }

    grpc::ClientContext ctx;
    kuksa::val::v2::PublishValueResponse resp;
    auto status = stub->PublishValue(&ctx, req, &resp);

    if (!status.ok()) {
        std::cerr << "PublishValue failed: "
                  << status.error_code() << " - "
                  << status.error_message() << "\n";
    }

    return status.ok();
}

template<typename T>
bool kuksaLib::valueToType(const kuksa::val::v2::Value& v, T& out)
{
    using V = kuksa::val::v2::Value;

    switch (v.typed_value_case()) {
        case V::kInt32:
            out = static_cast<T>(v.int32());
            return true;
        case V::kInt64:
            out = static_cast<T>(v.int64());
            return true;
        case V::kUint32:
            out = static_cast<T>(v.uint32());
            return true;
        case V::kUint64:
            out = static_cast<T>(v.uint64());
            return true;
        case V::kFloat:
            out = static_cast<T>(v.float_());
            return true;
        case V::kDouble:
            out = static_cast<T>(v.double_());
            return true;
        case V::kBool:
            out = static_cast<T>(v.bool_());
            return true;
        case V::kString:
            if constexpr (std::is_same_v<T, std::string>) {
                out = v.string();
                return true;
            }
            out = T{};
            return false;
        case V::TYPED_VALUE_NOT_SET:
        default:
            out = T{};
            return false;
    }
}
