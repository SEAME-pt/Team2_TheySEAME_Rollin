#pragma once

#include <linux/can.h>
#include <stdint.h>

class CAN {
public:
	CAN();
	~CAN();

	int getSocketFd() const;
	void openSocket(const char *interface);
	void closeSocket();
	void sendMsg(const canid_t id, const uint8_t *data, const uint8_t len);
	void readMsg();
	void printCANFrame(const struct can_frame frame);

private:
	int _sock;
};
