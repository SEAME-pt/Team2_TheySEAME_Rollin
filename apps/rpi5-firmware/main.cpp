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
#include "Bev.hpp"

std::atomic<bool> run = true;

void signal_handler(int signal) {
	run.store(false);
}

//int main() {
//	float img_h = 7;
//	float img_w = 10;
//	float data[7][10] = {
//		{0, 0, 0, 1, 0, 0, 1, 0, 0, 0},
//		{0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
//		{0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
//		{0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
//		{0, 0, 1, 0, 0, 0, 0, 1, 1, 0},
//		{0, 1, 1, 1, 0, 0, 0, 1, 1, 0},
//		{1, 1, 1, 0, 0, 0, 0, 1, 1, 1},
//	};
//
//	Bev bev(img_h, img_w, data);
//	float srcData[] = {0, img_h, img_w, img_h, 0, 0, img_w, 0};
//	float dstData[] = {2, img_h, 6, img_h, 0, 0, img_w, 0};
//	cv::Mat res;
//
//	bev.createPerspectiveMatrices(srcData, dstData);
//	bev.warp(&res);
//
//	std::cout << res << std::endl;
//}

int main() {
	float img_h = 464;
	float img_w = 1536;

	cv::Mat img = cv::imread("./track.jpeg");
	cv::Mat croppedImg = img(cv::Rect(0, 864 - img_h, img_w, img_h));

	Bev bev(croppedImg);
	float srcRaw[] = {0, img_h, img_w, img_h, 0, 0, img_w, 0};
	float dstRaw[] = {618, img_h, 918, img_h, 0, 0, img_w, 0};
	cv::Mat res;

	bev.createPerspectiveMatrices(srcRaw, dstRaw);
	bev.warp(&res);

	cv::imwrite("464Hcropp.jpg", croppedImg);
	cv::imwrite("300fovbird.jpg", res);
}

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
