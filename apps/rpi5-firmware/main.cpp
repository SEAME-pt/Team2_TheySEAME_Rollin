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

//	// PostLine Image
//	cv::cvtColor(mask, mask, cv::COLOR_GRAY2BGR);
//	cv::line(mask, topLeft, bottomLeft, cv::Scalar(0, 0, 255), 10);
//	cv::line(mask, topRight, bottomRight, cv::Scalar(0, 0, 255), 10);
//
//	cv::imwrite("./PostLine.jpg", mask);

int handleFrame(cv::VideoCapture &cam, Lka &lka) {
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	int i = 0;
	
	cv::Mat frameRaw;
	cam.read(frameRaw);
	if (frameRaw.empty()) {
		std::cout << "Failed to get Frame" << std::endl;
		return (-1);
	}
	Frame frame(frameRaw);

	lka.laneCenter(frame);
	return (0);
}

int main() {
	struct pollfd fds[2];
	Evdev evdev("/dev/input/event4");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	//CarActuator *car = new ActuatorCAN(can);
	CarActuator *car = new ActuatorKuksa(
		new ActuatorCAN(can)
	);
	kuksaLib kuksa;
	Lka lka;
	ActuatorController ctrl(car, &remote, &lka, kuksa);
	std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	std::signal(SIGINT, signal_handler);
	lka.attach(&ctrl);
	remote.attach(&ctrl);

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;

	cv::VideoCapture cam("pipe:0", cv::CAP_FFMPEG);
	cam.set(cv::CAP_PROP_FPS, 20);
	if (!cam.isOpened()) {
		std::cout << "Didnt open" << std::endl;
		return (-1);
	}
	while (run.load()) {
		if (poll(fds, 2, 50) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
		if (handleFrame(cam, lka) == -1) {
			break;
		}
	}
	cam.release();

	vhState.join();
	return (0);
}
