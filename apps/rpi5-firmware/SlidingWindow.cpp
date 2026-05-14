#include "SlidingWindow.hpp"
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

extern Frame show;

SlidingWindow::SlidingWindow() {}

SlidingWindow::~SlidingWindow() {}

void SlidingWindow::getLanePtns(Frame &frame, const int nbrPtns, std::vector<cv::Point> &leftLanePtns, std::vector<cv::Point> &rightLanePtns) {
	std::vector<int> histogram;
	int rectW = frame.getWidth() / 7;
	int laneX1, laneX2;

	frame.histogram(histogram);
	cv::Point pt1(show.getWidth() / 2, 0);
	cv::Point pt2(show.getWidth() / 2, show.getHeight());
	cv::Point pt3(0, show.getHeight() / 2);
	cv::Point pt4(show.getWidth(), show.getHeight() / 2);
	show.drawLine(pt1, pt2, RED, 3);
	show.drawLine(pt3, pt4, RED, 3);
	laneX1 = getLaneX(histogram, histogram.begin(), histogram.end() - (frame.getWidth() / 2));
	laneX2 = getLaneX(histogram, histogram.begin() + (frame.getWidth() / 2), histogram.end());
	std::cout << "LanesX: " << laneX1 << " " << laneX2 << std::endl;
	slidingWindow(frame, laneX1, nbrPtns, rectW, leftLanePtns);
	slidingWindow(frame, laneX2, nbrPtns, rectW, rightLanePtns);
	// Show Lane Window
	//show.showInScreen("WIN");
}

void SlidingWindow::slidingWindow(Frame &frame, int startX, int ptnsNbr, int rectW, std::vector<cv::Point> &ptns) {
	int x = startX;
	int step_y = frame.getHeight() / ptnsNbr;
	int y = frame.getHeight() - step_y;

	for (size_t i = 0; i < ptnsNbr; i++) {
		cv::Rect rect(x - (rectW / 2), y, rectW, step_y);
		int average = checkPixelsInRect(frame, rect);
		if (average != 0) {
			x = average;
		}
		cv::rectangle(show.getMatObj(), rect, GREEN, 1);
		ptns.push_back(cv::Point(x, y + (step_y / 2)));
		y -= step_y;
	}
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
			}
		}
	}
	if (found == 0) {
		return (0);
	}
	return (average / found);
}

int SlidingWindow::getLaneX(std::vector<int> &histogram, std::vector<int>::iterator hstart, std::vector<int>::iterator hend) {
	auto it = std::max_element(hstart, hend);
	int laneX = std::distance(histogram.begin(), it);
	int distance = 1;
	size_t start = laneX - distance;
	size_t end = laneX + distance;

	if (start < 0) {
		start = 0;
	}
	if (end >= histogram.size()) {
		end = histogram.size() - 1;
	}
	for (size_t i = start; i < end; i++) {
		histogram[i] = 0;
	}
	return (laneX);
}

