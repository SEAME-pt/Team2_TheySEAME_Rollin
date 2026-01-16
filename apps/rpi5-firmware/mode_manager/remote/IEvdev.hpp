#pragma once

#include <linux/input.h>

class IEvdev {
public:

	virtual ~IEvdev() {};
	virtual int getfd() const = 0;
	virtual const char *getDevice() const = 0;
	virtual void readEvent() = 0;
	virtual struct input_event &nextEvent() = 0;
	virtual int pendingEvent() const = 0;
};
