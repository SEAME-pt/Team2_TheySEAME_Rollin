#include "Lka.hpp"

Lka::Lka(const int fov, const int startX, const int startY, const int width, const int height, const int nbrPtns) : 
	_bev(Bev(fov, cv::Rect(startX, startY, width, height))) {
	_angle = 0;
	_laneWidth = 200;
	_nbrPtns = nbrPtns;
}

Lka::~Lka() {}

int Lka::getAngle() { return (_angle); }

void Lka::putLinesInScreen(std::vector<cv::Point> &leftLane, std::vector<cv::Point> &rightLane) {
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

double vecLenght(cv::Point2f vect) {
	return (std::sqrt((vect.x * vect.x) + (vect.y * vect.y)));
}

cv::Point2f normalize(const cv::Point2f &vect) {
	cv::Point2f norm;
	float mag;
	
	mag = vecLenght(vect);
	norm.x = vect.x / mag;
	norm.y = vect.y / mag;
	return (norm);
}

int Lka::calcAngle(Frame &frame, const std::vector<cv::Point> &leftLanePtns, const std::vector<cv::Point> &rightLanePtns) {
	float angle = 0;
	float laneSize = 0;
	cv::Point carPos(frame.getWidth() / 2, frame.getHeight());
	const size_t end = _nbrPtns;

	for (size_t i = 0; i < end; i++) {
		cv::Point midDist = (leftLanePtns[i] + rightLanePtns[i]) / 2;
		float distX = midDist.x - carPos.x;
		float distY = carPos.y - midDist.y;
		angle += atan(distX / distY);
	}
	_laneWidth = vecLenght(cv::Point2f(rightLanePtns[0].x - leftLanePtns[0].x, rightLanePtns[0].y - leftLanePtns[0].y));
	return ((angle / end) * (180 / M_PI));
}

void Lka::makeVirtualLane(std::vector<cv::Point> &lanePtns, std::vector<cv::Point> &virtualLanePtns, const int dir) {
	cv::Point2f norm;

	for (size_t i = 0; i + 1 < _nbrPtns; i++) {
		cv::Point vect = lanePtns[i] - lanePtns[i + 1];
		cv::Point2f perpVect(-vect.y, vect.x);
		norm = normalize(perpVect * dir);
		virtualLanePtns[i].x = lanePtns[i].x + norm.x * _laneWidth;
		virtualLanePtns[i].y = lanePtns[i].y + norm.y * _laneWidth;
		std::cout << "Virtual Points: " << virtualLanePtns[i] << std::endl;
	}
	virtualLanePtns[_nbrPtns - 1].x = lanePtns[_nbrPtns - 1].x + norm.x * _laneWidth;
	virtualLanePtns[_nbrPtns - 1].y = lanePtns[_nbrPtns - 1].y + norm.y * _laneWidth;
}

void Lka::poly(Frame &frame) {
	std::vector<cv::Point> leftLanePtns;
	std::vector<cv::Point> rightLanePtns;
	int angle = 0;

	_bev.applyBevToFrame(frame);
	_slwin.getLanePtns(frame, _nbrPtns, leftLanePtns, rightLanePtns);

	if (leftLanePtns[0].x == 0 && rightLanePtns[0].x == frame.getWidth() / 2) {
		leftLanePtns = _lastLeftLane;
		rightLanePtns = _lastRightLane;
		cv::putText(show.getMatObj(), "Lost 2 Lanes", cv::Point(30, 220), cv::FONT_HERSHEY_SIMPLEX, 1, RED, 3, cv::LINE_8, false);
	}

	if (leftLanePtns[0].x == 0 || leftLanePtns[0].x + (_laneWidth / 3) >= 320) {
		makeVirtualLane(rightLanePtns, leftLanePtns, RIGHT);
		cv::putText(show.getMatObj(), "Lost Left Lane", cv::Point(30, 100), cv::FONT_HERSHEY_SIMPLEX, 1, RED, 3, cv::LINE_8, false);
	} else if (rightLanePtns[0].x == frame.getWidth() / 2 || rightLanePtns[0].x - (_laneWidth / 3) <= 320) {
		makeVirtualLane(leftLanePtns, rightLanePtns, LEFT);
		cv::putText(show.getMatObj(), "Lost Right Lane", cv::Point(30, 160), cv::FONT_HERSHEY_SIMPLEX, 1, RED, 3, cv::LINE_8, false);
	}

	angle = calcAngle(frame, leftLanePtns, rightLanePtns);
	_angle = angle;
	putLinesInScreen(leftLanePtns, rightLanePtns);
	notify(Events::CAR_STEERING);
	_lastLeftLane = leftLanePtns;
	_lastRightLane = rightLanePtns;
}

