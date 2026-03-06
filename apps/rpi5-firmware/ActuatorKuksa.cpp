#include "ActuatorKuksa.hpp"

ActuatorKuksa::ActuatorKuksa(IActuator *car) : ActuatorDecorator(car) {}

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
