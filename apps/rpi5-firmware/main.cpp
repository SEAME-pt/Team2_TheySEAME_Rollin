#include "Evdev.hpp"
#include "CarCAN.hpp"
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
	struct pollfd fds[2];
	CAN can("can0", 500, 0, 0);
	Evdev evdev("/dev/input/event6");
	RemoteControl remote(evdev);
	CarCAN car(can, remote);

	std::signal(SIGINT, signal_handler);
	remote.attach(&car);

	fds[0].fd = can.getSocketFd();
	fds[0].events = POLLIN;
	fds[1].fd = evdev.getfd();
	fds[1].events = POLLIN;
	while (run) {
		if (poll(fds, 2, 0) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			//printf("Receiving frame\n");
			struct can_frame frame;
			can.readFrame(frame);
		}
		if (fds[1].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
	}
}

int main() {
	Evdev evdev("/dev/input/event4");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	kuksaLib kuksa;
	CarActuator *car = new ActuatorCAN(can);
	//CarActuator *car = new ActuatorKuksa(
	//	new ActuatorCAN(can),
	//	kuksa
	//);
	//Lka lka(400, 0, 250, 960, 390); // Carla Setup
	Lka lka(400, 0, 400, 1536, 464, 8); // Track Setup
	ActuatorController ctrl(car, &remote, &lka, kuksa);

	lka.attach(&ctrl);
	remote.attach(&ctrl);

	std::signal(SIGINT, signal_handler);

	//std::thread lkaThread(pathPlanning, &lka);
	std::thread remoteThread(remoteControl, &remote, &evdev);
	// Kuksa Thread
	//std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	//while (run.load()) {
	//	usleep(50000);
	//	ctrl.test();
	//}

	//lkaThread.join();
	remoteThread.join();
	//vhState.join();
	delete car;

	return (0);
}
