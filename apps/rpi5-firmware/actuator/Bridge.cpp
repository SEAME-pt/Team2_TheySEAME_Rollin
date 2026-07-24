#include "Bridge.hpp"

Bridge::Bridge(LkaControl *lkaCtrl) {
	_can = new CAN("can0", 500, 0, 0);
	_car = new ActuatorCAN(*_can);
	_kuksa = new kuksaLib;
	_acc = new ActuatorController(_car, NULL, lkaCtrl, *_kuksa, NULL);

	lkaCtrl->attach(_acc);
}

Bridge::~Bridge() {
	delete _acc;
	delete _car;
	delete _can;
	delete _kuksa;
}
