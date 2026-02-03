#pragma once

class ICar {
public:

	virtual ~ICar() {};

	virtual void startNstop(const bool signal) = 0;
	virtual int control() = 0;
	virtual void setThrottle(const int throttle) = 0;
	virtual void setSteering(const int steering) = 0;
	virtual void setGear(const short gear) = 0;
	virtual int getSteering() const = 0;
	virtual int getThrottle() const = 0;
	virtual short getGear() const = 0;
};
