#pragma once

#include "ActuatorController.hpp"
#include "CAN.hpp"
#include "KuksaLib.hpp"

/**
* @class Bridge
* @brief Bridge class
*
* The Bridge class initializes and setups C++ classes used by the LKA and
* then deletes them when the program finishes
* This class breaks the need to make a python binding for every class
* needed by the LKA
*/
class Bridge {
public:

	/**
	 * @brief Bridge constructor
	 *
	 * Creates all classes needed by the LKA
	 * Then setups the ActuatorController to receive events from the LkaControl class
	 */
	Bridge(LkaControl *lkaCtrl);

	/**
	 * @brief Bridge destructor
	 *
	 * Deletes all classes created in the constructor
	 */
	~Bridge();

private:

	ActuatorCAN *_car;
	ActuatorController *_acc;
	CAN *_can;
	kuksaLib *_kuksa;
};
