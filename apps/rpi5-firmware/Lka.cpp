#include "Lka.hpp"

Lka::Lka(const int fov, const int startX, const int startY, const int width, const int height) : 
	_bev(Bev(fov, cv::Rect(startX, startY, width, height))) {}

Lka::~Lka() {}

void Lka::setAngle(int angle) {
	_angle = angle;
}

int Lka::getAngle() { return (_angle); }

cv::Point Lka::searchLanes(cv::Mat &frame, uint row, int dir) {
	cv::Size size = frame.size();
	int x = size.width / 2;

	while (x > 0 && x < size.width) {
		uchar pixel = frame.at<uchar>(row, x);
		//printf("(%d, %d) = %d\n", row, x, pixel);
		if (pixel == 255) {
			//std::cout << "Found Point" << std::endl;
			break;
		}
		x += dir;
	}
	return (cv::Point(x, row));
}

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
}

void Lka::laneCenter(Frame &frame) {
	int frameH;
	int frameW;
	int angle;

	_bev.applyBevToFrame(frame);

	frameH = frame.getHeight();
	frameW = frame.getWidth();

	cv::Mat tmp = frame.getRawFrame();
	cv::Point carOrigin(frameW / 2, frameH - 50);
	int topRow = (frameH / 2) - 50;
	cv::Point topLeft = searchLanes(tmp, topRow, LEFT);
	cv::Point topRight = searchLanes(tmp, topRow, RIGHT);

	int bottomRow = (frameH / 2) + 50;
	cv::Point bottomLeft = searchLanes(tmp, bottomRow, LEFT);
	cv::Point bottomRight = searchLanes(tmp, bottomRow, RIGHT);

	std::cout << "Left: " << topLeft << " " << bottomLeft << "\n";
	std::cout << "Right: " << topRight << " " << bottomRight << "\n";

	cv::Point laneCenter((topLeft.x + topRight.x) / 2, (topLeft.y + topRight.y) / 2);
	float distX = laneCenter.x - carOrigin.x;
	float distY = carOrigin.y - laneCenter.y;

	std::cout << "Car: " << carOrigin << " " << "Lane: " << laneCenter << std::endl;
	std::cout << "Distances: " << distY << " " << distX << std::endl;
	angle = atan(distX / distY) * (180 / M_PI);
	std::cout << "Angle: " << angle << std::endl;
	_angle = std::clamp(angle, -30, 30);
	cv::cvtColor(frame.getRawFrame(), frame.getRawFrame(), cv::COLOR_GRAY2BGR);
	frame.drawLine(laneCenter, carOrigin, RED, 10);
	frame.drawLine(topLeft, bottomLeft, RED, 10);
	frame.drawLine(topRight, bottomRight, RED, 10);
	frame.drawLine(topRight, topLeft, RED, 10);
	cv::imwrite("./LinesFrame.jpg", frame.getRawFrame());
	notify(Events::CAR_STEERING);
}
