#include "Lka.hpp"

Lka::Lka(const int fov, const int startX, const int startY, const int width, const int height) : 
	_bev(Bev(fov, cv::Rect(startX, startY, width, height))) {}

Lka::~Lka() {}

void Lka::setAngle(int angle) {
	_angle = angle;
}

int Lka::getAngle() { return (_angle); }

void Lka::poly(Frame &frame) {
	std::vector<int> histogram;
	std::vector<cv::Point> lanePtns1;
	std::vector<cv::Point> lanePtns2;
	int ptsNbr = 8;
	int rectW = 100;
	int laneX1, laneX2;

	_bev.applyBevToFrame(frame);
	laneX1 = _bev.getLaneX();
	laneX2 = _bev.getLaneX();
	std::cout << "Lanes: " << laneX1 << " " << laneX2 << std::endl;
	_bev.slidingWindow(frame, laneX1, ptsNbr, rectW, lanePtns1);
	_bev.slidingWindow(frame, laneX2, ptsNbr, rectW, lanePtns2);

	cv::Mat color;
	cv::cvtColor(frame.getRawFrame(), color, cv::COLOR_GRAY2BGR);
	cv::Point lastPoint;
	cv::Point carPos(frame.getWidth() / 2, frame.getHeight());
	int angle = 0;
	std::cout << "CarPos: " << carPos << std::endl;
	for (size_t i = 0; i < ptsNbr; i++) {
		cv::Point midDist = (lanePtns1[i] + lanePtns2[i]) / 2;
		if (i % 2 ) {
			cv::line(color, lastPoint, midDist, RED, 5);
		}
		lastPoint = midDist;
		//std::cout << "MidLane: " << midDist << std::endl;
		float distX = midDist.x - carPos.x;
		float distY = carPos.y - midDist.y;
		angle += atan(distX / distY) * (180 / M_PI);
	}
	_angle = angle / ptsNbr;
	std::cout << "Angle: " << _angle << std::endl;
	_angle = std::clamp(_angle, -30, 30);
	cv::line(color, carPos, carPos, GREEN, 10);
	cv::imwrite("./Dir.jpg", color);
	notify(Events::CAR_STEERING);
	cv::imshow("Win", color);
}

