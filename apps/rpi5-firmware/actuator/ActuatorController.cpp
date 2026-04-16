#include "ActuatorController.hpp"
#include <iostream>
#include <algorithm>

ActuatorController::ActuatorController(CarActuator *car, RemoteControl *remote, Lka *lka, kuksaLib &kuksa) : _car(car), _remote(remote), _lka(lka), _kuksa(kuksa) {
}

ActuatorController::~ActuatorController() {}

int ActuatorController::processThrottle(const int rawThrottle) {
	return ((rawThrottle - 127) / 1.27);
}

int ActuatorController::processSteering(const int rawSteering) {
	//int angle = std::clamp(((rawSteering - 127) / 1.27), -30.0, 30.0);
	return (((rawSteering - 127) / 127));
}

void ActuatorController::update(Subject *subj, Events event) {
	std::cout << "Received notify " << event << std::endl;
	if (subj == _remote) {
		switch (event) {
			case Events::CAR_THROTTLE:
				_car->setThrottle(processThrottle(_remote->getkey(Keys::JoyY)));
				break;
			case Events::CAR_STEERING:
				_car->setSteering(processSteering(_remote->getkey(Keys::JoyZ)));
				break;
			case Events::CAR_BRAKE:
				_car->brake(_remote->getkey(Keys::L2));
				std::cout << "Brake " << _remote->getkey(Keys::L2) << std::endl;
				break;
			case Events::CAR_GEAR:
				if (_remote->getkey(Keys::X)) {
					_car->setGear(NEUTRAL);
				} else if (_remote->getkey(Keys::Y)) {
					_car->setGear(DRIVE);
				} else if (_remote->getkey(Keys::A)) {
					_car->setGear(REVERSE);
				} else if (_remote->getkey(Keys::B)) {
					_car->setGear(PARKING);
				}
				break;
			case Events::CAR_CRUISE_CONTROL:
				if (_kuksa.getCcActive()) {
					if (_remote->getkey(Keys::DpadY) == -1) {
						_car->setCruiseControl(true, (int)_kuksa.getCcTargetSpeed() + 1);
					} else if (_remote->getkey(Keys::DpadY) == 1) {
						_car->setCruiseControl(true, (int)_kuksa.getCcTargetSpeed() - 1);
					}
					std::cout << "Target Speed to " << _kuksa.getCcTargetSpeed() << std::endl;
				} else if (_remote->getkey(Keys::DpadY) == -1) {
					_car->setCruiseControl(true, (int)_kuksa.getSpeed());
					std::cout << "Cruise Control Active to " << _kuksa.getSpeed() << std::endl;
				}
				break;

			default:
				std::cout << "No event" << std::endl;
				break;
		}
	} else {
		_car->setSteering(_lka->getAngle());
		std::cout << "Bias" << std::endl;
	}
}
