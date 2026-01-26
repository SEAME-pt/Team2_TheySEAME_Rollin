#include "RemoteControl.hpp"
#include <iostream>

RemoteControl::RemoteControl(IEvdev &ev) : _ev(ev) {
	std::cout << "RemoteControl Constructor" << std::endl;
	_state.insert({ ABS_X, 0});
	_state.insert({ ABS_Y, 0});
}

RemoteControl::~RemoteControl() {
	std::cout << "RemoteControl Destructor" << std::endl;
}

void RemoteControl::setkey(const uint16_t keycode, const uint8_t value) {
	_state[keycode] = value;
}

uint8_t RemoteControl::getkey(const uint16_t key) const { return (_state.at(key)); }

const IEvdev &RemoteControl::getEvdev() const { return (_ev); }
