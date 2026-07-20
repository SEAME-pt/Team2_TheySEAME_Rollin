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

void ActuatorDecorator::setAccLeadVehicleDistance(const float distanceMeters) {
	_car->setAccLeadVehicleDistance(distanceMeters);
}

void ActuatorDecorator::setAccLeadVehicleOrientation(const float yawDegrees) {
	_car->setAccLeadVehicleOrientation(yawDegrees);
}

void ActuatorDecorator::setBsdLeftOccupied(const bool occupied) {
	_car->setBsdLeftOccupied(occupied);
}

void ActuatorDecorator::setBsdRightOccupied(const bool occupied) {
	_car->setBsdRightOccupied(occupied);
}

void ActuatorDecorator::setBsdLeftDistance(const float distanceMeters) {
	_car->setBsdLeftDistance(distanceMeters);
}

void ActuatorDecorator::setBsdRightDistance(const float distanceMeters) {
	_car->setBsdRightDistance(distanceMeters);
}

void ActuatorDecorator::setBsdLeftVehicleOrientation(const float yawDegrees) {
	_car->setBsdLeftVehicleOrientation(yawDegrees);
}

void ActuatorDecorator::setBsdRightVehicleOrientation(const float yawDegrees) {
	_car->setBsdRightVehicleOrientation(yawDegrees);
}

void ActuatorDecorator::setAEb_Enabled(bool autonomous) {
	_car->setAEb_Enabled(autonomous);
}
