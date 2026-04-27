#include "ActuatorController.hpp"
#include "CAN.hpp"
#include "ActuatorCAN.hpp"
#include "Lka.hpp"
#include <csignal>

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
	CAN can("can0", 500, 0, 0);
	CarActuator *car = new ActuatorCAN(can);
	Lka lka(400, 0, 400, 2304, 464);
	kuksaLib kuksa;
	ActuatorController ctrl(car, NULL, &lka, kuksa);

	std::signal(SIGINT, signal_handler);
	lka.attach(&ctrl);

	cv::namedWindow("WIN", cv::WINDOW_NORMAL);
	cv::moveWindow("WIN", 0, 0);
	cv::VideoCapture cam("pipe:0");
	cam.set(cv::CAP_PROP_FPS, 20);
	if (!cam.isOpened()) {
		std::cout << "Didnt open" << std::endl;
		return (-1);
	}

	while (run.load()) {
		usleep(50000);
		if (handleFrame(cam, lka) == -1) {
			break;
		}
	}
	cam.release();
	cv::destroyAllWindows();

	delete car;
}
