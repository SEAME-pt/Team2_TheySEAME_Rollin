#pragma once

#include "RemoteControl.hpp"
#include "ICar.hpp"
#include "ICAN.hpp"
#include "Observer.hpp"

enum Gear {
	PARKING,
	NEUTRAL,
	REVERSE,
	DRIVE
};

enum CAN_ID {
	THROTTLE = 0x100,
	GEAR = 0x101,
	STEERING = 0x102,
	BRAKE = 0x103,
	DRIVING_MODE = 0x104,
};

/**
 * @class Car
 * @brief Car class
 *
 * The Car encapsulates the communication between software and hardware
 * It implements the Observer and ICar interface
 */
class CarCAN : public ICar, public Observer {
public:

	CarCAN(ICAN &can, RemoteControl &remote);
	~CarCAN();

	void startNstop(const bool signal);
	void setThrottle(const int throttle);
	void setSteering(const int steering);
	void setGear(const short gear);
	void brake();
	short getGear() const;

	/**
	 * Implmentation of interface Observer
	 *
	 * \copydoc Observer::update
	 */
	void update(Events event);

private:
	int processThrottle(const int rawThrottle);
	int processSteering(const int rawSteering);
	ICAN &_can;
	RemoteControl &_subject;
	short _gear;
};
