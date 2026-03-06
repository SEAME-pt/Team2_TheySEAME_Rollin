#include "ActuatorCAN.hpp"
#include <iostream>

ActuatorCAN::ActuatorCAN(ICAN &can, RemoteControl &remote) : _can(can), _subject(remote) {
	std::cout << "CarCAN constructor" << std::endl;
	_gear = DRIVE;
}

ActuatorCAN::~ActuatorCAN() {
	std::cout << "CarCAN destructor" << std::endl;
}

int ActuatorCAN::processThrottle(const int rawThrottle) {
	return ((rawThrottle - 127) / 1.27);
}

int ActuatorCAN::processSteering(const int rawSteering) {
	return ((rawSteering - 127) / 127);
}

void ActuatorCAN::setGear(const short gear) {
	uint8_t data[1];

	_gear = gear;
	data[0] = _gear;
	_can.sendFrame(GEAR, data, sizeof(data));
	std::cout << "Changing Gear" << std::endl;
}

void ActuatorCAN::setThrottle(const int throttle) {
	uint8_t data[1];

	if (_gear == PARKING) { return; }
	if (throttle < 0) {
		setGear(DRIVE);
	} else {
		setGear(REVERSE);
	}
	data[0] = abs(throttle);
	_can.sendFrame(THROTTLE, data, sizeof(data));
	std::cout << "Changed Throttle" << std::endl;
}

void ActuatorCAN::setSteering(const int steering) {
	uint8_t data[1];

	data[0] = steering;
	_can.sendFrame(STEERING, data, sizeof(data));
	std::cout << "Changed Steering" << std::endl;
}

void ActuatorCAN::brake() {
	uint8_t data[1];

	data[0] = 0x01;
	_can.sendFrame(BRAKE, data, sizeof(data));
}

short ActuatorCAN::getGear() const { return (_gear); }
