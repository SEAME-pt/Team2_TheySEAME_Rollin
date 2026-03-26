#include "Evdev.hpp"
#include "RemoteControl.hpp"
#include "ActuatorCAN.hpp"
#include "CAN.hpp"
#include "ActuatorKuksa.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>
#include <thread>
#include "ActuatorController.hpp"

std::atomic<bool> run = true;

void signal_handler(int signal) {
	run.store(false);
}

int main() {
	struct pollfd fds[2];
	Evdev evdev("/dev/input/event4");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	//CarActuator *car = new ActuatorCAN(can, remote);
	CarActuator *car = new ActuatorKuksa(
		new ActuatorCAN(can, remote)
	);
	kuksaLib kuksa;
	ActuatorController ctrl(car, remote, kuksa);
	std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	std::signal(SIGINT, signal_handler);
	remote.attach(&ctrl);

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;
	while (run.load()) {
		if (poll(fds, 2, 0) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
	}

	vhState.join();
	return (0);
}
