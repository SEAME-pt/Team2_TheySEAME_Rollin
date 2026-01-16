#pragma once

#include "IEvdev.hpp"
#include <cstdint>
#include <unordered_map>

class RemoteControl {
public:

	RemoteControl(IEvdev &ev);
	~RemoteControl();

	uint8_t getkey(const uint16_t key) const;
	void setkey(const uint16_t keycode, const uint8_t value);
	const IEvdev &getEvdev() const;

private:
	IEvdev &_ev;
	std::unordered_map<uint16_t, uint8_t> _state;
};
