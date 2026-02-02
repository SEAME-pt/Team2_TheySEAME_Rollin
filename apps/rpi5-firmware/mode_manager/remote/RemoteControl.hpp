#pragma once

#include "IEvdev.hpp"
#include <cstdint>
#include <unordered_map>

enum Keys {
	JoyZ = ABS_Z,
	JoyY = ABS_Y,
	Start = BTN_START
};

class RemoteControl {
public:

	/**
	 * @brief RemoteControl constructor
	 *
	 * Prepares the unordered map to set and get ABS_Y and ABS_X values
	 * Also receives an evdev interface
	 *
	 * @param evdev interface reference
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
	 * Returns the current key value in the unordered map
	 *
	 * @param key keycode
	 *
	 * @return Current key value
	 *
	 */
	short getkey(const uint16_t key) const;

	/**
	 * @brief Set a Gamepad key value
	 *
	 * Sets a given key to the given value in the unordered map
	 *
	 * @param key keycode
	 * @param value to set the key to (0|1)
	 *
	 */
	void setkey(const uint16_t keycode, const short value);

	void getEvent();

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
	std::unordered_map<uint16_t, short> _state;
};
