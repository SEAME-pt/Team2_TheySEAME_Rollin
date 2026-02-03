#include "RemoteControl.hpp"
#include <iostream>

RemoteControl::RemoteControl(IEvdev &ev) : _ev(ev) {
	std::cout << "RemoteControl Constructor" << std::endl;
	_state.insert({ JoyZ, 0});
	_state.insert({ JoyY, 0});
	_state.insert({ Start, 0});
}

RemoteControl::~RemoteControl() {
	std::cout << "RemoteControl Destructor" << std::endl;
}

void RemoteControl::setkey(const uint16_t keycode, const short value) {
	_state[keycode] = value;
}

short RemoteControl::getkey(const uint16_t key) const { return (_state.at(key)); }

void RemoteControl::getEvent() {
	while (_ev.pendingEvent() > 0) {
		struct input_event &event = _ev.nextEvent();
		switch (event.type) {
			case EV_ABS:
			case EV_KEY:
				setkey(event.code, event.value);
		}
	}
}

const IEvdev &RemoteControl::getEvdev() const { return (_ev); }
