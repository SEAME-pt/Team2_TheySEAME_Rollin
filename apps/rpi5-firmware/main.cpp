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
#include "Bev.hpp"

std::atomic<bool> run = true;

void signal_handler(int signal) {
	run.store(false);
}

//int main() {
//	uint8_t img_h = 2;
//	uint8_t img_w = 5;
//	uint8_t data[2][5] = {
//		{1, 0, 0, 0, 1},
//		{1, 0, 0, 0, 1},
//	};
//	cv::Mat img(img_h, img_w, CV_8UC1, data);
//
//	std::vector<std::vector<cv::Point> >  contours;
//
//	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
//
//	int i = 0;
//	for (auto it = contours.begin(); it != contours.end(); it++) {
//		std::cout << "Vector " << i++ << "\n";
//		for (auto it2 = it->begin(); it2 != it->end(); it2++) {
//			std::cout << "Point: " << *it2 << "\n";
//		}
//	}
//
//	cv::Mat dist;
//
//	cv::distanceTransform(img, dist, cv::DIST_L2, 3);
//
//	std::cout << "DistanceArray: " << dist << "\n";
//	std::cout << "DistanceInTheCenter: " << dist.at<float>(0, 1) << std::endl; 
//	std::cout << "DistanceInTheCenter: " << dist.at<float>(1, 0) << std::endl; 
//	std::cout << std::endl;
//}

#define LEFT -1
#define RIGHT 1

cv::Point searchLanes(cv::Mat &frame, uint row, int dir) {
	cv::Size size = frame.size();
	int x = size.width / 2;

	while (x > 0 && x < size.width) {
		uchar pixel = frame.at<uchar>(row, x);
		printf("(%d, %d) = %d\n", row, x, pixel);
		if (pixel == 255) {
			std::cout << "Found Point" << std::endl;
			break;
		}
		x += dir;
	}
	return (cv::Point(x, row));
}

int main() {
	//cv::Mat grayimg;
	//cv::Mat binaryMask;
	cv::Mat img;
	cv::Mat mask;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	std::vector<std::vector<cv::Point> > contours;

	//img = cv::imread("./400fovbird.jpg", cv::IMREAD_COLOR);
	//cv::cvtColor(img, grayimg, cv::COLOR_BGR2GRAY);
	//cv::threshold(grayimg, binaryMask, 127, 255, cv::THRESH_BINARY);
	//cv::imwrite("./BinaryMask.jpg", binaryMask);

	img = cv::imread("./BinaryMask.jpg", cv::IMREAD_GRAYSCALE);
	cv::erode(img, mask, kernel);
	cv::dilate(mask, mask, kernel);
	cv::medianBlur(mask, mask, 9);

	cv::imwrite("./Blur.jpg", mask);

	cv::Point carOrigin(mask.size().width / 2, mask.size().height / 2);
	int topRow = mask.size().height / 2 - 125;
	cv::Point topLeft = searchLanes(mask, topRow, LEFT);
	cv::Point topRight = searchLanes(mask, topRow, RIGHT);

	int bottomRow = mask.size().height / 2 + 125;
	cv::Point bottomLeft = searchLanes(mask, bottomRow, LEFT);
	cv::Point bottomRight = searchLanes(mask, bottomRow, RIGHT);

	cv::Point laneCenter((topLeft.x + topRight.x) / 2, (topLeft.y + topRight.y) / 2);
	int distX = laneCenter.x - carOrigin.x;
	int distY = laneCenter.y - carOrigin.y;

	std::cout << "Distances: " << distY << " " << distX << std::endl;
	std::cout << "Angle: " << cv::fastAtan2(laneCenter.y, carOrigin.x) << std::endl;
	//double dist = cv::norm(laneCenter - carOrigin);

	std::cout << "Car: " << carOrigin << " " << "Lane: " << laneCenter << std::endl;
	std::cout << "Distance: " << distX << std::endl;

	// PostLine Image
	cv::cvtColor(mask, mask, cv::COLOR_GRAY2BGR);
	cv::line(mask, topLeft, bottomLeft, cv::Scalar(0, 0, 255), 10);
	cv::line(mask, topRight, bottomRight, cv::Scalar(0, 0, 255), 10);

	cv::imwrite("./PostLine.jpg", mask);
}

//int main() {
//	float img_h = 464;
//	float img_w = 1536;
//
//	cv::Mat img = cv::imread("./track.jpeg");
//	cv::Mat croppedImg = img(cv::Rect(0, 864 - img_h, img_w, img_h));
//
//	Bev bev(croppedImg);
//	float srcRaw[] = {0, img_h, img_w, img_h, 0, 0, img_w, 0};
//	float dstRaw[] = {618, img_h, 918, img_h, 0, 0, img_w, 0};
//	cv::Mat res;
//
//	bev.createPerspectiveMatrices(srcRaw, dstRaw);
//	bev.warp(&res);
//
//	cv::imwrite("464Hcropp.jpg", croppedImg);
//	cv::imwrite("300fovbird.jpg", res);
//}

//int main() {
//	struct pollfd fds[2];
//	Evdev evdev("/dev/input/event6");
//	RemoteControl remote(evdev);
//	CAN can("can0", 500, 0, 0);
//	//CarActuator *car = new ActuatorCAN(can, remote);
//	CarActuator *car = new ActuatorKuksa(
//		new ActuatorCAN(can, remote)
//	);
//	kuksaLib kuksa;
//	ActuatorController ctrl(car, remote, kuksa);
//	std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);
//
//	std::signal(SIGINT, signal_handler);
//	remote.attach(&ctrl);
//
//	fds[0].fd = evdev.getfd();
//	fds[0].events = POLLIN;
//	while (run.load()) {
//		if (poll(fds, 2, 0) < 0) {
//			perror("Error in poll:");
//			break;
//		}
//		if (fds[0].revents & POLLIN) {
//			evdev.readEvent();
//			remote.getEvent();
//		}
//	}
//
//	vhState.join();
//	return (0);
//}
