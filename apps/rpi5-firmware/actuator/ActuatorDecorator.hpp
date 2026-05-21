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
private:

	CarActuator *_car;
};
