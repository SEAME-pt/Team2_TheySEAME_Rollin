#pragma once

#include "ActuatorDecorator.hpp"
#include "KuksaLib.hpp"

class ActuatorKuksa : public ActuatorDecorator {
public:

	ActuatorKuksa(IActuator *car);
	~ActuatorKuksa();

	virtual void setThrottle(const int throttle);
	virtual void setSteering(const int steering);
	virtual void setGear(const short gear);

private:

	kuksaLib _kuksa;
};
