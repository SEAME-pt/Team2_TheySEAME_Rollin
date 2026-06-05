#pragma once

#include "CarActuator.hpp"

class ActuatorDecorator : public CarActuator {
public:

	ActuatorDecorator(CarActuator *car);
	~ActuatorDecorator();
	
	virtual void setThrottle(const int throttle);
	virtual void setSteering(const int steering);
	virtual void setGear(const short gear);
	virtual void brake(const bool brake);
	virtual void setCruiseControl(const bool flag, const int targetSpeed);
	virtual void setTrafficSign(const int trafficSign, const float distance);
	virtual void setSpeedLimit(const int speedLimit);
	virtual void setAEb_Enabled(const bool autonomous);
private:

	CarActuator *_car;
	int _currentThrottle = 0;
	int _activeSpeedLimit = 0;
	
	// Traffic sign constants
	static constexpr int STOP_SIGN = 1;
	static constexpr float STOP_BRAKE_DISTANCE_M = 10.0f;
	static constexpr float SPEED_REDUCTION_FACTOR = 0.75f;  // 25% reduction
};

