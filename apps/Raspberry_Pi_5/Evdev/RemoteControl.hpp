#pragma once

#include "Evdev.hpp"
#include <cstdint>
#include <unordered_map>

class RemoteControl {
public:

	RemoteControl(Evdev &ev);
	~RemoteControl();

	uint8_t getkey(const uint16_t key) const;
	void setkey(const uint16_t keycode, const uint8_t value);
	const Evdev &getEvdev() const;

private:
	Evdev &_ev;
	std::unordered_map<uint16_t, uint8_t> _state;
};
