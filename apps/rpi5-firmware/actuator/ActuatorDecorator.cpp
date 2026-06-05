#include "ActuatorDecorator.hpp"
#include <iostream>
#include <cmath>

ActuatorDecorator::ActuatorDecorator(CarActuator *car) : _car(car) {}

ActuatorDecorator::~ActuatorDecorator() {
	delete _car;
}

void ActuatorDecorator::setThrottle(const int throttle) {
	_currentThrottle = throttle;
	
	if (_activeSpeedLimit == 50 && throttle > 0) {
		int reducedThrottle = static_cast<int>(std::round(throttle * SPEED_REDUCTION_FACTOR));
		std::cout << "[TSR] Speed limit 50: reducing throttle " << throttle << "% -> " 
		          << reducedThrottle << "%" << std::endl;
		_car->setThrottle(reducedThrottle);
	} else {
		_car->setThrottle(throttle);
	}
}

void ActuatorDecorator::setSteering(const int steering) {
	_car->setSteering(steering);
}

void ActuatorDecorator::setGear(const short gear) {
	_car->setGear(gear);
}

void ActuatorDecorator::brake(const bool flag) {
	_car->brake(flag);
}

void ActuatorDecorator::setCruiseControl(const bool flag, const int targetSpeed) {
	_car->setCruiseControl(flag, targetSpeed);
}

void ActuatorDecorator::setTrafficSign(const int trafficSign, const float distance) {
	_car->setTrafficSign(trafficSign, distance);
	
	// Automatic response: STOP sign within 10 meters
	if (trafficSign == STOP_SIGN && distance >= 0 && distance < STOP_BRAKE_DISTANCE_M) {
		std::cout << "[TSR] STOP sign at " << distance << "m (< " << STOP_BRAKE_DISTANCE_M 
		          << "m) → BRAKE!" << std::endl;
		_car->brake(true);
	}
}

void ActuatorDecorator::setSpeedLimit(const int speedLimit) {
	_activeSpeedLimit = speedLimit;
	_car->setSpeedLimit(speedLimit);
}
void ActuatorDecorator::setAEb_Enabled(bool autonomous) {
	_car->setAEb_Enabled(autonomous);
}

