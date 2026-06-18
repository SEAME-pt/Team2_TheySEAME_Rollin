#include "PurePursuit.hpp"

PurePursuit::PurePursuit() : _alpha(0.4f), _L(80.0f), _kCte(12.0f), _stallFrames(8), _angleToll(10), _offset(-4), _clw(350) { 
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
	float cteNorm;
	float ctePi;
	int lw;
	int angle;
	int diff;

	lw = x2 - x1;
	if (lw == 0) {
		lw = _clw;
	}
	ctePi = (x2 + x1) / 2 - ((float)frameW / 2);
	cteNorm = ctePi / ((float)lw / 2.0f);
	ffangle = calcAngle((leftK + rightK) / 2);
	fbangle = cteNorm * _kCte;

	angle = ffangle + fbangle;
	angle = angle * _alpha + (1 - _alpha) * _prevAngle;
	diff = abs(angle - _prevAngle);
	if (diff < _angleToll) {
		_angle.push(angle + _offset);
		_prevAngle = angle;
	}
	if (_angle.size() >= _stallFrames) {
		notify(Events::CAR_STEERING);
		_showAngle = _angle.front() - _offset;
		_angle.pop();
	}
	//notify(Events::CAR_THROTTLE);
	return (Debug{_showAngle, (int)(cteNorm * 100), lw});
}

