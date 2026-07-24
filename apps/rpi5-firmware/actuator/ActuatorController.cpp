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

	_currentThrottle = throttle;
	_car->setThrottle(throttle);
	std::cout << "Changed Throttle" << std::endl;
}

void ActuatorController::gear(const short gear) {
	_car->setGear(gear);
	std::cout << "Changing Gear" << std::endl;
}

void ActuatorController::cruiseControl(const bool flag, const int inc) {
	if (flag == false) {
		_car->setCruiseControl(flag, 0);
		_CCActive = false;
		return;
	}
	if (_CCActive == false) {
		_car->setCruiseControl(flag, _lastCCSpeed + inc);
		std::cout << "Target Speed to " << _lastCCSpeed + inc << std::endl;
		_CCActive = flag;
		return;
	}
	_car->setCruiseControl(flag, _lastCCSpeed);
	_CCActive = flag;
	std::cout << "Cruise Control Active to " << _lastCCSpeed << std::endl;
}

void ActuatorController::brake(const bool flag) {
	_car->brake(flag);
	std::cout << "Brake " << flag << std::endl;
}

void ActuatorController::trafficSign() {
	auto signs = _tsr->getDetectedSigns();
	
	if (&_kuksa) {
		for (const auto &sign : signs) {
			if (sign <= 15)
				_kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSignType", static_cast<uint8_t>(sign));
		}
	}

    bool stopDetected = std::find(signs.begin(), signs.end(),
        static_cast<uint16_t>(TrafficSign::STOP)) != signs.end();

    float stopDist = _tsr->getStopDistance();

    if (stopDetected && stopDist != -1 && stopDist < 70.0f 
        && !_stopCooldown && !_stopDetected) {
		if (_tsr->getMainTsr() == false) {
        	brake(true);
        	throttle(0);
		}
        _stopDetected = true;
        _stopBrakeFrames = 0;
    }

    if (_stopDetected) {
        _stopBrakeFrames++;

        if (_stopBrakeFrames >= STOP_BRAKE_FRAMES) {
			if (_tsr->getMainTsr() == false) {
				brake(false);
			}
            _stopDetected = false;

            _stopCooldown = true;
            _stopCooldownFrames = 0;
        }
    }

    if (_stopCooldown) {
        _stopCooldownFrames++;

        if (_stopCooldownFrames >= STOP_COOLDOWN_FRAMES) {
            _stopCooldown = false;
        }
    }

}

void ActuatorController::speedLimit() {
    int currentLimit = _tsr->getSpeedLimit();
	if (_tsr->getMainTsr() == false) {
		if (_lastSpeedLimit == 80 && currentLimit == 50) {
			if (!_reduceSpeed) {
				throttle(_currentThrottle * 0.75f);
				_reduceSpeed = true;
			} else {
				throttle(_currentThrottle);
				_reduceSpeed = false;
			}
		}
	}
    _lastSpeedLimit = currentLimit;
    _kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSpeedLimit", static_cast<float>(currentLimit));
}

void ActuatorController::update(Subject *subj, Events event) {
	std::lock_guard<std::mutex> lock(_mutex);
	std::vector<uint16_t> signs;
	bool stopDetected = false;
	float stopDist = -1;
	if (_remote != nullptr && subj == _remote) {
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
				if (_remote->getkey(Keys::DpadX) == -1)
					setAEb_Enabled(_remote->getkey(Keys::DpadX));
				break;
			default:
				std::cout << "No event" << std::endl;
				break;
		}
	} else if (_tsr != nullptr && subj == _tsr) {
		switch (event) {
			case Events::CAR_TRAFFIC_SIGN:
				trafficSign();
				break;
			case Events::CAR_SPEED_LIMIT:
				speedLimit();
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
	(void)enabled;
	if (_aebEnabled == false) {
		_aebEnabled = true;
	}
	else {
		_aebEnabled = false;
	}
	_car->setAEb_Enabled(_aebEnabled);
	std::cout << "AEB " << _aebEnabled << std::endl;
}

void ActuatorController::test() {
	
}
