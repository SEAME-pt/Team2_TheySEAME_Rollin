#pragma once

#include "Bev.hpp"
#include "Subject.hpp"

#define LEFT -1
#define RIGHT 1

class Lka : public Subject {
public:

	Lka();
	~Lka();

	void setAngle(int angle);
	int getAngle();
	void laneCenter(Frame &frame);

private:

	cv::Point searchLanes(cv::Mat &frame, uint row, int dir);

	Bev _bev;
	int _angle;
};
