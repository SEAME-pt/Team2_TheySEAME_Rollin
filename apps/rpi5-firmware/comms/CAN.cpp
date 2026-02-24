#include "CAN.hpp"
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>

CAN::CAN(const std::string &interface) : _interface(interface) {
	_up = false;
	can_do_stop(_interface.c_str());
}

CAN::CAN(const std::string &interface, const unsigned int bitrate, 
	unsigned int modeToControl, unsigned int modeToTurnOn) : _interface(interface) {
	_up = false;
	can_do_stop(_interface.c_str());
	setBitrate(bitrate);
	setMode(modeToControl, modeToTurnOn);
	openSocket();
}

CAN::~CAN() {
	close(_sock);
	std::cout << "Closed CAN socket" << std::endl;
}

int CAN::setBitrate(unsigned int bitrate) {
	return (can_set_bitrate(_interface.c_str(), bitrate * 1000));
}

int CAN::setMode(unsigned int modeToControl, unsigned int modeToTurnOn) {
	struct can_ctrlmode canMode;

	canMode.mask = modeToControl;
	canMode.flags = modeToControl;
	return (can_set_ctrlmode(_interface.c_str(), &canMode));
}

unsigned int CAN::getActiveMode() const {
	struct can_ctrlmode cm;

	can_get_ctrlmode(_interface.c_str(), &cm);
	printf("CMode %u\n", cm.mask);
	return (cm.flags);
}

int CAN::getSocketFd() const { return (_sock); }

std::string CAN::getInterface() const { return (_interface); }

bool CAN::isUp() const { return (_up); }

unsigned int CAN::getBitrate() const {
	struct can_bittiming bt;

	can_get_bittiming(_interface.c_str(), &bt);
	return (bt.bitrate / 1000);
};

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

int CAN::readFrame(struct can_frame &frame) {
	int nbytes;

	nbytes = read(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in read");
		return (-1);
	}
	return (0);
}

int CAN::openSocket() {
	struct sockaddr_can addr;
	struct ifreq ifr;

	if (can_do_start(_interface.c_str()) < 0) {
		return (-1);
	}
	_up = true;
	_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (_sock < 0) {
		return (-1);
	}
	std::cout << "Created Socket" << std::endl;

	std::strcpy(ifr.ifr_name, _interface.c_str());
	if (ioctl(_sock, SIOCGIFINDEX, &ifr) < 0) {
		return (-1);
	}
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		return (-1);
	}
	std::cout << "Binded Socket" << std::endl;
	return (0);
}

