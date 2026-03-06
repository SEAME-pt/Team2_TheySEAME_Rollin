#pragma once

/// Gear Values
enum GearState {
	PARKING,
	NEUTRAL,
	REVERSE,
	DRIVE
};

/**
 * @interface IActuator
 * @brief IActuator interface
 *
 * The IActuator interface defines the contract to create a middleware 
 * between software and hardware 
 */
class IActuator {
public:

	virtual ~IActuator() {};

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
	virtual void brake() = 0;

	/**
	 * @brief Get Gear
	 *
	 * Get the current Gear in the Car
	 */
	virtual short getGear() const = 0;
};
