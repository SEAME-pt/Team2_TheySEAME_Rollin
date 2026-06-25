#include "ActuatorKuksa.hpp"
#include <iostream>

ActuatorKuksa::ActuatorKuksa(CarActuator *car, kuksaLib &kuksa) : ActuatorDecorator(car), _kuksa(kuksa) {}

ActuatorKuksa::~ActuatorKuksa() {}

void ActuatorKuksa::setThrottle(const int throttle) {
	ActuatorDecorator::setThrottle(throttle);
	_kuksa.sendValueToKuksa("Vehicle.Control.Throttle.Value", (float)abs(throttle));
}

void ActuatorKuksa::setSteering(const int steering) {
	ActuatorDecorator::setSteering(steering);
	_kuksa.sendValueToKuksa("Vehicle.Control.Steering.Angle", (float)steering);
}

void ActuatorKuksa::setGear(const short gear) {
	ActuatorDecorator::setGear(gear);
	_kuksa.sendValueToKuksa("Vehicle.Control.Gear.Value", (uint8_t)gear);
}

void ActuatorKuksa::setCruiseControl(const bool flag, const int targetSpeed) {
	ActuatorDecorator::setCruiseControl(flag, targetSpeed);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.CruiseControl.Enabled", (bool)flag);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.CruiseControl.TargetSpeed", (float)targetSpeed);
}

void ActuatorKuksa::setTrafficSign(const int trafficSign, const float distance) {
	ActuatorDecorator::setTrafficSign(trafficSign, distance);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSignType", static_cast<uint8_t>(trafficSign));
	_kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSignDistance", distance);

	static constexpr int STOP_SIGN = 1;
	static constexpr float STOP_BRAKE_DISTANCE_M = 100.0f;
	if (trafficSign == STOP_SIGN && distance >= 0 && distance < STOP_BRAKE_DISTANCE_M) {
		std::cout << "[TSR] (Kuksa) STOP sign at " << distance << "m (< " << STOP_BRAKE_DISTANCE_M
				  << "m) → BRAKE!" << std::endl;
		ActuatorDecorator::brake(true);
	}
}

void ActuatorKuksa::setSpeedLimit(const int speedLimit) {
	ActuatorDecorator::setSpeedLimit(speedLimit);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSpeedLimit", static_cast<float>(speedLimit));
}
void ActuatorKuksa::setAEb_Enabled(const bool autonomous) {
	ActuatorDecorator::setAEb_Enabled(autonomous);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.AutomaticEmergencyBraking.Enabled", (bool)autonomous);
}
