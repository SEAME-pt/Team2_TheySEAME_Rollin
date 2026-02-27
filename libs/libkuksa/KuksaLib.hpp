#include <cmath> 
#include <grpcpp/grpcpp.h>
#include <thread>
#include <atomic>
#include <string>
#include <type_traits>
#include "val.grpc.pb.h"
#include "types.pb.h"

using kuksa::val::v2::VAL;

class kuksaLib
{
private:
    std::atomic<float> throttle{0};
    std::atomic<float> steering{0};
    std::atomic<int> gear{0};
    std::atomic<float> battery{0};
    std::atomic<float> speed{0};
    std::atomic<int> drivingMode{0};
    std::string _server = "10.21.221.17:55555";
    std::thread _thread;
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<VAL::Stub> stub;

    void setThrottle(float t);
    void setSteering(float s);
    void setGear(int g);
    void setBattery(float b);
    void setSpeed(float s);
    void setDrivingMode(int dm);
public:
    kuksaLib();
    ~kuksaLib();

    bool subscribeFromKuksa();
    template<typename T>
    bool sendValueToKuksa(const std::string& path, const T& value);
    template<typename T>
    bool valueToType(const kuksa::val::v2::Value& v, T& out);
    
    float getThrottle() const { return throttle.load(); }
    float getSteering() const { return steering.load(); }
    int getGear() const { return gear.load(); }
    float getBattery() const { return battery.load(); }
    float getSpeed() const { return speed.load(); }
    int getDrivingMode() const { return drivingMode.load(); }
};

template<typename T>
bool kuksaLib::sendValueToKuksa(const std::string& path, const T& value) {
  kuksa::val::v2::PublishValueRequest req;

  auto *sid = req.mutable_signal_id();
  sid->set_path(path); 

  auto *dp  = req.mutable_data_point();
  auto *val = dp->mutable_value();

  if constexpr (std::is_same_v<T, float>)        val->set_float_(value);
  else if constexpr (std::is_same_v<T, int32_t>) val->set_int32(value);
  else if constexpr (std::is_same_v<T, bool>)    val->set_bool_(value);
  else if constexpr (std::is_same_v<T, std::string>) val->set_string(value);

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
bool kuksaLib::valueToType(const kuksa::val::v2::Value& v, T& out) {
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
            if constexpr (std::is_same<T, std::string>::value) {
                out = v.string();
                return true;
            } else {
                out = T{};
                return false;
            }
        case V::TYPED_VALUE_NOT_SET:
        default:
            out = T{};
            return false;
    }
}
