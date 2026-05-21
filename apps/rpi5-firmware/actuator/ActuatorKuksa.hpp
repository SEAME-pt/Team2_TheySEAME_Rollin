#pragma once

#include "ActuatorDecorator.hpp"
#include "KuksaLib.hpp"

class ActuatorKuksa : public ActuatorDecorator {
public:

	ActuatorKuksa(CarActuator *car, kuksaLib &kuksa);
	~ActuatorKuksa();

	virtual void setThrottle(const int throttle);
	virtual void setSteering(const int steering);
	virtual void setGear(const short gear);
	virtual void setCruiseControl(const bool flag, const int targetSpeed);
	virtual void setTrafficSign(const int trafficSign, const float distance);
	virtual void setSpeedLimit(const int speedLimit);

private:

	kuksaLib &_kuksa;
};
