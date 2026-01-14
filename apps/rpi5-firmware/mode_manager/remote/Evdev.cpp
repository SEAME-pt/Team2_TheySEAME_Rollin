#include "Evdev.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * @brief Evdev constructor
 *
 * @param evdev device name
 *
 * Opens the evdev in read only mode
 * Also puts the queue count and index to zero
 *
 */
Evdev::Evdev(const char *device) : _device(device) {
	std::cout << "Evdev Constructor" << std::endl;
	_fd = open(_device, O_RDONLY);
	if (_fd < 0) {
		std::perror("Error in open");
	}
	std::memset(_q, 0, sizeof(_q));
	_qCount = 0;
	_qOut = 0;
	std::cout << "Opened evdev" << std::endl;
}

/*
 * @brief Evdev destructor
 *
 * Closes the evdev fd
 *
 */
Evdev::~Evdev() {
	std::cout << "Evdev Destructor" << std::endl;
	if (close(_fd) < 0) {
		std::perror("Error in close");
	}
	std::cout << "Closed evdev" << std::endl;
}

/*
 * @brief Get Evdev fd
 *
 * Returns the Evdev fd
 *
 * @return evdev fd
 *
 */
int Evdev::getfd() const { return (_fd); }

/*
 * @brief Get Evdev name
 *
 * Returns the Evdev name
 *
 * @return evdev name
 *
 */
const char *Evdev::getDevice() const { return (_device); }

/*
 * @brief Read evdev event
 *
 * Read an evdev event. EV_KEY and EV_ABS events go into
 * the event queue and the queue count is incremented
 *
 */
void Evdev::readEvent() {
	struct input_event ev;
	int nbytes;

	nbytes = read(_fd, &ev, sizeof(struct input_event));
	if (nbytes < 0) {
		std::perror("Error in read");
	}
	switch (ev.type) {
		case EV_KEY:
		case EV_ABS:
			_q[_qCount] = ev;
			_qCount++;
			break;

		default:
			break;
	}
}

/*
 * @brief Get event count
 *
 * Returns the current event count in the queue
 *
 * @return queue count
 *
 */
int Evdev::pendingEvent() const { return (_qCount); }

/*
 * @brief Returns an event from the queue
 *
 * Returns the event currently pointed to by the queue index
 * and the queue count is decremented
 * If the queue has no more events the queue index is reseted to zero
 *
 * @return reference to the event popped from the queue
 *
 */
struct input_event &Evdev::nextEvent() {
	struct input_event &ev = _q[_qOut++];

	_qOut = (_qOut + 1) % _qCount;
	_qCount--;
	return (ev);
}

