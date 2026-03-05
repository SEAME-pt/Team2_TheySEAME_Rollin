#include "CarKuksa.hpp"

CarKuksa::CarKuksa(ICar *car) : CarDecorator(car) {}

CarKuksa::~CarKuksa() {}

void CarKuksa::setThrottle(const int throttle) {
	CarDecorator::setThrottle(throttle);
	_kuksa.sendValueToKuksa("Vehicle.Control.Throttle.Value", (float)abs(throttle));
}

void CarKuksa::setSteering(const int steering) {
	CarDecorator::setSteering(steering);
	_kuksa.sendValueToKuksa("Vehicle.Control.Steering.Angle", (float)steering);
}

void CarKuksa::setGear(const short gear) {
	CarDecorator::setSteering(gear);
	_kuksa.sendValueToKuksa("Vehicle.Control.Gear.Value", (uint8_t)gear);
}
