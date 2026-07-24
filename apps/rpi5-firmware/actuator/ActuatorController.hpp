#pragma once

#include "Observer.hpp"
#include "CarActuator.hpp"
#include "RemoteControl.hpp"
#include "LkaControl.hpp"
#include "KuksaLib.hpp"
#include "Tsr.hpp"

#include "CAN.hpp"
#include "ActuatorCAN.hpp"

class ActuatorController : public Observer {
public:

	//ActuatorController(CarActuator *car, RemoteControl *remote, LkaControl *lka);
	ActuatorController(CarActuator *_car, RemoteControl *remote, LkaControl *lkaCtrl, kuksaLib &kuksa, Tsr *tsr);
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
	void setAEb_Enabled(bool enabled);


	void trafficSign();
	void speedLimit();
	
	CarActuator *_car;
	RemoteControl *_remote;
	LkaControl *_pp;
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
	int _lastCCSpeed = 30;
	bool _CCActive = false;
	bool _aebEnabled = true;
	static const int STOP_BRAKE_FRAMES = 60;
	static const int STOP_COOLDOWN_FRAMES = 90;
};
