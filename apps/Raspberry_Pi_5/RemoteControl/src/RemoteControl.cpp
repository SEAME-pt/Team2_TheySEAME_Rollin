#include "RemoteControl.hpp"
#include <iostream>

RemoteControl::RemoteControl() {
	std::cout << "RemoteControl Constructor" << std::endl;
}

RemoteControl::~RemoteControl() {
	std::cout << "RemoteControl Destructor" << std::endl;
}

void RemoteControl::openRemoteControl() {
	_fd = open("/dev/input/js0");
}
