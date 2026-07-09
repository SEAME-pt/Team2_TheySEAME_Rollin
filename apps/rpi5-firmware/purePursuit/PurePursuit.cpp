#include "PurePursuit.hpp"

PurePursuit::PurePursuit() : _kCte(10.0f), _stallFrames(1), _angleToll(15.0f), _offset(-4), _cteAlpha(0.3f), _camOffset(10.0f), _frameTime(60) { 
	_prevAngle = 0;
	_angle.push(0);
	_showAngle = 0;
	_timeLapse = 0;

	_lastlw = 250;
}

PurePursuit::~PurePursuit() {}

int PurePursuit::getAngle() { return (_angle.front()); }

float passToDegree(float radians) {
	return ((radians * 180) / M_PI);
}

void PurePursuit::debug(float angle, float diff, float cteOff, int lw, float heading) {
	std::cout 
		<< "\tCteOffset: " << cteOff << "\n"
		<< "\tHeadingError: " << passToDegree(heading) << "\n"
		<< "\tAngleDiff: " << diff << "\n"
		<< "\tFinalAngle: " << angle << std::endl;
}

struct Debug PurePursuit::control(float x1, float x2, float lftA1, float lftB1, float rghtA1, float rghtB1) {
	float stangle;
	float cteNorm;
	float ctePi;
	int lw;
	float angle;
	float diff;
	float heading;

	std::cout << "Debug:" << std::endl;
	std::cout << "\tLeftA1: " << lftA1 << " LeftB1: " << lftB1 << std::endl;
	std::cout << "\tRightA1: " << rghtA1 << " RightB1: " << rghtB1 << std::endl;

	lw = x2 - x1;
	ctePi = ((x2 + x1) / 2) - ((float)frameW / 2) + _camOffset;
	cteNorm = ctePi / ((float)lw / 2);
	cteNorm = cteNorm * _cteAlpha + (1 - _cteAlpha) * _prevCteNorm;
	_prevCteNorm = cteNorm;

	_lastlw = lw;

	heading = (atan2(lftA1, -1 * lftB1) + atan2(rghtA1, -1 * rghtB1)) / 2;
	stangle = heading + atan((_kCte * cteNorm) / 15);
	angle = passToDegree(stangle);
	diff = abs(angle - _prevAngle);
	if (diff <= _angleToll || _timeLapse > _frameTime) {
		_angle.push((int)angle + _offset);
		_prevAngle = angle;
		_timeLapse = 0;
	} else {
		_timeLapse += 1;
		std::cout << "Not today" << std::endl;
	}
	debug(angle, diff, cteNorm, lw, heading);
	if (_angle.size() >= _stallFrames) {
		notify(Events::CAR_STEERING);
		_showAngle = _angle.front() - _offset;
		_angle.pop();
	}
	return (Debug{_showAngle, cteNorm, heading});
}

