#include "Bev.hpp"

Bev::Bev(const int fov, const int frame_h, const int frame_w) {
	_fov = fov;
	float frameH = frame_h;
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

void Bev::slidingWindow(Frame &frame, int startX, int ptnNbr, int rectW, std::vector<cv::Point> &ptns) {
	int x = startX;
	int step_y = frame.getHeight() / ptnNbr;
	int y = frame.getHeight() - step_y;

	cv::Mat color;
	cv::cvtColor(frame.getRawFrame(), color, cv::COLOR_GRAY2BGR);
	for (size_t i = 0; i < ptnNbr; i++) {
		cv::Rect rect(x - (rectW / 2), y, rectW, step_y);
		std::cout << "Point: (" << x << ", " << y << ")" << std::endl;
		int average = checkPixelsInRect(frame, rect);
		if (average != 0) {
			x = average;
		}
		cv::rectangle(color, rect, GREEN, 1);
		ptns.push_back(cv::Point(x, y));
		y -= step_y;
	}
	cv::imwrite("./HV.jpg", color);
}

int Bev::checkPixelsInRect(Frame &frame, cv::Rect &rect) {
	int average = 0;
	int found = 0;

	for (int y = rect.y; y < rect.y + rect.height; y++) {
		for (int x = rect.x; x < rect.x + rect.width; x++) {
			if (frame.getPointValue(x, y) == 255) {
				average += x;
				found++;
				frame.setPointValue(x, y, 0);
				//std::cout << x << " ";
			}
		}
	}
	std::cout << std::endl;
	return (average / found);
}

void Bev::applyBevToFrame(Frame &frame) {
	int cropp = 400;
	int croppH = frame.getHeight() - cropp;

	frame.save("./OrigFrame.jpg");
	frame.cropp(0, cropp, croppH, frame.getWidth());
	frame.transformToBinary();
	frame.open();
	frame.warp(_M);
	frame.save("./WarpFrame.jpg");
}
