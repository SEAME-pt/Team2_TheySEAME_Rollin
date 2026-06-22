#pragma once

#include "Observer.hpp"
#include "CarActuator.hpp"
#include "RemoteControl.hpp"
#include "KuksaLib.hpp"
#include "Lka.hpp"
#include "Tsr.hpp"

class ActuatorController : public Observer {
public:

	ActuatorController(CarActuator *_car, RemoteControl *remote, Lka *lka, kuksaLib &kuksa, Tsr *tsr);
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
	void trafficSign();
	void speedLimit();
	
	CarActuator *_car;
	RemoteControl *_remote;
	Lka *_lka;
	Tsr *_tsr;
	kuksaLib &_kuksa;

	std::mutex _mutex;

	int _currentThrottle = 0;
	int _lastSpeedLimit = 0;
	bool _reduceSpeed = false;
	bool _stopDetected = false;
	int _stopBrakeFrames = 0;
	int _stopCooldownFrames = 0;
	bool _stopCooldown = false;
	static const int STOP_BRAKE_FRAMES = 60;
	static const int STOP_COOLDOWN_FRAMES = 90;
};
