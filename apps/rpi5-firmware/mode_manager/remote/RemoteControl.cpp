#include "RemoteControl.hpp"
#include <iostream>

/*
 * @brief RemoteControl constructor
 *
 * @param evdev interface reference
 *
 * Prepares the unordered map to set and get ABS_Y and ABS_X values
 * Also receives an evdev interface
 *
 */
RemoteControl::RemoteControl(IEvdev &ev) : _ev(ev) {
	std::cout << "RemoteControl Constructor" << std::endl;
	_state.insert({ ABS_X, 0});
	_state.insert({ ABS_Y, 0});
}

/*
 * @brief RemoteControl destructor
 *
 * Destructs the RemoteControl
 *
 */
RemoteControl::~RemoteControl() {
	std::cout << "RemoteControl Destructor" << std::endl;
}

/*
 * @brief Set a Gamepad key value
 *
 * @param key keycode
 * @param value to set the key to (0|1)
 *
 * Sets a given key to the given value in the unordered map
 *
 */
void RemoteControl::setkey(const uint16_t keycode, const uint8_t value) {
	_state[keycode] = value;
}

/*
 * @brief Get a Gamepad value
 *
 * @param key keycode
 *
 * Returns the current key value in the unordered map
 *
 * @return Current key value
 *
 */
uint8_t RemoteControl::getkey(const uint16_t key) const { return (_state.at(key)); }

/*
 * @brief Get evedev
 *
 * Returns the evdev used to poll events
 *
 * @return reference to evdev interface
 *
 */
const IEvdev &RemoteControl::getEvdev() const { return (_ev); }
