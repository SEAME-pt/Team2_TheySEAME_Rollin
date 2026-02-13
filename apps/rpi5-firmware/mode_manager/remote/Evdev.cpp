#include "Evdev.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

Evdev::Evdev(const char *device) : _device(device) {
	std::cout << "Evdev Constructor" << std::endl;
	_fd = open(_device, O_RDONLY);
	if (_fd < 0) {
		std::perror("Error in open");
	}
	std::memset(_q, 0, sizeof(_q));
	_qCount = 0;
	_qOut = 0;
	std::cout << "Opened evdev" << std::endl;
}

Evdev::~Evdev() {
	std::cout << "Evdev Destructor" << std::endl;
	if (close(_fd) < 0) {
		std::perror("Error in close");
	}
	std::cout << "Closed evdev" << std::endl;
}

int Evdev::getfd() const { return (_fd); }

const char *Evdev::getDevice() const { return (_device); }

void Evdev::readEvent() {
	struct input_event ev;
	int nbytes;

	nbytes = read(_fd, &ev, sizeof(struct input_event));
	if (nbytes < 0) {
		std::perror("Error in read");
	}
	_q[_qCount] = ev;
	_qCount++;
}

int Evdev::pendingEvent() const { return (_qCount); }

struct input_event &Evdev::nextEvent() {
	struct input_event &ev = _q[_qOut++];

	_qOut = (_qOut + 1) % _qCount;
	_qCount--;
	return (ev);
}

