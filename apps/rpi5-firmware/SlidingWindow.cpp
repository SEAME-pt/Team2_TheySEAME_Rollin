#include "SlidingWindow.hpp"
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

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

	cv::Mat color = frame.getMatObj();
	cv::cvtColor(frame.getMatObj(), color, cv::COLOR_GRAY2BGR);
	for (size_t i = 0; i < ptnsNbr; i++) {
		cv::Rect rect(x - (rectW / 2), y, rectW, step_y);
		std::cout << "Point: (" << x << ", " << y + (step_y / 2)<< ")" << std::endl;
		int average = checkPixelsInRect(frame, rect);
		if (average != 0) {
			x = average;
		}
		cv::rectangle(color, rect, GREEN, 1);
		ptns.push_back(cv::Point(x, y + (step_y / 2)));
		y -= step_y;
	}
	cv::imwrite("./HV.jpg", color);
}

int SlidingWindow::checkPixelsInRect(Frame &frame, cv::Rect &rect) {
	int average = 0;
	int found = 0;
	size_t startX = rect.x;
	size_t endX = rect.x + rect.width;

	if (startX < 0) {
		startX = 0;
	} else if (endX >= frame.getWidth()) {
		endX = frame.getWidth() - 1;
	}
	for (size_t y = rect.y; y < rect.y + rect.height; y++) {
		for (size_t x = startX; x < endX; x++) {
			if (frame.getPointValue(x, y) == WHITE_PIXEL) {
				average += x;
				found++;
				//std::cout << x << " ";
			}
		}
	}
	if (found == 0) {
		return (0);
	}
	return (average / found);
}

int SlidingWindow::getLaneX(std::vector<int> &histogram) {
	auto it = std::max_element(histogram.begin(), histogram.end());
	int laneX = std::distance(histogram.begin(), it);
	int distance = 50;
	size_t start = laneX - distance;
	size_t end = laneX + distance;

	if (start < 0) {
		start = 0;
	}
	if (end >= histogram.size()) {
		end = histogram.size() - 1;
	}
	std::cout << "LaneX: " << laneX << std::endl;
	for (size_t i = start; i < end; i++) {
		histogram[i] = 0;
	}
	return (laneX);
}

