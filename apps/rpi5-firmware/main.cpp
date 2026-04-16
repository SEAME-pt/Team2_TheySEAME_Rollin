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
#include <opencv4/opencv2/highgui.hpp>
#include "Lka.hpp"

std::atomic<bool> run = true;

void signal_handler(int signal) {
	run.store(false);
}

#define LEFT -1
#define RIGHT 1

cv::Point searchLanes(cv::Mat &frame, uint row, int dir) {
	cv::Size size = frame.size();
	int x = size.width / 2;

	while (x > 0 && x < size.width) {
		uchar pixel = frame.at<uchar>(row, x);
		//printf("(%d, %d) = %d\n", row, x, pixel);
		if (pixel == 255) {
			//std::cout << "Found Point" << std::endl;
			break;
		}
		x += dir;
	}
	return (cv::Point(x, row));
}

//int main() {
//	//cv::Mat grayimg;
//	//cv::Mat binaryMask;
//	cv::Mat img;
//	cv::Mat mask;
//	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
//	std::vector<std::vector<cv::Point> > contours;
//
//	img = cv::imread("./400fovbird.jpg", cv::IMREAD_COLOR);
//	cv::cvtColor(img, grayimg, cv::COLOR_BGR2GRAY);
//	cv::threshold(grayimg, binaryMask, 127, 255, cv::THRESH_BINARY);
//	cv::imwrite("./BinaryMask.jpg", binaryMask);
//
//	img = cv::imread("./BinaryMask.jpg", cv::IMREAD_GRAYSCALE);
//	cv::erode(img, mask, kernel);
//	cv::dilate(mask, mask, kernel);
//	cv::medianBlur(mask, mask, 9);
//
//	cv::imwrite("./Blur.jpg", mask);
//
//	cv::Point carOrigin(mask.size().width / 2, mask.size().height / 2);
//	int topRow = carOrigin.y - 125;
//	cv::Point topLeft = searchLanes(mask, topRow, LEFT);
//	cv::Point topRight = searchLanes(mask, topRow, RIGHT);
//
//	int bottomRow = carOrigin.y + 125;
//	cv::Point bottomLeft = searchLanes(mask, bottomRow, LEFT);
//	cv::Point bottomRight = searchLanes(mask, bottomRow, RIGHT);
//
//	cv::Point laneCenter((topLeft.x + topRight.x) / 2, (topLeft.y + topRight.y) / 2);
//	float distX = laneCenter.x - carOrigin.x;
//	float distY = laneCenter.y - carOrigin.y;
//
//	std::cout << "Car: " << carOrigin << " " << "Lane: " << laneCenter << std::endl;
//	std::cout << "Distances: " << distY << " " << distX << std::endl;
//	std::cout << "Angle: " << atan(distX / distY) * (180 / M_PI) << std::endl;
//	//std::cout << "Angle: " << cv::fastAtan2(carOrigin.y - laneCenter.y, carOrigin.x - laneCenter.x) << std::endl;
//	//double dist = cv::norm(laneCenter - carOrigin);
//
//	// PostLine Image
//	cv::cvtColor(mask, mask, cv::COLOR_GRAY2BGR);
//	cv::line(mask, topLeft, bottomLeft, cv::Scalar(0, 0, 255), 10);
//	cv::line(mask, topRight, bottomRight, cv::Scalar(0, 0, 255), 10);
//
//	cv::imwrite("./PostLine.jpg", mask);
//}

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

int handleFrame(cv::VideoCapture &cam, Lka &lka) {
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	float img_h = 464;
	float img_w = 1536;
	int i = 0;
	
	cv::Mat frameRaw;
	cam.read(frameRaw);
	if (frameRaw.empty()) {
		std::cout << "Failed to get Frame" << std::endl;
		return (-1);
	}
	cv::Mat frame = frameRaw(cv::Rect(0, 864 - img_h, img_w, img_h));

	float srcRaw[] = {0, img_h, img_w, img_h, 0, 0, img_w, 0};
	float dstRaw[] = {568, img_h, 968, img_h, 0, 0, img_w, 0};
	Bev bev(frame);
	bev.createPerspectiveMatrices(srcRaw, dstRaw);
	bev.warp(&frame);
	cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
	cv::threshold(frame, frame, 127, 255, cv::THRESH_BINARY);
	cv::erode(frame, frame, kernel);
	cv::dilate(frame, frame, kernel);
	cv::medianBlur(frame, frame, 9);

	cv::Point carOrigin(img_w / 2, img_h - 50);
	int topRow = img_h - 125;
	cv::Point topLeft = searchLanes(frame, topRow, LEFT);
	cv::Point topRight = searchLanes(frame, topRow, RIGHT);

	int bottomRow = img_h + 125;
	cv::Point bottomLeft = searchLanes(frame, bottomRow, LEFT);
	cv::Point bottomRight = searchLanes(frame, bottomRow, RIGHT);

	cv::Point laneCenter((topLeft.x + topRight.x) / 2, (topLeft.y + topRight.y) / 2);
	float distX = laneCenter.x - carOrigin.x;
	float distY = carOrigin.y - laneCenter.y;

	std::cout << "Car: " << carOrigin << " " << "Lane: " << laneCenter << std::endl;
	std::cout << "Distances: " << distY << " " << distX << std::endl;
	int angle = atan(distX / distY) * (180 / M_PI);
	std::cout << "Angle: " << angle << std::endl;
	lka.setAngle(std::clamp(angle, -30, 30));
	lka.notify(Events::CAR_STEERING);

	//std::cout << "Frame " << i++ << std::endl;
	return (0);
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
	Lka lka;
	ActuatorController ctrl(car, &remote, &lka, kuksa);
	//std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	std::signal(SIGINT, signal_handler);
	lka.attach(&ctrl);
	remote.attach(&ctrl);

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;

	cv::VideoCapture cam("./badlane.mjpeg", cv::CAP_FFMPEG);
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

	//vhState.join();
	return (0);
}
