#pragma once

#include <linux/can.h>
#include <stdint.h>
#include "Comms.hpp"

class CAN : public Comms {
public:
	CAN(const char *interface);
	~CAN();

	int getSocketFd() const;
	void readComm();
	void sendComm(const canid_t id, const uint8_t *data, const uint8_t len);
	void printCANFrame(const struct can_frame frame);

private:
	int _sock;
};
