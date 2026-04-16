#pragma once

#include "Subject.hpp"

class Lka : public Subject {
public:

	Lka();
	~Lka();

	void setAngle(int angle);
	int getAngle();

private:

	int _angle;
};
