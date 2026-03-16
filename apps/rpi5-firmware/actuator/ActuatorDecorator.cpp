#include "ActuatorDecorator.hpp"

ActuatorDecorator::ActuatorDecorator(CarActuator *car) : _car(car) {}

ActuatorDecorator::~ActuatorDecorator() {
	delete _car;
}

void ActuatorDecorator::setThrottle(const int throttle) {
	_car->setThrottle(throttle);
}

void ActuatorDecorator::setSteering(const int steering) {
	_car->setSteering(steering);
}

void ActuatorDecorator::setGear(const short gear) {
	_car->setGear(gear);
}

void ActuatorDecorator::brake() {
	_car->brake();
}

short ActuatorDecorator::getGear() const {
	return (_car->getGear());
}

void ActuatorDecorator::setCruiseControl(const bool flag, const int targetSpeed) {
	_car->setCruiseControl(flag, targetSpeed);
}
