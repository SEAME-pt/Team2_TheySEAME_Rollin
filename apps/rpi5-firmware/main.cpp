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
	
	//cv::Mat frameRaw;
	//cam.read(frameRaw);
	//if (frameRaw.empty()) {
	//	std::cout << "Failed to get Frame" << std::endl;
	//	return (-1);
	//}
	cv::Mat frameRaw = cv::imread("./OrigFrame.jpg");
	Frame frame(frameRaw);

	lka.poly(frame);
	return (0);
}

//int main() {
//	cv::Mat img = cv::imread("./OrigFrame.jpg");
//	Frame frame(img);
//	Bev bev(400, 864, 1536);
//	std::vector<cv::Vec4i> lines;
//
//	bev.applyBevToFrame(frame);
//
//	cv::Mat edge;
//	cv::Canny(frame.getRawFrame(), edge, 50, 200);
//	cv::imwrite("./Canny.jpg", edge);
//	std::vector<int> tmp;
//	cv::reduce(edge, tmp, 0, cv::REDUCE_SUM, CV_32S);
//
//	auto hv = std::max_element(tmp.begin(), tmp.end());
//	int lane1_x = std::distance(tmp.begin(), hv);
//	*hv = -1;
//	hv = std::max_element(tmp.begin(), tmp.end());
//	int lane2_x = std::distance(tmp.begin(), hv);
//
//	cv::Mat color;
//	cv::cvtColor(edge, color, cv::COLOR_GRAY2BGR);
//	int average = lane1_x;
//	int step_y = frame.getHeight() / 8;
//	int y = frame.getHeight() - step_y;
//
//	cv::Mat A(8, 3, CV_32F);
//	cv::Mat B(8, 1, CV_32F);
//	for (int i = 0; i < 8; i++) {
//		cv::Rect rect(average - 100, y, 175, step_y);
//		std::cout << "Point: (" << average << ", " << y << ")" << std::endl;
//		average = slidingWindow(edge, rect);
//		cv::rectangle(color, rect, GREEN, 3);
//		y -= step_y;
//	}
//	cv::Point carPos(frame.getWidth() / 2, frame.getHeight());
//	cv::line(color, carPos, cv::Point(frame.getWidth() / 2, frame.getWidth() - 50), RED, 10);
//	cv::Mat coeffs;
//	cv::solve(A, B, coeffs, cv::DECOMP_NORMAL);
//
//	double a = coeffs.at<double>(0, 0);
//    double b = coeffs.at<double>(1, 0);
//    double c = coeffs.at<double>(2, 0);
//
//    std::cout << "a=" << a << " b=" << b << " c=" << c << std::endl;
//
//
//	//cv::line(color, cv::Point(lane1_x, 0), cv::Point(lane1_x, 464), RED, 10);
//	//cv::line(color, cv::Point(lane2_x, 0), cv::Point(lane2_x, 464), RED, 10);
//	cv::imwrite("./HV.jpg", color);
//	//for (int i = 0; i < tmp.size(); i++) {
//	//	std::cout << tmp[i] << std::endl;
//	//}
//	//cv::cvtColor(edge, tmp, cv::COLOR_GRAY2BGR);
//	//cv::HoughLinesP(edge, lines, 1, CV_PI / 180, 100);
//
//	//std::cout << "Size: " << lines.size() << std::endl;
//	//for (size_t i = 0; i < lines.size(); i++) {
//	//	std::cout << lines[i] << std::endl;
//	//	cv::Point pt1(lines[i][0], lines[i][1]);
//	//	cv::Point pt2(lines[i][2], lines[i][3]);
//	//	cv::line(tmp, pt2, pt2, RED, 5);
//	//}
//	//cv::imwrite("./Hough.jpg", tmp);
//}

int main() {
	struct pollfd fds[1];
	Evdev evdev("/dev/input/event4");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	CarActuator *car = new ActuatorCAN(can);
	//CarActuator *car = new ActuatorKuksa(
	//	new ActuatorCAN(can)
	//);
	kuksaLib kuksa;
	Lka lka;
	ActuatorController ctrl(car, &remote, &lka, kuksa);
	//std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	cv::VideoCapture cam;
	//cam.set(cv::CAP_PROP_FPS, 20);
	//if (!cam.isOpened()) {
	//	std::cout << "Didnt open" << std::endl;
	//	//return (-1);
	//}

	std::signal(SIGINT, signal_handler);
	lka.attach(&ctrl);
	remote.attach(&ctrl);

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;

	if (handleFrame(cam, lka) == -1) {
		return (0);
	}
	while (run.load()) {
		if (poll(fds, 1, 50) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
	}
	cam.release();

	//vhState.join();
	return (0);
}
