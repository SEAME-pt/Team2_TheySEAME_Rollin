#include "CarKuksa.hpp"
#include <iostream>

CarKuksa::CarKuksa(RemoteControl &remote) : _subject(remote) {
	std::cout << "CarKuksa Constructor" << std::endl;
}

CarKuksa::~CarKuksa() {
	std::cout << "CarKuska destructor" << std::endl;
}

int CarKuksa::processThrottle(const int rawThrottle) {
	return ((rawThrottle - 127) / 1.27);
}

int CarKuksa::processSteering(const int rawSteering) {
	return ((rawSteering - 127) / 127);
}

void CarKuksa::setThrottle(const int throttle) {
	_kuksa.sendValueToKuksa("Vehicle.Control.Throttle.Value", (float)abs(throttle));
}

void CarKuksa::setSteering(const int steering) {
	_kuksa.sendValueToKuksa("Vehicle.Control.Steering.Angle", (float)steering);
}

void CarKuksa::setGear(const short gear) {
	_gear = gear;
	_kuksa.sendValueToKuksa("Vehicle.Control.Gear.Value", (uint8_t)gear);
}

void CarKuksa::brake() {}

short CarKuksa::getGear() const {
	return (_gear);
}

void CarKuksa::update(Events event) {
	std::cout << "Received notify" << std::endl;
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
