#pragma once

#include <cstdint>
#include "IEvdev.hpp"

class Evdev : public IEvdev {
public:

	Evdev(const char *device);
	~Evdev();

	int getfd() const;
	const char *getDevice() const;
	void readEvent();
	struct input_event &nextEvent();
	int pendingEvent() const;
	void printQueue();

private:
	int _fd;
	const char *_device;
	struct input_event _q[32];
	uint8_t _qOut;
	uint8_t _qCount;
};
