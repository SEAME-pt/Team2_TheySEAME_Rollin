#include "Evdev.hpp"
#include "RemoteControl.hpp"
#include "ActuatorCAN.hpp"
#include "CAN.hpp"
#include "ActuatorKuksa.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>
#include <stdio.h>
#include <thread>
#include "ActuatorController.hpp"
#include <opencv4/opencv2/highgui.hpp>

std::atomic<bool> run = true;

void signal_handler(int signal) {
	run.store(false);
}

int main() {
	struct pollfd fds[1];
	Evdev evdev("/dev/input/event4");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	kuksaLib kuksa;
	CarActuator *car = new ActuatorCAN(can);
	//CarActuator *car = new ActuatorKuksa(
	//	new ActuatorCAN(can),
	//	kuksa
	//);
	ActuatorController ctrl(car, &remote, NULL, kuksa);

	std::signal(SIGINT, signal_handler);

	remote.attach(&ctrl);

	// Kuksa Thread
	//std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;

	while (run.load()) {
		if (poll(fds, 1, 100) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
	}

	delete car;

	//vhState.join();
	return (0);
}
