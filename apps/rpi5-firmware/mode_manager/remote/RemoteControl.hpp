#pragma once

#include "IEvdev.hpp"
#include <cstdint>
#include <unordered_map>

class RemoteControl {
public:

	/**
	 * @brief RemoteControl constructor
	 *
	 * @param evdev interface reference
	 *
	 * Prepares the unordered map to set and get ABS_Y and ABS_X values
	 * Also receives an evdev interface
	 *
	 */
	RemoteControl(IEvdev &ev);

	/**
	 * @brief RemoteControl destructor
	 *
	 * Destructs the RemoteControl
	 *
	 */
	~RemoteControl();

	/**
	 * @brief Get a Gamepad value
	 *
	 * @param key keycode
	 *
	 * Returns the current key value in the unordered map
	 *
	 * @return Current key value
	 *
	 */
	uint8_t getkey(const uint16_t key) const;

	/**
	 * @brief Set a Gamepad key value
	 *
	 * @param key keycode
	 * @param value to set the key to (0|1)
	 *
	 * Sets a given key to the given value in the unordered map
	 *
	 */
	void setkey(const uint16_t keycode, const uint8_t value);

	/**
	 * @brief Get evedev
	 *
	 * Returns the evdev used to poll events
	 *
	 * @return reference to evdev interface
	 *
	 */
	const IEvdev &getEvdev() const;

private:
	IEvdev &_ev;
	std::unordered_map<uint16_t, uint8_t> _state;
};
