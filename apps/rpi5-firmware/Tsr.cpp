#include "Tsr.hpp"

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
    switch (static_cast<TrafficSign>(tsrData.trafficSign)) {
        case TrafficSign::STOP:
            _car->setTrafficSign(static_cast<int>(TrafficSign::STOP));
            break;
        case TrafficSign::SPEED_LIMIT_30:
            _car->setSpeedLimit(static_cast<int>(TrafficSign::SPEED_LIMIT_30));
            break;
        case TrafficSign::SPEED_LIMIT_50:
            _car->setSpeedLimit(static_cast<int>(TrafficSign::SPEED_LIMIT_50));
            break;
        case TrafficSign::SPEED_LIMIT_100:
            _car->setSpeedLimit(static_cast<int>(TrafficSign::SPEED_LIMIT_100));
            break;
        case TrafficSign::SPEED_LIMIT_80:
            _car->setSpeedLimit(static_cast<int>(TrafficSign::SPEED_LIMIT_80));
            break;
        case TrafficSign::SPEED_LIMIT_120:
            _car->setSpeedLimit(static_cast<int>(TrafficSign::SPEED_LIMIT_120));
            break;
        case TrafficSign::YIELD:
            _car->setTrafficSign(static_cast<int>(TrafficSign::YIELD));
            break;
        case TrafficSign::NO_ENTRY:
            _car->setTrafficSign(static_cast<int>(TrafficSign::NO_ENTRY));
            break;
        case TrafficSign::TURN_LEFT:
            _car->setTrafficSign(static_cast<int>(TrafficSign::TURN_LEFT));
            break;
        case TrafficSign::TURN_RIGHT:
            _car->setTrafficSign(static_cast<int>(TrafficSign::TURN_RIGHT));
            break;
        case TrafficSign::PEDESTRIAN:
            _car->setTrafficSign(static_cast<int>(TrafficSign::PEDESTRIAN));
            break;
        case TrafficSign::TRAFFIC_LIGHT:
            _car->setTrafficSign(static_cast<int>(TrafficSign::TRAFFIC_LIGHT));
            break;
        case TrafficSign::ONE_WAY:
            _car->setTrafficSign(static_cast<int>(TrafficSign::ONE_WAY));
            break;
        case TrafficSign::NO_PARKING:
            _car->setTrafficSign(static_cast<int>(TrafficSign::NO_PARKING));
            break;
        case TrafficSign::NO_OVERTAKING:
            _car->setTrafficSign(static_cast<int>(TrafficSign::NO_OVERTAKING));
            break;
        default:
            break;
    }
}