#include "CarCAN.hpp"
#include <iostream>

CarCAN::CarCAN(ICAN &can, RemoteControl &remote) : _can(can), _subject(remote) {
	std::cout << "CarCAN constructor" << std::endl;
	_gear = DRIVE;
}

CarCAN::~CarCAN() {
	std::cout << "CarCAN destructor" << std::endl;
}

int CarCAN::processThrottle(const int rawThrottle) {
	return ((rawThrottle - 127) / 1.27);
}

int CarCAN::processSteering(const int rawSteering) {
	return ((rawSteering - 127) / 127);
}

void CarCAN::startNstop(const bool signal) {
	uint8_t data[1];

	data[0] = signal;
	_can.sendFrame(0x00, data, sizeof(data));
}

void CarCAN::setGear(const short gear) {
	uint8_t data[1];

	_gear = gear;
	data[0] = _gear;
	_can.sendFrame(GEAR, data, sizeof(data));
	std::cout << "Changing Gear" << std::endl;
}

void CarCAN::setThrottle(const int throttle) {
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

void CarCAN::setSteering(const int steering) {
	uint8_t data[1];

	data[0] = steering;
	_can.sendFrame(STEERING, data, sizeof(data));
	std::cout << "Changed Steering" << std::endl;
}

void CarCAN::brake() {
	uint8_t data[1];

	data[0] = 0x01;
	_can.sendFrame(BRAKE, data, sizeof(data));
}

short CarCAN::getGear() const { return (_gear); }

void CarCAN::update(Events event) {
	std::cout << "Receiving notifies " << event << std::endl;
	switch (event) {
		case Events::CAR_THROTTLE:
			setThrottle(processThrottle(_subject.getkey(Keys::JoyY)));
			break;
		case Events::CAR_STEERING:
			setSteering(processSteering(_subject.getkey(Keys::JoyZ)));
			break;
		case Events::CAR_BRAKE:
			brake();
			std::cout << "L2 Pressed" << std::endl;
			break;
		case Events::CAR_GEAR:
			if (_subject.getkey(Keys::X)) {
				setGear(NEUTRAL);
			} else if (_subject.getkey(Keys::Y)) {
				setGear(DRIVE);
			} else if (_subject.getkey(Keys::A)) {
				setGear(REVERSE);
			} else if (_subject.getkey(Keys::B)) {
				setGear(PARKING);
			}
			break;

		default:
			std::cout << "No event" << std::endl;
			break;
	}
}
