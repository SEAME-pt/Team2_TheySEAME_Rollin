#include "CAN.hpp"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>

int send_msg(int sock, canid_t id, uint8_t *data, uint8_t len) {
	struct can_frame frame;
	int nbytes;

	frame.can_id = id;
	frame.len = len;
	memcpy(frame.data, data, len);

	nbytes = write(sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		return (-1);
	}
	return (0);
}

int rec_msg(int sock) {
	struct can_frame frame;

	if (read(sock, &frame, sizeof(struct can_frame)) < 0) {
		return (-1);
	}

	printf("\tFrame_id: %02xh\n", frame.can_id);
	printf("\tData: ");
	for (size_t i = 0; i < frame.len; i++) {
		printf("%02x ", frame.data[i]);
	}
	printf("\nFinish\n");
	return (0);
}

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
