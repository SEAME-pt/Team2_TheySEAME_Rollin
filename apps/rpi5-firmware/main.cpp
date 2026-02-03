#include "CAN.hpp"
#include "Evdev.hpp"
#include "Car.hpp"
#include "RemoteControl.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>

bool run = true;

void signal_handler(int signal) {
	run = false;
}

void carControl(RemoteControl &remote, ICar &car) {
	uint8_t data[3];

	car.setThrottle(remote.getkey(Keys::JoyY));
	car.setSteering(remote.getkey(Keys::JoyZ));
	car.control();
}

int main() {
	struct pollfd fds[2];
	CAN can("can0", 500, 0, 0);
	Evdev evdev("/dev/input/event6");
	Car car(can);
	RemoteControl remote(evdev);

	std::signal(SIGINT, signal_handler);

	fds[0].fd = can.getSocketFd();
	fds[0].events = POLLIN;
	fds[1].fd = evdev.getfd();
	fds[1].events = POLLIN;
	while (run) {
		if (poll(fds, 2, 0) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			//printf("Receiving frame\n");
			struct can_frame frame;
			can.readFrame(frame);
		}
		if (fds[1].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
			carControl(remote, car);
		}
	}
	return (0);
}
