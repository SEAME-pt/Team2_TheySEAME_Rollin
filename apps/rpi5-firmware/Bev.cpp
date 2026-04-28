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
	frame.save("./OrigFrame.jpg");
	frame.cropp(_roi);
	frame.transformToBinary(180);
	frame.warp(_M);
	frame.save("./WarpFrame.jpg");
	frame.open();
	frame.save("./Garf.jpg");
}

cv::Mat &Bev::getReverseMatrix() {
	return (_Mreverse);
}
