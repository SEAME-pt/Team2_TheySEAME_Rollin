#include "Tsr.hpp"

namespace {

enum class ModelTrafficSignClass : uint16_t {
    SPEED_LIMIT_50 = 0,
    SPEED_LIMIT_80 = 1,
    GATE = 2,
    CROSSWALK = 3,
    STOP = 4,
    YIELD = 5,
    CAR = 6,
    DANGER = 7,
    OBSTACLE = 8,
    TRAFFIC_LIGHT_GREEN = 9,
    TRAFFIC_LIGHT_OFF = 10,
    TRAFFIC_LIGHT_RED = 11,
    TRAFFIC_LIGHT_YELLOW = 12,
};

bool mapModelClassToSpeedLimit(uint16_t classId, int &speedLimit)
{
    
    switch (classId) {
        case static_cast<uint16_t>(ModelTrafficSignClass::SPEED_LIMIT_50):
            speedLimit = 50;
            return true;
        case static_cast<uint16_t>(ModelTrafficSignClass::SPEED_LIMIT_80):
            speedLimit = 80;
            return true;
        default:
            return false;
    }
}

TrafficSign mapModelClassToTrafficSign(uint16_t classId)
{
    switch (classId) {
        case static_cast<uint16_t>(ModelTrafficSignClass::STOP):
            return TrafficSign::STOP;
        case static_cast<uint16_t>(ModelTrafficSignClass::CROSSWALK):
            return TrafficSign::PEDESTRIAN;
        case static_cast<uint16_t>(ModelTrafficSignClass::YIELD):
            return TrafficSign::YIELD;
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_GREEN):
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_RED):
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_YELLOW):
            return TrafficSign::TRAFFIC_LIGHT;
        default:
            return TrafficSign::UNKNOWN;
    }
}

}

Tsr::Tsr(CarActuator *car) : _car(car)
{
}

Tsr::~Tsr()
{
}

const TsrHeader& Tsr::getLastDetection() {
    return _lastDetection;
}

void Tsr::handleTrafficSign(const TsrHeader &tsrData)
{
    _lastDetection = tsrData;
    int speedLimit = 0;
    if (mapModelClassToSpeedLimit(tsrData.trafficSign, speedLimit)) {
        std::cout << "Detected Speed Limit: " << speedLimit << " km/h, Accuracy: " << tsrData.accuracy << std::endl;
        _car->setSpeedLimit(speedLimit);
        return;
    }
    std::cout << "Detected Traffic Sign Class ID: " << tsrData.trafficSign << ", Accuracy: " << tsrData.accuracy << std::endl;
    _car->setTrafficSign(static_cast<int>(mapModelClassToTrafficSign(tsrData.trafficSign)));
}