#include <sys/socket.h>
#include <linux/can.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
	int sock;
	struct sockaddr_can sockaddr;
	struct ifreq ifr;

	sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (sock < 0) {
		printf("Error creating socket\n");
		exit(1);
	}
	printf("Created socket\n");

	strcpy(ifr.ifr_name, "can0");
	ioctl(sock, SIOCGIFINDEX, &ifr);
	printf("Received CAN interface\n");

	sockaddr.can_family = AF_CAN;
	sockaddr.can_ifindex = ifr.ifr_ifindex;
	if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_can)) < 0) {
		printf("Error in bind\n");
		exit(1);
	}
	printf("Binded socket\n");

	printf("Sending frame\n");
	uint8_t data[2] = { 0x42, 0x54 };
	if (send_msg(sock, 0x124, data, sizeof(data)) < 0) {
		printf("Error sending can_frame\n");
		exit(1);
	}

	printf("Receiving frame\n");
	if (rec_msg(sock) < 0) {
		printf("Error in read\n");
		exit(1);
	}
	return (0);
}
