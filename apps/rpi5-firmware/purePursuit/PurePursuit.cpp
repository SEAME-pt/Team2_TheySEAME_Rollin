#include "PurePursuit.hpp"

PurePursuit::PurePursuit() : _alpha(0.4f), _L(60.0f), _angleTol(10) { 
	_prevAngle = 0;
	_angle = 0;
}

PurePursuit::~PurePursuit() {}

int PurePursuit::getAngle() { return (_angle); }

float PurePursuit::calcAngle(const cv::Point2f carPos, cv::Point2f lookahead) {
	float ld = cv::norm(carPos - lookahead);
	float k;
	float angle;

	//std::cout << "LookAhead: " << lookahead << " Car: " << carPos << std::endl;
	//std::cout << "X Error: " << lookahead.x - carPos.x << std::endl;
	//std::cout << "lookaheadDist: " << ld << std::endl;
	k = (2 * (lookahead.x - carPos.x)) / (ld * ld);
	std::cout << "Curvature: " << k << std::endl;
	angle = atan(_L * k);
	return ((angle * 180) / M_PI);
}

int PurePursuit::control(float x1, float y1, float x2, float y2) {
	cv::Point2f carPos(frameW / 2.0f, frameH);
	cv::Point2f lftPtn(x1, y1);
	cv::Point2f rghPtn(x2, y2);
	cv::Point2f lookahead;
	int angle;
	int diff;

	lookahead = (cv::Point2f)(lftPtn + rghPtn) / 2.0f;
	//std::cout << "LPoint: " << lftPtn << std::endl;
	//std::cout << "RPoint: " << rghPtn << std::endl;
	//std::cout << "LookAhead: " << lookahead << std::endl;
	angle = calcAngle(carPos, lookahead);
	//std::cout << "Before Angle: " << angle << std::endl;
	_angle = angle * _alpha + (1 - _alpha) * _prevAngle;
	//diff = std::abs(_angle - _prevAngle);
	//if (diff >= _angleTol) {
	//	std::cout << "Tolerance excedded: " << _angle << std::endl;
	//	_angle = _prevAngle;
	//}
	_prevAngle = _angle;
	//notify(Events::CAR_STEERING);
	return (_angle);
}

