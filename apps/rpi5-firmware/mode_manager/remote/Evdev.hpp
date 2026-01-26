#pragma once

#include <cstdint>
#include "IEvdev.hpp"

class Evdev : public IEvdev {
public:

	/**
	 * @brief Evdev constructor
	 *
	 * @param evdev device name
	 *
	 * Opens the evdev in read only mode
	 * Also puts the queue count and index to zero
	 *
	 */
	Evdev(const char *device);

	/**
	 * @brief Evdev destructor
	 *
	 * Closes the evdev fd
	 *
	 */
	~Evdev();

	int getfd() const;
	const char *getDevice() const;
	void readEvent();
	struct input_event &nextEvent();
	int pendingEvent() const;

private:
	int _fd;
	const char *_device;
	struct input_event _q[32];
	uint8_t _qOut;
	uint8_t _qCount;
};
