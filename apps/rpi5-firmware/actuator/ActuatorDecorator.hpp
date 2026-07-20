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
	virtual void setAccLeadVehicleDistance(const float distanceMeters);
	virtual void setAccLeadVehicleOrientation(const float yawDegrees);
	virtual void setBsdLeftOccupied(const bool occupied);
	virtual void setBsdRightOccupied(const bool occupied);
	virtual void setBsdLeftDistance(const float distanceMeters);
	virtual void setBsdRightDistance(const float distanceMeters);
	virtual void setBsdLeftVehicleOrientation(const float yawDegrees);
	virtual void setBsdRightVehicleOrientation(const float yawDegrees);
	virtual void setAEb_Enabled(const bool autonomous);
private:

	CarActuator *_car;
};
