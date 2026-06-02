#include "PurePursuit.hpp"

PurePursuit::PurePursuit() { 
	_angle = 0;
	_laneWidth = 200;
}

PurePursuit::~PurePursuit() {}

int PurePursuit::getAngle() { return (_angle); }

int PurePursuit::calcAngle(const cv::Point2f carPos, cv::Point2f lookahead) {
	// 2. Transform lookahead into the robot's local frame
	//    x = forward (up in image), y = lateral (left is positive)
	float y = lookahead.x - carPos.x;   // lateral offset
	float x = carPos.y - lookahead.y;   // forward distance

	// 3. Pure pursuit curvature: kappa = 2y / L^2
	//    Equivalent steering angle: alpha = atan2(2y, L^2) — or simply atan2(y, x)
	//    atan2(y, x) is the angle to the target in the robot frame, which is what you want
	float angle = std::atan2(y, x);  // radians

	return static_cast<int>(angle * (180.0f / M_PI));
}

float solveQuadY(struct quadFunc f, const int y) {
	float res = f.a * (y * y) + f.b * y + f.c;
	return (res);
}

void PurePursuit::control(struct quadFunc leftfunc, struct quadFunc rightfunc, const int frameH, const int frameW) {
	cv::Point2f carPos(frameW / 2.0f, frameH);
	cv::Point2f leftLanePtn;
	cv::Point2f rightLanePtn;
	cv::Point2f lookahead;
	float angle;
	float ret;

	ret = solveQuadY(leftfunc, carPos.y - _lookaheadDist);
	leftLanePtn = cv::Point2f{ret, carPos.y - _lookaheadDist};
	ret = solveQuadY(rightfunc, carPos.y - _lookaheadDist);
	rightLanePtn = cv::Point2f{ret, carPos.y - _lookaheadDist};
	lookahead = (cv::Point2f)(leftLanePtn + rightLanePtn) / 2.0f;
	_angle = calcAngle(carPos, lookahead);
	std::cout << "CarPos: " << carPos << std::endl;
	std::cout << "LPoint: " << leftLanePtn << std::endl;
	std::cout << "RPoint: " << rightLanePtn << std::endl;
	std::cout << "Point: " << lookahead << std::endl;
	std::cout << "Angle: " << _angle << std::endl;
	//notify(Events::CAR_STEERING);
}

