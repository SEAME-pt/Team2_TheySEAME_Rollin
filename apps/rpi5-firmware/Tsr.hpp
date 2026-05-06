#pragma once

#include "Subject.hpp"
#include "CarActuator.hpp"
#include "KuksaLib.hpp"


enum class TrafficSign : int {
	UNKNOWN = 0,
	STOP = 1,
	SPEED_LIMIT_30 = 2,
	SPEED_LIMIT_50 = 3,
	SPEED_LIMIT_100 = 4,
	SPEED_LIMIT_80 = 5,
	SPEED_LIMIT_120 = 6,
	YIELD = 7,
	NO_ENTRY = 8,
	TURN_LEFT = 9,
	TURN_RIGHT = 10,
	PEDESTRIAN = 11,
	TRAFFIC_LIGHT = 12,
	ONE_WAY = 13,
	NO_PARKING = 14,
	NO_OVERTAKING = 15
};

class Tsr
{
	public:

	/**
	 * @brief Tsr Constructor
	 * Constructor for Tsr class which takes a CarActuator pointer as an argument to control the car based on traffic sign recognition
	 */
    Tsr(CarActuator *car);

	/**
	 * @brief Tsr Destructor
	 * Destructor for Tsr class
	 */
    ~Tsr();
	
	/**
	 * @brief Handle Traffic Sign
	 * This function takes an integer representing a traffic sign and performs the corresponding action on the car using the CarActuator interface. The traffic sign is determined by the integer value, which is cast to the Traffic 
	 */
	void handleTrafficSign(const int trafficSign);

	private:
	
	CarActuator *_car;
};
