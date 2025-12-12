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
			uint8_t data[2] = { 0x42, 0x54 };
			can.sendMsg(0x12, data, sizeof(data));
			printf("Sending frame\n");
		}
		if (fds[1].revents & POLLIN) {
			printf("Receiving RemoteControl command\n");
			remote.readEvent();
		}
	}
	return (0);
}
