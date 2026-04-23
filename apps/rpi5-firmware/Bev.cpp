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
		//std::cout << "Point: (" << x << ", " << y << ")" << std::endl;
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
				//frame.setPointValue(x, y, 0);
				//std::cout << x << " ";
			}
		}
	}
	return (average / found);
}

int Bev::getLaneX() {
	auto it = std::max_element(histogram.begin(), histogram.end());
	int laneX = std::distance(histogram.begin(), it);
	int distance = 200;
	for (size_t i = laneX - distance; i < laneX + distance; i++) {
		histogram[i] = 0;
	}
	return (laneX);
}

void Bev::applyBevToFrame(Frame &frame) {
	frame.save("./OrigFrame.jpg");
	frame.cropp(_roi);
	frame.transformToBinary();
	frame.warp(_M);
	frame.save("./WarpFrame.jpg");
	frame.open();
	frame.save("./Canny.jpg");
	frame.histogram(histogram);
}
