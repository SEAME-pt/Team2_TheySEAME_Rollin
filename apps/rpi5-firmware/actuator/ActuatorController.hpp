#pragma once

#include "Observer.hpp"
#include "CarActuator.hpp"
#include "RemoteControl.hpp"
#include "KuksaLib.hpp"
#include "Lka.hpp"

class ActuatorController : public Observer {
public:

	ActuatorController(CarActuator *_car, RemoteControl *remote, Lka *lka, kuksaLib &kuksa);
	~ActuatorController();

	void update(Subject *subj, Events event);
	void test();
	void throttle(const int throttle);

private:

	int processThrottle(const int rawThrottle);
	int processSteering(const int rawSteering);
	void steering(const int angle);
	void gear(const short gear);
	void cruiseControl(const bool flag, const int targetSpeed);
	void brake(const bool flag);

	CarActuator *_car;
	RemoteControl *_remote;
	Lka *_lka;
	kuksaLib &_kuksa;
};
