#include "RemoteControl.hpp"
#include <iostream>
#include <stdio.h>

RemoteControl::RemoteControl(IEvdev &ev) : _ev(ev) {
	std::cout << "RemoteControl Constructor" << std::endl;
	_state.insert({ JoyY, 0 });
	_state.insert({ JoyZ, 0 });
	_state.insert({ Start, 0 });
	_state.insert({ L2, 0 });
	_state.insert({ A, 0 });
	_state.insert({ Y, 0 });
	_state.insert({ X, 0 });
	_state.insert({ B, 0 });
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
		printf("Event %x, %x\n", event.type, event.code);
		switch (event.type) {
			case EV_ABS:
				setkey(event.code, event.value);
				if (event.code == JoyY) {
					notify(Events::CAR_THROTTLE);
				} else if (event.code == JoyZ) {
					notify(Events::CAR_STEERING);
				} else if (event.code == DpadY) {
					notify(Events::CAR_CRUISE_CONTROL);
				}
				break;
			case EV_KEY:
				setkey(event.code, event.value);
				if (event.code == L2) {
					notify(Events::CAR_BRAKE);
				} else if (event.code == B || event.code == X
					|| event.code == A || event.code == Y) {
					notify(Events::CAR_GEAR);
				}
				break;
		}
	}
}

const IEvdev &RemoteControl::getEvdev() const { return (_ev); }
