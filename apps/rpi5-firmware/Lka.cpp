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
		frame.drawLine(leftLane[i - 1], leftLane[i], GREEN, 7);
		frame.drawLine(rightLane[i - 1], rightLane[i], BLUE, 7);
	}
	frame.drawLine(leftLane[0], rightLane[0], YELLOW, 7);
	frame.drawLine(leftLane[_nbrPtns - 1], rightLane[_nbrPtns - 1], GREEN, 7);
	frame.warp(_bev.getReverseMatrix());
	std::string angleStr = "Angle: " + std::to_string(_angle);
	cv::putText(frame.getMatObj(), angleStr, cv::Point(40, 40), cv::FONT_HERSHEY_SIMPLEX, 1, WHITE, 3, cv::LINE_8, false);
	frame.showInScreen("WIN");
	frame.save("./Final.jpg");
}

void Lka::poly(Frame &frame) {
	std::vector<cv::Point> leftLanePtns;
	std::vector<cv::Point> rightLanePtns;
	float angle = 0;
	cv::Point carPos;

	_bev.applyBevToFrame(frame);
	_slwin.getLanePtns(frame, _nbrPtns, leftLanePtns, rightLanePtns);

	Frame colorFrame = frame.getColoredFrame();
	cv::Point lastPoint((leftLanePtns[0] + rightLanePtns[0]) / 2);
	carPos = cv::Point(frame.getWidth() / 2, frame.getHeight());
	//std::cout << "CarPos: " << carPos << std::endl;
	for (size_t i = 0; i < _nbrPtns; i++) {
		cv::Point midDist = (leftLanePtns[i] + rightLanePtns[i]) / 2;
		colorFrame.drawLine(lastPoint, midDist, RED, 10);
		lastPoint = midDist;
		//std::cout << "MidLane: " << midDist << std::endl;
		float distX = midDist.x - carPos.x;
		float distY = carPos.y - midDist.y;
		angle += atan(distX / distY);
		//std::cout << "Angle: " << _angle << std::endl;
	}
	_angle = (angle / _nbrPtns) * (180 / M_PI);
	std::cout << "Angle: " << _angle << std::endl;
	putLinesInScreen(colorFrame, leftLanePtns, rightLanePtns);
	notify(Events::CAR_STEERING);
}

