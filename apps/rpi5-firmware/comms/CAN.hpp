#pragma once

#include "ICAN.hpp"
#include <net/if.h>
#include <libsocketcan.h>

class CAN : public ICAN {
public:
	CAN(const std::string &interface);
	CAN(const std::string &interface, const unsigned int bitrate,
		unsigned int modeToControl, unsigned int modeToTurnOn);
	~CAN();

	int openSocket();
	int readFrame(struct can_frame &frame);
	int sendFrame(const canid_t id, const uint8_t *data, const uint8_t len);
	int getSocketFd() const;
	std::string getInterface() const;
	unsigned int getBitrate() const;
	int setBitrate(unsigned int bitrate);
	unsigned int getActiveMode() const;
	int setMode(unsigned int modeToControl, unsigned int modeToTurnOn);
	bool isUp() const;

private:
	const std::string _interface;
	bool _up;
	int _sock;
};
