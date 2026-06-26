#pragma once
#include <cstdint>

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
	NO_OVERTAKING = 15,
    CAR = 16,
    OBJECT = 17
};

struct FrameHeader {
	uint32_t frameNbr;
	uint16_t heigth;
	uint16_t width;
	float laneScore;
};

struct TsrHeader {
    uint32_t	frameNbr;
    uint16_t	numDetections;
    uint16_t    trafficSign;
    float		accuracy;
    uint32_t    x;
    uint32_t    y;
    uint32_t    width;
    uint32_t    height;
};

enum class ModelTrafficSignClass : uint16_t {
    SPEED_LIMIT_50 = 0,
    SPEED_LIMIT_80 = 1,
    GATE = 2,
    CROSSWALK = 3,
    STOP = 4,
    YIELD = 5,
    CAR_FRONT = 6,
    CAR_BACK = 7,
    CAR_LEFT = 8,
    CAR_RIGHT = 9,
    DANGER_SIGN = 10,
    OBSTACLE = 11,
    TRAFFIC_LIGHT_GREEN = 12,
    TRAFFIC_LIGHT_OFF = 13,
    TRAFFIC_LIGHT_RED = 14,
    TRAFFIC_LIGHT_YELLOW = 15,
    CAR = 16
};

inline bool mapModelClassToSpeedLimit(uint16_t classId, int &speedLimit)
{
    
    switch (classId) {
        case static_cast<uint16_t>(ModelTrafficSignClass::SPEED_LIMIT_50):
            speedLimit = 50;
            return true;
        case static_cast<uint16_t>(ModelTrafficSignClass::SPEED_LIMIT_80):
            speedLimit = 80;
            return true;
        default:
            return false;
    }
}

inline TrafficSign mapModelClassToTrafficSign(uint16_t classId)
{
    switch (classId) {
        case static_cast<uint16_t>(ModelTrafficSignClass::STOP):
            return TrafficSign::STOP;
        case static_cast<uint16_t>(ModelTrafficSignClass::CROSSWALK):
            return TrafficSign::PEDESTRIAN;
        case static_cast<uint16_t>(ModelTrafficSignClass::YIELD):
            return TrafficSign::YIELD;
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_GREEN):
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_OFF):
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_RED):
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_YELLOW):
            return TrafficSign::TRAFFIC_LIGHT;
        case static_cast<uint16_t>(ModelTrafficSignClass::CAR_FRONT):
        case static_cast<uint16_t>(ModelTrafficSignClass::CAR_BACK):
        case static_cast<uint16_t>(ModelTrafficSignClass::CAR_LEFT):
        case static_cast<uint16_t>(ModelTrafficSignClass::CAR_RIGHT):
        case static_cast<uint16_t>(ModelTrafficSignClass::CAR):
            return TrafficSign::CAR;
        case static_cast<uint16_t>(ModelTrafficSignClass::OBSTACLE):
            return TrafficSign::OBJECT;
        default:
            return TrafficSign::UNKNOWN;
    }
}