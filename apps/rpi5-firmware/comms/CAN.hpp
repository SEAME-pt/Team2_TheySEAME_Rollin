#pragma once

#include "ICAN.hpp"

class CAN : public ICAN {
public:
	CAN(const char *interface);
	~CAN();

	void readFrame();
	void sendFrame(const canid_t id, const uint8_t *data, const uint8_t len);
	int getSocketFd() const;

private:
	int _sock;
};
