#include "CAN.hpp"
#include "RemoteControl.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>

CAN can;
RemoteControl remote;

void signal_handler(int signal) {
	can.closeSocket();
	remote.closeRemoteControl();
}

int main() {
	struct pollfd fds[2];

	std::signal(SIGINT, signal_handler);

	remote.openRemoteControl();
	can.openSocket("can0");
	fds[0].fd = can.getSocketFd();
	fds[0].events = POLLIN;
	fds[1].fd = remote.getfd();
	fds[1].events = POLLIN;
	while (1) {
		if (poll(fds, 2, -1) < 0) {
			perror("Error in poll:");
			exit(1);
		}
		if (fds[0].revents & POLLIN) {
			printf("Receiving frame\n");
			can.readMsg();
		}
		if (fds[1].revents & POLLIN) {
			printf("Receiving RemoteControl command\n");
			remote.readEvent();
			uint8_t throttle = abs((remote.axis_y - 127) / 1.27);
			uint8_t steering = (remote.axis_x - 127) / 127;
			printf("\tSteering %d\n", steering);
			printf("\tThrottle %d\n", throttle);
			uint8_t data[3] = { 0x00, throttle, steering };
			printf("Sending frame\n");
			can.sendMsg(0x100, data, sizeof(data));
		}
	}
	return (0);
}
