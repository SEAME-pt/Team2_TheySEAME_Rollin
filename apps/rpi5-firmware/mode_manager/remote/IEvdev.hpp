#pragma once

#include <linux/input.h>

/**
 * @interface IEvdev
 * @brief IEvdev interface
 *
 * The IEvdev interface defines the contract to receive events
 * in the linux evdev interface
 */
class IEvdev {
public:

	virtual ~IEvdev() {};

	/**
	 * @brief Get Evdev fd
	 *
	 * Returns the Evdev fd
	 *
	 * @return evdev fd
	 */
	virtual int getfd() const = 0;

	/**
	 * @brief Get Evdev name
	 *
	 * Returns the Evdev name
	 *
	 * @return evdev name
	 */
	virtual const char *getDevice() const = 0;

	/**
	 * @brief Read evdev event
	 *
	 * Read an evdev event. EV_KEY and EV_ABS events go into
	 * the event queue and the queue count is incremented
	 */
	virtual void readEvent() = 0;

	/**
	 * @brief Returns an event from the queue
	 *
	 * Returns the event currently pointed to by the queue index
	 * and the queue count is decremented
	 * If the queue has no more events the queue index is reseted to zero
	 *
	 * @return reference to the event popped from the queue
	 */
	virtual struct input_event &nextEvent() = 0;

	/**
	 * @brief Get event count
	 *
	 * Returns the current event count in the queue
	 *
	 * @return queue count
	 */
	virtual int pendingEvent() const = 0;
};
