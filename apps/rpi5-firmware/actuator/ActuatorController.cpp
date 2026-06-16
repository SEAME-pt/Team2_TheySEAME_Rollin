#include "ActuatorController.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

ActuatorController::ActuatorController(CarActuator *car, RemoteControl *remote, Lka *lka, kuksaLib &kuksa, Tsr *tsr) : _car(car), _remote(remote), _lka(lka), _tsr(tsr), _kuksa(kuksa) {
}

ActuatorController::~ActuatorController() {}

int ActuatorController::processThrottle(const int rawThrottle) {
	return ((rawThrottle - 127) / 1.9);
}

int ActuatorController::processSteering(const int rawSteering) {
	//int angle = std::clamp(((rawSteering - 127) / 1.27), -30.0, 30.0);
	return (((rawSteering - 127) / 1.27));
}

void ActuatorController::steering(const int angle) {
	const int steering = std::clamp(angle, -30, 30);
	//if (steering == _kuksa.getSteering()) {
	//	return;
	//}
	_car->setSteering(steering);
	std::cout << "Changed Steering " << steering << std::endl;
}

void ActuatorController::throttle(const int throttle) {
	if (_stopDetected)
		return;
	if (throttle < 0) {
		gear(DRIVE);
	} else if (throttle > 0) {
		gear(REVERSE);
	}
	else {
		gear(NEUTRAL);
	}
	
	cruiseControl(false, 0);
	_currentThrottle = throttle;
	_car->setThrottle(throttle);
	std::cout << "Changed Throttle" << std::endl;
}

void ActuatorController::setSpeedLimit(const int speedLimit) {
	_car->setSpeedLimit(speedLimit);
}

void ActuatorController::setTrafficSign(const int trafficSign, const float distance) {
	_car->setTrafficSign(trafficSign, distance);
	// std::cout << "Detected Traffic Sign " << trafficSign << " at distance " << distance << std::endl;
}

void ActuatorController::gear(const short gear) {
	_car->setGear(gear);
	std::cout << "Changing Gear" << std::endl;
}

void ActuatorController::cruiseControl(const bool flag, const int inc) {
	if (flag == false) {
		_car->setCruiseControl(flag, 0);
		return;
	}
	if (_kuksa.getCcActive()) {
		_car->setCruiseControl(flag, _kuksa.getCcTargetSpeed() + inc);
		std::cout << "Target Speed to " << _kuksa.getCcTargetSpeed() << std::endl;
		return;
	}
	_car->setCruiseControl(flag, _kuksa.getSpeed());
	std::cout << "Cruise Control Active to " << _kuksa.getSpeed() << std::endl;
}

void ActuatorController::brake(const bool flag) {
	cruiseControl(false, 0);
	_car->brake(flag);
	std::cout << "Brake " << flag << std::endl;
}

void ActuatorController::update(Subject *subj, Events event) {
	// std::cout << "Received notify " << event << " sub: " << subj << std::endl;
	std::lock_guard<std::mutex> lock(_mutex);
	std::vector<uint16_t> signs;
	bool stopDetected;
	bool brakeFlag;
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
			case Events::CAR_AEB_ENABLED:
				setAEb_Enabled(_remote->getkey(Keys::DpadX));
				break;
			default:
				std::cout << "No event" << std::endl;
				break;
		}
	} else if (_tsr != nullptr && subj == _tsr) {
		switch (event) {
			case Events::CAR_TRAFFIC_SIGN:
				signs = _tsr->getDetectedSigns();
				for (const auto &sign : signs) {
					setTrafficSign(sign, _tsr->estimateDistance(_tsr->getLastDetection()));
				}
				stopDetected = std::find(signs.begin(), signs.end(),
					static_cast<uint16_t>(TrafficSign::STOP)) != signs.end();

				if (stopDetected &&  (_tsr->getStopDistance() < 70.0f && _tsr->getStopDistance() != -1)) {
						std::cout << "STOP sign detected at " << _tsr->getStopDistance() << "m, applying brake!" << std::endl;
						brake(true);
						_stopDetected = true;
						brakeFlag = true;

				} else {
					if (brakeFlag)
						brake(false);
					brakeFlag = false;
						_stopDetected = false;
				}
				break;
			case Events::CAR_SPEED_LIMIT:
				if (_lastSpeedLimit == 80 && _tsr->getSpeedLimit() == 50)
				{
					std::cout << "last speed limit: " << _lastSpeedLimit << " km/h" << std::endl;
					std::cout << "speed limit:" << _tsr->getSpeedLimit() << " km/h, reducing throttle to 75%" << std::endl;
					if (!_reduceSpeed) {
						throttle(_currentThrottle * 0.75f);
						_reduceSpeed = true;
					}
					else
					{
						throttle(_currentThrottle);
						_reduceSpeed = false;
					}
				}
				_lastSpeedLimit = _tsr->getSpeedLimit();
				setSpeedLimit(_tsr->getSpeedLimit());
				break;
			default:
				break;
		}
	} else {
		steering(_lka->getAngle());
		throttle((-20));
	}
}

void ActuatorController::setAEb_Enabled(bool enabled) {
	if (_kuksa.getAebEnabled() != enabled) {
		enabled = true;
	}
	else {
		enabled = false;
	}
	_car->setAEb_Enabled(enabled);
	std::cout << "AEB " << enabled << std::endl;
}

void ActuatorController::test() {
	
}
