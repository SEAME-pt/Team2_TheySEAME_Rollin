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
	void setTrafficSign(const int trafficSign);
	void setSpeedLimit(const int speedLimit);
private:
	ICAN &_can;
};
