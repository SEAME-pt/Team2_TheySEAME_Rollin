#include "Lka.hpp"

Lka::Lka() : _bev(Bev(400, 1536, 864)) {}

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

void Lka::laneCenter(Frame &frame) {
	int frameH;
	int frameW;
	int angle;

	_bev.applyBevToFrame(frame);

	frameH = frame.getHeight();
	frameW = frame.getWidth();

	cv::Mat tmp = frame.getRawFrame();
	cv::Point carOrigin(frameW / 2, frameH - 50);
	int topRow = frameH - 125;
	cv::Point topLeft = searchLanes(tmp, topRow, LEFT);
	cv::Point topRight = searchLanes(tmp, topRow, RIGHT);

	int bottomRow = frameH + 125;
	cv::Point bottomLeft = searchLanes(tmp, bottomRow, LEFT);
	cv::Point bottomRight = searchLanes(tmp, bottomRow, RIGHT);

	cv::Point laneCenter((topLeft.x + topRight.x) / 2, (topLeft.y + topRight.y) / 2);
	float distX = laneCenter.x - carOrigin.x;
	float distY = carOrigin.y - laneCenter.y;

	std::cout << "Car: " << carOrigin << " " << "Lane: " << laneCenter << std::endl;
	std::cout << "Distances: " << distY << " " << distX << std::endl;
	angle = atan(distX / distY) * (180 / M_PI);
	std::cout << "Angle: " << angle << std::endl;
	_angle = std::clamp(angle, -30, 30);
	notify(Events::CAR_STEERING);
}
