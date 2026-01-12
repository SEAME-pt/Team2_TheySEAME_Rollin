#pragma once

#include <linux/can.h>
#include <stdint.h>
#include <string>

class ICAN {
public:

	virtual ~ICAN() {};
	virtual int openSocket() = 0;
	virtual int readFrame(struct can_frame &frame) = 0;
	virtual int sendFrame(const canid_t id, const uint8_t *data, const uint8_t len) = 0;
	virtual int getSocketFd() const = 0;
	virtual std::string getInterface() const = 0;
	virtual unsigned int getBitrate() const = 0;
	virtual int setBitrate(unsigned int bitrate) = 0;
	virtual unsigned int getActiveMode() const = 0;
	virtual int setMode(unsigned int modeToControl, unsigned int modeToTurnOn) = 0;
	virtual bool isUp() const = 0;
};
