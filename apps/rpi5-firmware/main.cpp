#include "Evdev.hpp"
#include "RemoteControl.hpp"
#include "ActuatorCAN.hpp"
#include "CAN.hpp"
#include "ActuatorKuksa.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>
#include "ActuatorController.hpp"

bool run = true;

void signal_handler(int signal) {
	run = false;
}

//int main() {
//	CAN can("can0", 500, 0, 0);
//	Evdev evdev("/dev/input/event6");
//	RemoteControl remote(evdev);
//	ICar *carKuksa = new CarKuksa(new CarCAN(can, remote));
//
//	carKuksa->setThrottle(100);
//}

int main() {
	struct pollfd fds[2];
	Evdev evdev("/dev/input/event6");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	CarActuator *car = new ActuatorCAN(can, remote);
	//CarActuator *car = new ActuatorKuksa(
	//	new ActuatorCAN(can, remote)
	//);
	ActuatorController ctrl(car, remote);

	std::signal(SIGINT, signal_handler);
	remote.attach(&ctrl);

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;
	while (run) {
		if (poll(fds, 2, 0) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
	}
	return (0);
}
