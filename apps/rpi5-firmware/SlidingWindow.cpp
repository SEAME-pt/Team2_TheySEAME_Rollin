#include "SlidingWindow.hpp"

SlidingWindow::SlidingWindow() {}

SlidingWindow::~SlidingWindow() {}

void SlidingWindow::getLanePtns(Frame &frame, const int nbrPtns, std::vector<cv::Point> &leftLanePtns, std::vector<cv::Point> &rightLanePtns) {
	std::vector<int> histogram;
	int rectW = frame.getWidth() / 8;
	int laneX1, laneX2;

	frame.histogram(histogram);
	laneX1 = getLaneX(histogram);
	laneX2 = getLaneX(histogram);
	slidingWindow(frame, laneX1, nbrPtns, rectW, leftLanePtns);
	slidingWindow(frame, laneX2, nbrPtns, rectW, rightLanePtns);
	if (laneX1 > laneX2) {
		std::swap(leftLanePtns, rightLanePtns);
	}
	//std::cout << "Lanes: " << laneX1 << " " << laneX2 << std::endl;
}

void SlidingWindow::slidingWindow(Frame &frame, int startX, int ptnsNbr, int rectW, std::vector<cv::Point> &ptns) {
	int x = startX;
	int step_y = frame.getHeight() / ptnsNbr;
	int y = frame.getHeight() - step_y;

	for (size_t i = 0; i < ptnsNbr; i++) {
		cv::Rect rect(x - (rectW / 2), y, rectW, step_y);
		//std::cout << "Point: (" << x << ", " << y + (step_y / 2)<< ")" << std::endl;
		int average = checkPixelsInRect(frame, rect);
		if (average != 0) {
			x = average;
		}
		//cv::rectangle(color, rect, GREEN, 1);
		ptns.push_back(cv::Point(x, y + (step_y / 2)));
		y -= step_y;
	}
}

int SlidingWindow::checkPixelsInRect(Frame &frame, cv::Rect &rect) {
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

int SlidingWindow::getLaneX(std::vector<int> &histogram) {
	auto it = std::max_element(histogram.begin(), histogram.end());
	int laneX = std::distance(histogram.begin(), it);
	int distance = 200;
	for (size_t i = laneX - distance; i < laneX + distance; i++) {
		histogram[i] = 0;
	}
	return (laneX);
}

