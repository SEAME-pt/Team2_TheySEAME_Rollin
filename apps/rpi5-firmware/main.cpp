#include "CAN.hpp"
#include "Evdev.hpp"
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

void carControl(RemoteControl &remote, ICAN &can) {
	uint8_t data[3];
	short steering = remote.getkey(Keys::JoyZ);
	short throttle = remote.getkey(Keys::JoyY);
	short gear = 0;

	if (throttle > 127) {
		gear = 1;
	}
	data[0] = 0x00;
	data[1] = ((int)abs((throttle - 127) / 1.27) | (gear << 7));
	data[2] = (steering - 127 / 127);
	printf("Throttle %d\n", data[1]);
	printf("Steering %d\n", data[2]);
	can.sendFrame(0x100, data, sizeof(data));
}

int main() {
	struct pollfd fds[2];
	CAN can("can0", 500, 0, 0);
	Evdev evdev("/dev/input/event6");
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
			carControl(remote, can);
		}
	}
	return (0);
}
