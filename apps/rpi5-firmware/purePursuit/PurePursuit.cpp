#include "PurePursuit.hpp"

PurePursuit::PurePursuit() : _alpha(0.4f), _L(80.0f), _kCte(12.0f) { 
	_prevAngle = 0;
	_angle.push(0);
	_showAngle = 0;
}

PurePursuit::~PurePursuit() {}

int PurePursuit::getAngle() { return (_angle.front()); }

float PurePursuit::calcAngle(float k) {
	float angle;

	//std::cout << "Curvature: " << k << std::endl;
	angle = atan(_L * k);
	return ((angle * 180) / M_PI);
}

struct Debug PurePursuit::control(float leftK, float rightK, float x1, float x2) {
	float ffangle;
	float fbangle;
	int diff;
	int lw = x2 - x1;
	float ctePi = (x2 + x1) / 2 - ((float)frameW / 2);
	float cteNorm = ctePi / ((float)lw / 2.0f);
	int angle;

	ffangle = calcAngle((leftK + rightK) / 2);
	fbangle = cteNorm * _kCte;
	std::cout << "CTEAngle: " << fbangle << std::endl;

	angle = ffangle + fbangle;
	angle = angle * _alpha + (1 - _alpha) * _prevAngle;
	_angle.push(angle);
	_prevAngle = angle;
	//std::cout << "Angle: " << _angle << std::endl;
	if (_angle.size() >= 15) {
		notify(Events::CAR_STEERING);
		_showAngle = _angle.front();
		_angle.pop();
	}
	notify(Events::CAR_THROTTLE);
	return (Debug{_showAngle, (int)(cteNorm * 100)});
}

