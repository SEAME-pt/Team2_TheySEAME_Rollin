#pragma once

#include "ICAN.hpp"
#include <net/if.h>

class CAN : public ICAN {
public:
	CAN(const std::string &interface);
	~CAN();

	int readFrame(struct can_frame &frame);
	int sendFrame(const canid_t id, const uint8_t *data, const uint8_t len);
	int getSocketFd() const;
	std::string getInterface() const;
	int getBitrate() const;
	bool isUp() const;
	bool isRunning() const;

private:
	int sendDumpReq();
	int netlinkSocket();
	int getAttr();

	struct ifreq _ifr;
	int _sock;
	int _nlSock;
};
