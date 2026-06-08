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
	void setSpeedLimit(const int speedLimit);
	void setTrafficSign(const int trafficSign, const float distance);
	void test();

private:

	int processThrottle(const int rawThrottle);
	int processSteering(const int rawSteering);
	void steering(const int angle);
	void throttle(const int throttle);
	void gear(const short gear);
	void cruiseControl(const bool flag, const int targetSpeed);
	void brake(const bool flag);
	void setAEb_Enabled(bool enabled);

	CarActuator *_car;
	RemoteControl *_remote;
	Lka *_lka;
	kuksaLib &_kuksa;

	int _activeSpeedLimit = 0;
	static constexpr float SPEED_REDUCTION_FACTOR = 0.75f;

	static constexpr int STOP_SIGN = 1;
	static constexpr float STOP_BRAKE_DISTANCE_M = 10000.0f;
};
