#include "PurePursuit.hpp"

PurePursuit::PurePursuit() : _alpha(1.0f), _L(200.0f), _kCte(32.0f), _stallFrames(8), _angleToll(10), _offset(-4), _kTreshold(15), _kStep(0.01), _kAlpha(1.0f), _cteAlpha(0.4) { 
	_prevAngle = 0;
	_angle.push(0);
	_showAngle = 0;

	_badleftK = false;
	_badrightK = false;

	_badleftKI = 0;
	_badrightKI = 0;

	_prevrightK = 0;
	_prevleftK = 0;

	_prevK = 0;
	_lastlw = 250;
}

PurePursuit::~PurePursuit() {}

int PurePursuit::getAngle() { return (_angle.front()); }

void PurePursuit::debug(float leftK, float rightK, float fbangle, float ffangle, float angle, int diff, float cteOff) {
	std::cout 
		<< "\tALeftK: " << leftK << " " << "ARightK: " << rightK << "\n"
		<< "\tAngleCurve: " << ffangle << " " << "AngleCTE: " << fbangle << "\n"
		<< "\tFinalAngle: " << angle << "\n"
		<< "\tIsBadCurve: " << _badleftK << " " << _badrightK << "\n"
		<< "\tCteOffset: " << cteOff << std::endl;
}

float PurePursuit::calcAngle(float k) {
	float angle;

	angle = atan(_L * k);
	return ((angle * 180) / M_PI);
}

void PurePursuit::validateCurves(float leftK, float rightK) {
	float leftdiff = abs(leftK - _prevleftK);
	float rightdiff = abs(rightK - _prevrightK);

	if (leftdiff > _kStep || (_prevleftK < 0 && leftK > 0) || (_prevleftK > 0 && leftK < 0)) {
		_badleftKI++;
		if (_badleftKI < _kTreshold) {
			_badleftK = true;
		} else {
			_badleftKI = 0;
			_badleftK = false;
		}
	} else {
		_badleftKI = 0;
		_badleftK = false;
	}

	if (rightdiff > _kStep || (_prevrightK < 0 && rightK > 0) || (_prevrightK > 0 && rightK < 0)) {
		_badrightKI++;
		if (_badrightKI < _kTreshold) {
			_badrightK = true;
		} else {
			_badrightKI = 0;
			_badrightK = false;
		}
	} else {
		_badrightKI = 0;
		_badrightK = false;
	}
}

float PurePursuit::calcCurve(float leftK, float rightK) {
	float leftAngle = calcAngle(leftK);
	float rightAngle = calcAngle(rightK);
	return ((leftAngle));
}

struct Debug PurePursuit::control(float leftK, float rightK, float x1, float x2) {
	float ffangle;
	float fbangle;
	float cteNorm;
	float ctePi;
	float k;
	int lw;
	float angle;
	int diff;
	const float step = 0.005f;

	std::cout << "Debug:" << std::endl;
	std::cout << "\tLeftK: " << leftK << " RightK: " << rightK << "\n"
			  << "\tPreviousK: " << _prevleftK << " " << _prevrightK << std::endl;

	validateCurves(leftK, rightK);

	if (_badleftK == true) {
		leftK = _prevleftK;
	}
	if (_badrightK == true) {
		rightK = _prevrightK;
	}
	k = calcCurve(leftK, rightK);
	k = k * _kAlpha + (1 - _kAlpha) * _prevK;
	_prevK = k;
	_prevleftK = leftK;
	_prevrightK = rightK;

	lw = x2 - x1;
	ctePi = ((x2 + x1) / 2) - (float)frameW / 2;
	cteNorm = ctePi / (float)lw;
	cteNorm = cteNorm * _cteAlpha + (1 - _cteAlpha) *_prevCteNorm;
	_prevCteNorm = cteNorm;

	_lastlw = lw;
	ffangle = calcCurve(leftK, rightK);
	fbangle = cteNorm * _kCte;

	//angle = ffangle + fbangle;
	angle = fbangle;
	angle = angle * _alpha + (1 - _alpha) * _prevAngle;
	diff = abs(angle - _prevAngle);
	if (diff < _angleToll) {
		_angle.push((int)angle + _offset);
		_prevAngle = angle;
	}
	debug(leftK, rightK, fbangle, ffangle, angle, diff, ctePi / lw);
	if (_angle.size() >= _stallFrames) {
		notify(Events::CAR_STEERING);
		_showAngle = _angle.front() - _offset;
		_angle.pop();
	}
	//notify(Events::CAR_THROTTLE);
	return (Debug{_showAngle, cteNorm, lw});
}

