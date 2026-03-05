#pragma once

#include "ICar.hpp"

class CarDecorator : public ICar {
public:

	CarDecorator(ICar *car);
	~CarDecorator();
	
	virtual void setThrottle(const int throttle);
	virtual void setSteering(const int steering);
	virtual void setGear(const short gear);
	virtual void brake();
	virtual short getGear() const;

private:

	ICar *_car;
};
