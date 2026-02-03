#pragma once

#include "ICar.hpp"
#include "ICAN.hpp"

enum Signals {
	START = 0,
	STOP = 1
};

class Car : public ICar {
public:

	Car(ICAN &can);
	~Car();

	void startNstop(const bool signal);
	int control();
	void setThrottle(const int throttle);
	void setSteering(const int steering);
	void setGear(const short gear);
	int getThrottle() const;
	int getSteering() const;
	short getGear() const;

private:
	ICAN &_can;
	int _throttle;
	int _steering;
	short _gear;
};
