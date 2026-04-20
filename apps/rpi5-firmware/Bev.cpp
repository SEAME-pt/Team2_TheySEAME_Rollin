#include "Bev.hpp"

Bev::Bev(const int fov, const int frame_h, const int frame_w) {
	_fov = fov;
	float frameH = frame_h - _fov;
	float frameW = frame_w;
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
}

Bev::~Bev() {}

void Bev::applyBevToFrame(Frame &frame) {
	frame.save("./OrigFrame.jpg");
	frame.cropp();
	frame.transformToBinary();
	frame.open();
	frame.save("./BinaryFrame.jpg");
	frame.warp(_M);
	frame.save("./WarpFrame.jpg");
}
