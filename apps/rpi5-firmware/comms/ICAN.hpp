#pragma once

#include <linux/can.h>
#include <stdint.h>

class ICAN {
public:

	virtual ~ICAN() {};
	virtual int getSocketFd() const = 0;
	virtual void readFrame() = 0;
	virtual void sendFrame(const canid_t id, const uint8_t *data, const uint8_t len) = 0;
};
