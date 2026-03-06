#pragma once

#include "Observer.hpp"
#include "IActuator.hpp"
#include "RemoteControl.hpp"

class ActuatorController : public Observer {
public:

	ActuatorController(IActuator *_car, RemoteControl &remote);
	~ActuatorController();

	void update(Events event);

private:

	int processThrottle(const int rawThrottle);
	int processSteering(const int rawSteering);
	IActuator *_car;
	RemoteControl &_subject;
};
