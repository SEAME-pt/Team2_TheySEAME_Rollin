#pragma once

#include "Subject.hpp"
#include "CarActuator.hpp"
#include "KuksaLib.hpp"
#include "Utils.hpp"
#include <arpa/inet.h>
#include <unordered_map>

extern float FX_PX;
extern float FY_PX;
static constexpr float SIGN_HEIGHT_OFFSET_CM = 4.5f;
static constexpr int   TSR_TIMEOUT_MS        = 500;
#define FRAME_NMBR 77000

struct SignSize {
    float width_cm;
    float height_cm;
};

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

const std::unordered_map<uint16_t, SignSize> SIGN_SIZES = {
	{static_cast<uint16_t>(TrafficSign::UNKNOWN),        { 0.0f, 0.0f } },
    { static_cast<uint16_t>(TrafficSign::STOP),          { 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::SPEED_LIMIT_30),{ 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::SPEED_LIMIT_50),{ 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::SPEED_LIMIT_80),{ 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::SPEED_LIMIT_100),{ 4.990f, 4.990f }},
    { static_cast<uint16_t>(TrafficSign::SPEED_LIMIT_120),{ 4.990f, 4.990f }},
    { static_cast<uint16_t>(TrafficSign::YIELD),         { 5.445f, 4.920f } },
    { static_cast<uint16_t>(TrafficSign::NO_ENTRY),      { 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::TURN_LEFT),     { 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::TURN_RIGHT),    { 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::PEDESTRIAN),    { 4.980f, 4.980f } },
    { static_cast<uint16_t>(TrafficSign::TRAFFIC_LIGHT), { 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::ONE_WAY),       { 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::NO_PARKING),    { 4.990f, 4.990f } },
    { static_cast<uint16_t>(TrafficSign::NO_OVERTAKING), { 4.990f, 4.990f } },
};

static const std::vector<std::pair<float,float>> DIST_LUT = {
    { 120.0f, 20.0f },
    {  80.0f, 30.0f },
    {  55.0f, 42.0f },
    {  38.0f, 60.0f },
    {  28.0f, 80.0f },
};

class Tsr : public Subject
{
public:
    Tsr();
    ~Tsr();

    void handleTrafficSign(const TsrHeader &tsrData);
    void tick();

    const TsrHeader& getLastDetection();
    const std::vector<uint16_t>& getDetectedSigns() const;
    const std::vector<std::pair<uint16_t, float>>& getDistance() const { return _distance; }

    int getSpeedLimit() const;
    bool isStopBrakeActive() const;
    float estimateDistance(const TsrHeader& det);
    float getStopDistance() const;
    void applyScaleCalibration(float measured_dist, float true_dist_cm);
    float lookupDistance(float bboxPx);
    void resetKuksa();
    void clearDetectedSigns();
private:

    TsrHeader    _lastDetection;

    std::chrono::steady_clock::time_point _lastSignalTime;
    bool _hasSignal = false;
    bool _stopBrakeActive = false;
    int _speedLimit = 80;
    std::vector<uint16_t> _detectedSigns;

    std::vector<std::pair<uint16_t, float>> _distance;
    static constexpr int DIST_FILTER_SIZE = 10;
    std::deque<float> _distBuffer;
};