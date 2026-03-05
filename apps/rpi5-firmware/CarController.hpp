#pragma once

#include "Observer.hpp"
#include "ICar.hpp"
#include "RemoteControl.hpp"

class CarController : public Observer {
public:

	CarController(ICar *_car, RemoteControl &remote);
	~CarController();

	void update(Events event);

private:

	int processThrottle(const int rawThrottle);
	int processSteering(const int rawSteering);
	ICar *_car;
	RemoteControl &_subject;
};
