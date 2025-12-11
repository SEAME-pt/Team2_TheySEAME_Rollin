#include "CAN.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>

int main() {
	CAN can;
	struct pollfd fds;

	can.openSocket("can0");
	fds.fd = can.getSocketFd();
	fds.events = POLLIN;
	while (1) {
		if (poll(&fds, 1, 5000) < 0) {
			perror("Error in poll:");
			exit(1);
		}
		if (fds.revents & POLLIN) {
			printf("Receiving frame\n");
			can.readMsg();
		}
		printf("Sending frame\n");
		uint8_t data[2] = { 0x42, 0x54 };
		can.sendMsg(0x12, data, sizeof(data));
		break;
	}

	can.closeSocket();
	return (0);
}
