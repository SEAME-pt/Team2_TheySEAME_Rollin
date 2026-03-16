#pragma once

#include "CarActuator.hpp"

class ActuatorDecorator : public CarActuator {
public:

	ActuatorDecorator(CarActuator *car);
	~ActuatorDecorator();
	
	virtual void setThrottle(const int throttle);
	virtual void setSteering(const int steering);
	virtual void setGear(const short gear);
	virtual void brake();
	virtual short getGear() const;
	virtual void setCruiseControl(const bool flag, const int targetSpeed);

private:

	CarActuator *_car;
};
