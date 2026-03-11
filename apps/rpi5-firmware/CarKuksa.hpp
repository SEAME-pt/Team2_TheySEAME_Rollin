#pragma once

#include "RemoteControl.hpp"
#include "ICar.hpp"
#include "Observer.hpp"
#include "KuksaLib.hpp"

/**
 * @class CarKuksa
 * @brief CarKuksa class
 *
 * The CarKuksa encapsulates the Kuksa communication between software and hardware
 * It implements the Observer and ICar interface
 */
class CarKuksa : public ICar, public Observer {
public:

	CarKuksa(RemoteControl &remote);
	~CarKuksa();

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
	RemoteControl &_subject;
	kuksaLib _kuksa;
	short _gear;
};
