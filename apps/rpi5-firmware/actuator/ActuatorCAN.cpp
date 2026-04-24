#include "ActuatorCAN.hpp"
#include <iostream>

ActuatorCAN::ActuatorCAN(ICAN &can) : _can(can) {
	std::cout << "CarCAN constructor" << std::endl;
	_gear = DRIVE;
}

ActuatorCAN::~ActuatorCAN() {
	std::cout << "CarCAN destructor" << std::endl;
}

void ActuatorCAN::setGear(const short gear) {
	uint8_t data[1];

	_gear = gear;
	data[0] = _gear;
	_can.sendFrame(GEAR, data, sizeof(data));
}

void ActuatorCAN::setThrottle(const int throttle) {
	uint8_t data[1];

	data[0] = abs(throttle);
	_can.sendFrame(THROTTLE, data, sizeof(data));
}

void ActuatorCAN::setSteering(const int steering) {
	uint8_t data[1];

	data[0] = steering;
	_can.sendFrame(STEERING, data, sizeof(data));
}

void ActuatorCAN::brake(const bool flag) {
	uint8_t data[1];

	data[0] = flag;
	_can.sendFrame(BRAKE, data, sizeof(data));
}

short ActuatorCAN::getGear() const { return (_gear); }

void ActuatorCAN::setCruiseControl(const bool flag, const int targetSpeed) {
	uint8_t data[2];

	data[0] = flag;
	data[1] = targetSpeed;
	_can.sendFrame(CRUISE_CONTROL, data, sizeof(data));
}
