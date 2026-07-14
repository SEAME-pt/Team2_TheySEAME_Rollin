#pragma once

#include "ActuatorController.hpp"
#include "CAN.hpp"
#include "KuksaLib.hpp"

class Bridge {
public:

	Bridge(LkaControl *lkaCtrl);
	~Bridge();

private:

	ActuatorCAN *_car;
	ActuatorController *_acc;
	CAN *_can;
	kuksaLib *_kuksa;
};
