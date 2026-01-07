#pragma once

class Comms {
public:

	virtual ~Comms();
	virtual void readComm() = 0;
	virtual void sendComm() = 0;
};
