#include "Car.hpp"
#include <iostream>
#include <stdio.h>

Car::Car(ICAN &can, RemoteControl &remote) : _can(can), _subject(remote) {
	std::cout << "Car constructor" << std::endl;
	_throttle = 0;
	_steering = 0;
	_gear = 0;
}

Car::~Car() {
	std::cout << "Car destructor" << std::endl;
}

void Car::startNstop(const bool signal) {
	uint8_t data[1];

	data[0] = signal;
	_can.sendFrame(0x00, data, sizeof(data));
}

int Car::control() {
	uint8_t data[3];

	data[0] = 0x00;
	data[1] = _throttle | (_gear << 7);
	data[2] = _steering;
	printf("Throttle %d\n", data[1]);
	printf("Steering %d\n", data[2]);
	_can.sendFrame(0x100, data, sizeof(data));
	return (0);
}

int Car::getThrottle() const { return (_throttle); }

int Car::getSteering() const { return (_steering); }

short Car::getGear() const { return (_gear); }

void Car::setThrottle(const int throttle) {
	if (throttle > 127) {
		_gear = 1;
	} else {
		_gear = 0;
	}
	_throttle = abs((int)((throttle - 127) / 1.27));
}

void Car::setSteering(const int steering) {
	_steering = (steering - 127) / 127;
}

void Car::setGear(const short gear) {
	_gear = gear;
}

void Car::update(Events event) {
	std::cout << "Receiving notifies " << event << std::endl;
	switch (event) {
		case Events::CAR_THROTTLE:
			setThrottle(_subject.getkey(Keys::JoyY));
			control();
			break;
		case Events::CAR_STEERING:
			setSteering(_subject.getkey(Keys::JoyZ));
			control();
			break;
		//case Events::CAR_START:
		//	startNstop(_subject.getkey(Keys::Start));
		//	break;
		case Events::CAR_BRAKE:
			std::cout << "L2 Pressed" << std::endl;
			break;

		default:
			std::cout << "No event" << std::endl;
			break;
	}
}
