#include "ActuatorController.hpp"
#include "CAN.hpp"
#include "ActuatorCAN.hpp"
#include <iostream>
#include <algorithm>

ActuatorController::ActuatorController(RemoteControl *remote, PurePursuit *pp) : _remote(remote), _pp(pp) {
	CAN *can = new CAN("can0", 500, 0, 0);
	_car = new ActuatorCAN(*can);
	pp->attach(this);
}

ActuatorController::~ActuatorController() {}

int ActuatorController::processThrottle(const int rawThrottle) {
	return ((rawThrottle - 127) / 1.27);
}

int ActuatorController::processSteering(const int rawSteering) {
	return (((rawSteering - 127) / 1.27));
}

void ActuatorController::steering(const int angle) {
	const int steering = std::clamp(angle, -30, 30);

	_car->setSteering(steering);
	std::cout << "Changed Steering " << steering << std::endl;
}

void ActuatorController::throttle(const int throttle) {
	if (throttle < 0) {
		gear(DRIVE);
	} else {
		gear(REVERSE);
	}
	cruiseControl(false, 0);
	_car->setThrottle(throttle);
	std::cout << "Changed Throttle" << std::endl;
}

void ActuatorController::gear(const short gear) {
	_car->setGear(gear);
	std::cout << "Changing Gear" << std::endl;
}

void ActuatorController::cruiseControl(const bool flag, const int inc) {
	//if (flag == false) {
	//	_car->setCruiseControl(flag, 0);
	//	return;
	//}
	//if (_kuksa.getCcActive()) {
	//	_car->setCruiseControl(flag, _kuksa.getCcTargetSpeed() + inc);
	//	std::cout << "Target Speed to " << _kuksa.getCcTargetSpeed() << std::endl;
	//	return;
	//}
	//_car->setCruiseControl(flag, _kuksa.getSpeed());
	//std::cout << "Cruise Control Active to " << _kuksa.getSpeed() << std::endl;
	_car->setCruiseControl(true, 15);
}

void ActuatorController::brake(const bool flag) {
	cruiseControl(false, 0);
	_car->brake(flag);
	std::cout << "Brake " << flag << std::endl;
}

void ActuatorController::update(Subject *subj, Events event) {
	std::cout << "Received notify " << event << std::endl;
	if (subj == _remote) {
		switch (event) {
			case Events::CAR_THROTTLE:
				throttle(processThrottle(_remote->getkey(Keys::JoyY)));
				break;
			case Events::CAR_STEERING:
				steering(processSteering(_remote->getkey(Keys::JoyZ)));
				break;
			case Events::CAR_BRAKE:
				brake(_remote->getkey(Keys::L2));
				break;
			case Events::CAR_GEAR:
				if (_remote->getkey(Keys::X)) {
					gear(NEUTRAL);
				} else if (_remote->getkey(Keys::Y)) {
					gear(DRIVE);
				} else if (_remote->getkey(Keys::A)) {
					gear(REVERSE);
				} else if (_remote->getkey(Keys::B)) {
					gear(PARKING);
				}
				break;
			case Events::CAR_CRUISE_CONTROL:
				if (_remote->getkey(Keys::DpadY) == -1) {
					cruiseControl(true, 1);
				} else if (_remote->getkey(Keys::DpadY) == 1) {
					cruiseControl(true, -1);
				}
				break;

			default:
				std::cout << "No event" << std::endl;
				break;
		}
	} else {
		steering(_pp->getAngle());
		//throttle(-30);
	}
}
