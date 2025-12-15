#include "RemoteControl.hpp"
#include <fcntl.h>
#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>

RemoteControl::RemoteControl() {
	std::cout << "RemoteControl Constructor" << std::endl;
}

RemoteControl::~RemoteControl() {
	std::cout << "RemoteControl Destructor" << std::endl;
}

int RemoteControl::getfd() const { return (_fd); }

void RemoteControl::openRemoteControl() {
	_fd = open("/dev/input/event6", O_RDONLY);
	if (_fd < 0) {
		std::perror("Error in open");
		exit(1);
	}
	std::cout << "Opened joystick" << std::endl;
}

void RemoteControl::closeRemoteControl() {
	if (close(_fd) < 0) {
		std::perror("Error in close");
		exit(1);
	}
	std::cout << "Closed Remote" << std::endl;
}

void RemoteControl::readEvent() {
	struct input_event ev;
	int nbytes;

	nbytes = read(_fd, &ev, sizeof(struct input_event));
	if (nbytes < 0) {
		std::perror("Error in read");
		exit(1);
	}
	switch (ev.type) {
		case EV_ABS:
			if (ev.code == ABS_X) {
				axis_x = ev.value;
			} else if (ev.code == ABS_Y) {
				axis_y = ev.value;
			}
			break;
		case EV_KEY:
			std::cout << "Value " << ev.value << std::endl;
			break;
		case EV_SYN:
			if (ev.code == SYN_REPORT) {
				std::cout << "Syn Report" << std::endl;
			}
			break;
		default:
			printf("Event not handled\n");
	}
	printf("\tX %d Y %d\n", axis_x, axis_y);
}
