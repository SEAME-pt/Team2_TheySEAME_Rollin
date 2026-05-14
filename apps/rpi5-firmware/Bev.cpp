#include "Bev.hpp"

Bev::Bev(const int fov, const cv::Rect &roi) {
	_fov = fov;
	_roi = roi;
	float frameH = roi.height;
	float frameW = roi.width;
	float srcData[] = { \
		0, frameH, \
		frameW, frameH, \
		0, 0, \
		frameW, 0
	};
	float dstData[] = {
		(frameW / 2) - ((float)_fov / 2), frameH, \
		(frameW / 2) + ((float)_fov / 2), frameH, \
		0, 0, \
		frameW, 0
	};
	_M = cv::getPerspectiveTransform(
		cv::Mat(4, 2, CV_32F, srcData), \
		cv::Mat(4, 2, CV_32F, dstData)
	);
	_Mreverse = cv::getPerspectiveTransform(
		cv::Mat(4, 2, CV_32F, dstData), \
		cv::Mat(4, 2, CV_32F, srcData)
	);
}

Bev::~Bev() {}

void Bev::applyBevToFrame(Frame &frame) {
	frame.cropp(_roi);
	// Debug Cropp Line
	//cv::Point pt1(0, _roi.y);
	//cv::Point pt2(frame.getWidth(), _roi.y);
	//show.drawLine(pt1, pt2, RED, 5);
	//show.showInScreen("WIN");
	show.cropp(_roi);
	frame.warp(_M);
	frame.open();
	show.warp(_M);
	show.open();
	// Show Bev Frame
	//frame.showInScreen("WIN");
	//show.showInScreen("WIN2");
}

cv::Mat &Bev::getReverseMatrix() {
	return (_Mreverse);
}
