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
#include "Lka.hpp"
#include "Frame.hpp"

std::atomic<bool> run = true;

void signal_handler(int signal) {
	run.store(false);
}

int handleFrame(cv::VideoCapture &cam, Lka &lka) {
	int i = 0;
	
	cv::Mat frameRaw;
	cam.read(frameRaw);
	if (frameRaw.empty()) {
		std::cout << "Failed to get Frame" << std::endl;
		return (-1);
	}
	Frame frame(frameRaw);

	lka.poly(frame);
	return (0);
}

int main() {
	struct pollfd fds[1];
	Evdev evdev("/dev/input/event4");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	kuksaLib kuksa;
	//CarActuator *car = new ActuatorCAN(can);
	CarActuator *car = new ActuatorKuksa(
		new ActuatorCAN(can),
		kuksa
	);
	Lka lka(400, 0, 400, 1536, 464);
	ActuatorController ctrl(car, &remote, &lka, kuksa);

	std::signal(SIGINT, signal_handler);

	lka.attach(&ctrl);
	remote.attach(&ctrl);

	// Kuksa Thread
	//std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	cv::namedWindow("WIN", cv::WINDOW_NORMAL);
	cv::moveWindow("WIN", 0, 0);
	cv::VideoCapture cam("pipe:0");
	cam.set(cv::CAP_PROP_FPS, 20);
	if (!cam.isOpened()) {
		std::cout << "Didnt open" << std::endl;
		return (-1);
	}

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;

	while (run.load()) {
		if (poll(fds, 1, 50) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
		if (handleFrame(cam, lka) == -1) {
			return (0);
		}
	}
	cam.release();
	cv::destroyAllWindows();

	delete car;

	//vhState.join();
	return (0);
}
