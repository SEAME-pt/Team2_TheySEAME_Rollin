#pragma once

#include "RemoteControl.hpp"
#include "ICar.hpp"
#include "ICAN.hpp"
#include "Observer.hpp"

enum Signals {
	START = 0,
	STOP = 1
};

class Car : public ICar, public Observer {
public:

	Car(ICAN &can, RemoteControl &remote);
	~Car();

	void startNstop(const bool signal);
	int control();
	void setThrottle(const int throttle);
	void setSteering(const int steering);
	void setGear(const short gear);
	int getThrottle() const;
	int getSteering() const;
	short getGear() const;

	void update(Events event);

private:
	ICAN &_can;
	RemoteControl &_subject;
	int _throttle;
	int _steering;
	short _gear;
};
