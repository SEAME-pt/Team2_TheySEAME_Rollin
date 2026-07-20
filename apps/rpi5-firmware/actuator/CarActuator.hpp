#pragma once

/// Gear Values
enum GearState {
	PARKING,
	NEUTRAL,
	REVERSE,
	DRIVE
};

/**
 * @interface CarActuator
 * @brief CarActuator interface
 *
 * The CarActuator interface defines the contract to create a middleware 
 * between software and hardware in a Car
 */
class CarActuator {
public:

	virtual ~CarActuator() {};

	/**
	 * @brief Set Car Throttle
	 *
	 * Sets the Car throttle
	 *
	 * @param throttle throttle percentage
	 */
	virtual void setThrottle(const int throttle) = 0;

	/**
	 * @brief Set Car Steering Angle
	 *
	 * Sets the Car Steering
	 *
	 * @param steering steering angle
	 */
	virtual void setSteering(const int steering) = 0;

	/**
	 * @brief Set Car Gear
	 *
	 * Sets the Car Gear. Possible Gear values in @ref Gear
	 *
	 * @param gear gear to set
	 */
	virtual void setGear(const short gear) = 0;

	/**
	 * @brief Car Brake
	 *
	 * Send a msg to brake the Car
	 */
	virtual void brake(const bool flag) = 0;

	virtual void setCruiseControl(const bool flag, const int targetSpeed) = 0;

	virtual void setAEb_Enabled(const bool autonomous) = 0;
};
