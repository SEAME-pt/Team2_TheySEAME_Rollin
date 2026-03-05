#include "CarDecorator.hpp"

CarDecorator::CarDecorator(ICar *car) : _car(car) {}

CarDecorator::~CarDecorator() {
	delete _car;
}

void CarDecorator::setThrottle(const int throttle) {
	_car->setThrottle(throttle);
}

void CarDecorator::setSteering(const int steering) {
	_car->setSteering(steering);
}

void CarDecorator::setGear(const short gear) {
	_car->setGear(gear);
}

void CarDecorator::brake() {
	_car->brake();
}

short CarDecorator::getGear() const {
	return (_car->getGear());
}
