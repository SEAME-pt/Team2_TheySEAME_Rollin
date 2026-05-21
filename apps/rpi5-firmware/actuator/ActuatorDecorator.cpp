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

void ActuatorDecorator::brake(const bool flag) {
	_car->brake(flag);
}

void ActuatorDecorator::setCruiseControl(const bool flag, const int targetSpeed) {
	_car->setCruiseControl(flag, targetSpeed);
}

void ActuatorDecorator::setTrafficSign(const int trafficSign, const float distance) {
	_car->setTrafficSign(trafficSign, distance);
}

void ActuatorDecorator::setSpeedLimit(const int speedLimit) {
	_car->setSpeedLimit(speedLimit);
}
