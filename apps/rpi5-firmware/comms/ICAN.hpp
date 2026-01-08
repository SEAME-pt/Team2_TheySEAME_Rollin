#pragma once

#include <linux/can.h>
#include <stdint.h>

class ICAN {
public:

	virtual ~ICAN() {};
	virtual int getSocketFd() const = 0;
	virtual int readFrame(struct can_frame &frame) = 0;
	virtual int sendFrame(const canid_t id, const uint8_t *data, const uint8_t len) = 0;
};
