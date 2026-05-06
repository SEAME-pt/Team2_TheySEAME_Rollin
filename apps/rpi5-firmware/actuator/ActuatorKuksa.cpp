#include "ActuatorKuksa.hpp"

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

void ActuatorKuksa::setTrafficSign(const int trafficSign) {
	ActuatorDecorator::setTrafficSign(trafficSign);
	_kuksa.sendValueToKuksa("Vehicle.Perception.TrafficSign.Value", (int)trafficSign);
}

void ActuatorKuksa::setSpeedLimit(const int speedLimit) {
	ActuatorDecorator::setSpeedLimit(speedLimit);
	_kuksa.sendValueToKuksa("Vehicle.Perception.SpeedLimit.Value", (int)speedLimit);
}