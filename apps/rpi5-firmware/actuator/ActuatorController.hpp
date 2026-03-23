#pragma once

#include "Observer.hpp"
#include "CarActuator.hpp"
#include "RemoteControl.hpp"
#include "KuksaLib.hpp"

class ActuatorController : public Observer {
public:

	ActuatorController(CarActuator *_car, RemoteControl &remote, kuksaLib &kuksa);
	~ActuatorController();

	void update(Events event);

private:

	int processThrottle(const int rawThrottle);
	int processSteering(const int rawSteering);
	CarActuator *_car;
	RemoteControl &_subject;
	kuksaLib &_kuksa;
};
