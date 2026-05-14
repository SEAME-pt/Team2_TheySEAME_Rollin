#include "Lka.hpp"

Lka::Lka(const int fov, const int startX, const int startY, const int width, const int height, const int nbrPtns) : 
	_bev(Bev(fov, cv::Rect(startX, startY, width, height))) {
	_angle = 0;
	_nbrPtns = nbrPtns;
}

Lka::~Lka() {}

int Lka::getAngle() { return (_angle); }

void Lka::putLinesInScreen(Frame &frame, std::vector<cv::Point> &leftLane, std::vector<cv::Point> &rightLane) {
	for (size_t i = 1; i < _nbrPtns; i++) {
		show.drawLine(leftLane[i - 1], leftLane[i], GREEN, 10);
		show.drawLine(rightLane[i - 1], rightLane[i], BLUE, 10);
	}
	//frame.drawLine(leftLane[0], rightLane[0], YELLOW, 7);
	//frame.drawLine(leftLane[_nbrPtns - 1], rightLane[_nbrPtns - 1], GREEN, 7);
	//frame.warp(_bev.getReverseMatrix());
	std::string angleStr = "Angle: " + std::to_string(_angle);
	cv::putText(show.getMatObj(), angleStr, cv::Point(30, 40), cv::FONT_HERSHEY_SIMPLEX, 1, RED, 3, cv::LINE_8, false);
	// Show Lane Lines
	show.showInScreen("WIN");
}

int Lka::calcAngle(Frame &frame, const std::vector<cv::Point> &leftLanePtns, const std::vector<cv::Point> &rightLanePtns) {
	float angle = 0;
	cv::Point lastPoint = (leftLanePtns[0] + rightLanePtns[0]) / 2;
	cv::Point carPos(frame.getWidth() / 2, frame.getHeight());
	const size_t end = _nbrPtns;

	for (size_t i = 0; i < end; i++) {
		cv::Point midDist = (leftLanePtns[i] + rightLanePtns[i]) / 2;
		lastPoint = midDist;
		float distX = midDist.x - carPos.x;
		float distY = carPos.y - midDist.y;
		angle += atan(distX / distY);
	}
	return ((angle / end) * (180 / M_PI));
}

void Lka::poly(Frame &frame) {
	std::vector<cv::Point> leftLanePtns;
	std::vector<cv::Point> rightLanePtns;
	int laneSize;
	int angle = 0;

	_bev.applyBevToFrame(frame);
	_slwin.getLanePtns(frame, _nbrPtns, leftLanePtns, rightLanePtns);

	if (leftLanePtns[0].x == 0 && rightLanePtns[0].x == frame.getWidth() / 2) {
		leftLanePtns = _lastLeftLane;
		rightLanePtns = _lastRightLane;
		cv::putText(show.getMatObj(), "Lost 2 Lanes", cv::Point(30, 220), cv::FONT_HERSHEY_SIMPLEX, 1, RED, 3, cv::LINE_8, false);
	}

	if (leftLanePtns[0].x == 0 || leftLanePtns[0].x + 75 >= 320) {
		for (size_t i = 0; i < _nbrPtns; i++) {
			leftLanePtns[i].x = rightLanePtns[i].x - _laneSize;
			std::cout << "Left Points: " << leftLanePtns[i] << std::endl;
		}
		cv::putText(show.getMatObj(), "Lost Left Lane", cv::Point(30, 100), cv::FONT_HERSHEY_SIMPLEX, 1, RED, 3, cv::LINE_8, false);
	} else if (rightLanePtns[0].x == frame.getWidth() / 2 || rightLanePtns[0].x - 75 <= 320) {
		for (size_t i = 0; i < _nbrPtns; i++) {
			rightLanePtns[i].x = leftLanePtns[i].x + _laneSize;
			std::cout << "RIght Points: " << rightLanePtns[i] << std::endl;
		}
		cv::putText(show.getMatObj(), "Lost Right Lane", cv::Point(30, 160), cv::FONT_HERSHEY_SIMPLEX, 1, RED, 3, cv::LINE_8, false);
	}

	angle = calcAngle(frame, leftLanePtns, rightLanePtns);
	_angle = angle;
	Frame colorFrame = frame.getColoredFrame();
	putLinesInScreen(colorFrame, leftLanePtns, rightLanePtns);
	//notify(Events::CAR_STEERING);
	_lastLeftLane = leftLanePtns;
	_lastRightLane = rightLanePtns;
}

