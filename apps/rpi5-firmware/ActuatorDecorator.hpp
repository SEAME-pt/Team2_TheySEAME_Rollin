#pragma once

#include "IActuator.hpp"

class ActuatorDecorator : public IActuator {
public:

	ActuatorDecorator(IActuator *car);
	~ActuatorDecorator();
	
	virtual void setThrottle(const int throttle);
	virtual void setSteering(const int steering);
	virtual void setGear(const short gear);
	virtual void brake();
	virtual short getGear() const;

private:

	IActuator *_car;
};
