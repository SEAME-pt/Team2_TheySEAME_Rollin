#pragma once

/**
 * @interface ICar
 * @brief ICar interface
 *
 * The ICar interface defines the contract to create a middleware 
 * between software and hardware 
 */
class ICar {
public:

	virtual ~ICar() {};

	virtual void startNstop(const bool signal) = 0;
	virtual void setThrottle(const int throttle) = 0;
	virtual void setSteering(const int steering) = 0;
	virtual void setGear(const short gear) = 0;
	virtual void brake() = 0;
	virtual short getGear() const = 0;
};
