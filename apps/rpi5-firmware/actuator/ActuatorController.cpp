#include "ActuatorController.hpp"
#include <iostream>

ActuatorController::ActuatorController(CarActuator *car, RemoteControl &remote) : _car(car), _subject(remote) {}

ActuatorController::~ActuatorController() {}

int ActuatorController::processThrottle(const int rawThrottle) {
	return ((rawThrottle - 127) / 1.27);
}

int ActuatorController::processSteering(const int rawSteering) {
	return ((rawSteering - 127) / 127);
}

void ActuatorController::update(Events event) {
	std::cout << "Received notify " << event << std::endl;
	switch (event) {
		case Events::CAR_THROTTLE:
			_car->setThrottle(processThrottle(_subject.getkey(Keys::JoyY)));
			break;
		case Events::CAR_STEERING:
			_car->setSteering(processSteering(_subject.getkey(Keys::JoyZ)));
			break;
		case Events::CAR_BRAKE:
			_car->brake();
			std::cout << "L2 Pressed" << std::endl;
			break;
		case Events::CAR_GEAR:
			if (_subject.getkey(Keys::X)) {
				_car->setGear(NEUTRAL);
			} else if (_subject.getkey(Keys::Y)) {
				_car->setGear(DRIVE);
			} else if (_subject.getkey(Keys::A)) {
				_car->setGear(REVERSE);
			} else if (_subject.getkey(Keys::B)) {
				_car->setGear(PARKING);
			}
			break;

		default:
			std::cout << "No event" << std::endl;
			break;
	}
}
