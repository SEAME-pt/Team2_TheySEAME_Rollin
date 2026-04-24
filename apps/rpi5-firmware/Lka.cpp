#include "Lka.hpp"

Lka::Lka(const int fov, const int startX, const int startY, const int width, const int height) : 
	_bev(Bev(fov, cv::Rect(startX, startY, width, height))) {}

Lka::~Lka() {}

int Lka::getAngle() { return (_angle); }

void Lka::poly(Frame &frame) {
	std::vector<int> histogram;
	std::vector<cv::Point> leftLanePtns;
	std::vector<cv::Point> rightLanePtns;
	int ptsNbr = 8;
	int rectW = 200;
	int laneX1, laneX2;
	cv::Point carPos;

	_bev.applyBevToFrameTD(frame);
	laneX1 = _bev.getLaneX();
	laneX2 = _bev.getLaneX();
	_bev.slidingWindow(frame, laneX1, ptsNbr, rectW, leftLanePtns);
	_bev.slidingWindow(frame, laneX2, ptsNbr, rectW, rightLanePtns);
	if (laneX1 > laneX2) {
		std::swap(leftLanePtns, rightLanePtns);
	}
	std::cout << "Lanes: " << laneX1 << " " << laneX2 << std::endl;
	std::cout << "CarPos: " << carPos << std::endl;

	Frame colorFrame = frame.getColoredFrame();
	cv::Point lastPoint((leftLanePtns[0] + rightLanePtns[0]) / 2);
	cv::Point lastLaneLine1 = leftLanePtns[0];
	cv::Point lastLaneLine2 = rightLanePtns[0];
	carPos = cv::Point(frame.getWidth() / 2, frame.getHeight());
	for (size_t i = 0; i < ptsNbr; i++) {
		cv::Point midDist = (leftLanePtns[i] + rightLanePtns[i]) / 2;
		cv::Point laneLine1 = leftLanePtns[i];
		cv::Point laneLine2 = rightLanePtns[i];
		colorFrame.drawLine(lastLaneLine1, laneLine1, GREEN, 7);
		colorFrame.drawLine(lastLaneLine2, laneLine2, BLUE, 7);
		lastLaneLine1 = laneLine1;
		lastLaneLine2 = laneLine2;
		colorFrame.drawLine(lastPoint, midDist, RED, 10);
		lastPoint = midDist;
		//std::cout << "MidLane: " << midDist << std::endl;
		float distX = midDist.x - carPos.x;
		float distY = carPos.y - midDist.y;
		_angle += atan(distX / distY) * (180 / M_PI);
	}
	_angle = _angle / ptsNbr;
	std::cout << "Angle: " << _angle << std::endl;
	colorFrame.drawLine(leftLanePtns[0], rightLanePtns[0], YELLOW, 7);
	colorFrame.drawLine(leftLanePtns[ptsNbr - 1], rightLanePtns[ptsNbr - 1], GREEN, 7);
	colorFrame.warp(_bev.getReverseMatrix());
	colorFrame.showInScreen("WIN");
	//notify(Events::CAR_STEERING);
}

