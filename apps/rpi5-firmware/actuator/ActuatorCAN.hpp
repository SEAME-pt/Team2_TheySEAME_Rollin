#pragma once

#include "CarActuator.hpp"
#include "ICAN.hpp"

/// ID for the different CAN frames
enum CAN_ID {
	THROTTLE = 0x100,
	GEAR = 0x101,
	STEERING = 0x102,
	BRAKE = 0x103,
	DRIVING_MODE = 0x104,
	CRUISE_CONTROL = 0x212,
	TRAFFIC_SIGN = 0x226,
	SPEED_LIMIT = 0x227,
	AEB_ENABLED = 0x20D,
};

/**
 * @class ActuatorCAN
 * @brief ActuatorCAN class
 *
 * The ActuatorCAN encapsulates the CAN communication between software and hardware
 * It implements IActuator interface
 */
class ActuatorCAN : public CarActuator {
public:

	ActuatorCAN(ICAN &can);
	~ActuatorCAN();

	void startNstop(const bool signal);
	void setThrottle(const int throttle);
	void setSteering(const int steering);
	void setGear(const short gear);
	void brake(const bool flag);
	void setCruiseControl(const bool flag, const int targetSpeed);
	void setTrafficSign(const int trafficSign, const float distance);
	void setSpeedLimit(const int speedLimit);
	void setAccLeadVehicleDistance(const float distanceMeters);
	void setAccLeadVehicleOrientation(const float yawDegrees);
	void setBsdLeftOccupied(const bool occupied);
	void setBsdRightOccupied(const bool occupied);
	void setBsdLeftDistance(const float distanceMeters);
	void setBsdRightDistance(const float distanceMeters);
	void setBsdLeftVehicleOrientation(const float yawDegrees);
	void setBsdRightVehicleOrientation(const float yawDegrees);
	void setAEb_Enabled(const bool autonomous);
private:
	ICAN &_can;
};
