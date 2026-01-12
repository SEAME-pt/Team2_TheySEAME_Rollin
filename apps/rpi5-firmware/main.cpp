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

int main() {
	struct pollfd fds[2];
	CAN can("can0");
	Evdev evdev("/dev/input/event6");
	RemoteControl remote(evdev);

	std::signal(SIGINT, signal_handler);

	can.setBitrate(500);
	can.openSocket();
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
			printf("Receiving frame\n");
			struct can_frame frame;
			can.readFrame(frame);
		}
		if (fds[1].revents & POLLIN) {
			evdev.readEvent();
		}
		while (evdev.pendingEvent() > 0) {
			struct input_event &ev = evdev.nextEvent();
			remote.setkey(ev.code, ev.value);
			uint8_t data[3];
			data[0] = 0x00;
			data[1] = abs((remote.getkey(ABS_Y) - 127) / 1.27);
			data[2] = (remote.getkey(ABS_X) - 127) / 127;
			printf("Throttle %d\n", data[1]);
			printf("Steering %d\n\n", data[2]);
			can.sendFrame(0x100, data, sizeof(data));
		}
	}
	return (0);
}
