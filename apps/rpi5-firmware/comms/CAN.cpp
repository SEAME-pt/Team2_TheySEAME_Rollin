#include "CAN.hpp"
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>

CAN::CAN(const std::string &interface) {
	struct sockaddr_can addr;

	_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (_sock < 0) {
		std::perror("Error creating socket");
	}
	std::cout << "Created Socket" << std::endl;

	std::strcpy(_ifr.ifr_name, interface.c_str());
	if (ioctl(_sock, SIOCGIFINDEX, &_ifr) < 0) {
		std::perror("Error in ioctl");
	}
	addr.can_family = AF_CAN;
	addr.can_ifindex = _ifr.ifr_ifindex;
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

std::string CAN::getInterface() const { return (_ifr.ifr_name); }

int CAN::getBitrate() const { return (10); };

bool CAN::isUp() const { return (_ifr.ifr_flags & IFF_UP); }

bool CAN::isRunning() const { return (_ifr.ifr_flags & IFF_RUNNING); }

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
int CAN::sendFrame(const canid_t id, const uint8_t *data, const uint8_t len) {
	struct can_frame frame;
	int nbytes;

	frame.can_id = id;
	frame.len = len;
	std::memcpy(frame.data, data, frame.len);

	nbytes = write(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in write");
		return (-1);
	}
	return (0);
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
int CAN::readFrame(struct can_frame &frame) {
	int nbytes;

	nbytes = read(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in read");
		return (-1);
	}
	return (0);
}

