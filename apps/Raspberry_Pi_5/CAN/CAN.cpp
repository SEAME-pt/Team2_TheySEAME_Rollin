#include "CAN.hpp"
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>

CAN::CAN(const char *interface) {
	struct sockaddr_can addr;
	struct ifreq ifr;

	_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (_sock < 0) {
		std::perror("Error creating socket");
	}
	std::cout << "Created Socket" << std::endl;

	std::strcpy(ifr.ifr_name, interface);
	if (ioctl(_sock, SIOCGIFINDEX, &ifr) < 0) {
		std::perror("Error in ioctl");
	}

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		std::perror("Error in bind");
	}
	std::cout << "Binded Socket" << std::endl;
}

CAN::~CAN() {
	if (close(_sock) < 0) {
		std::perror("Error in close");
	}
	std::cout << "Closed CAN socket" << std::endl;
}

int CAN::getSocketFd() const { return (_sock); }

/*
 * @brief Sends a CAN frame to the Bus
 *
 * This function fills the can_frame struct with the id, length and data and
 * sends the frame to the Bus
 *
 * ====================== Requirement Traceability ===========================
 * [impl->dsn~comms-can-rpi-sendMsg~1]
 * ==========================================================================
 *
 * @return void
 *
 */
void CAN::sendComm(const canid_t id, const uint8_t *data, const uint8_t len) {
	struct can_frame frame;
	int nbytes;

	frame.can_id = id;
	frame.len = len;
	std::memcpy(frame.data, data, frame.len);

	nbytes = write(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in write");
		exit(1);
	}
}

/*
 * @brief Read a CAN frame from the Bus
 *
 * This function reads a CAN frame from the Bus and prints the information
 * inside the can_frame struct
 *
 * ====================== Requirement Traceability ===========================
 * [impl->dsn~comms-can-rpi-receiveMsg~1]
 * ==========================================================================
 *
 * @return void
 *
 */
void CAN::readComm() {
	struct can_frame frame;
	int nbytes;

	nbytes = read(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in read");
		exit(1);
	}
	printCANFrame(frame);
}

void CAN::printCANFrame(const struct can_frame frame) {
	std::cout << "\tFrameId: " << frame.can_id << std::endl;
	std::cout << "\tData: ";
	for (size_t i = 0; i < frame.len; i++) {
		printf("%02x ", frame.data[i]);
	}
	std::cout << "\nFinish" << std::endl;
}
