#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include "Tsr.hpp"
#include "Utils.hpp"

struct TsrHeader;


enum class HazardType {
    NONE,
    STOPPED_CAR,
    TWO_STOPPED_CARS,
    OUR_CAR_STOPPED,
    OBJECT_ON_TRACK,
};

struct DetectionTrack {
    TrafficSign signClass;
    int      framesDetected = 0;
    int      framesSinceLastDetection = 0;
    bool     seenThisFrame = false;
    uint32_t marker_id = 0;
};

struct HazardResult {
    HazardType  hazard      = HazardType::NONE;
    uint32_t    marker_id   = 0;
    TrafficSign    triggerClass = TrafficSign::UNKNOWN;
};


class HazardDetector {
public:
    // Tunable parameters
    struct Config {
        float    frameWidth         = 640.0f;
        float    minConfidence      = 0.45;
        int      minStableFrames    = 5;
        int      shortTimeFrames    = 10;
        int      longTimeFrames     = 60;
        int      confirmMoveFrames = 40;
        int      lostFrames = 40;
    };

    explicit HazardDetector(Config cfg);
    HazardDetector();    
    void setOurSpeed(float speedMetersPerSecond);

    void update(const TsrHeader& det);

    HazardResult evaluate();

    void endFrame();

    Config& config() { return _cfg; }

private:
    Config _cfg;
    float  _ourSpeed       = 0.0f;
    bool   _ourMoving      = false;
    int    _framesSinceReset = 0;
    int   _carTrustedFrames = 0;
    std::unordered_map<TrafficSign, DetectionTrack> _tracks;

    static bool isObjectClass(TrafficSign c);

    static bool isCarClass(TrafficSign c);

    bool inCenterRight(uint32_t x, uint32_t width) const;
    bool inCenterLeft (uint32_t x, uint32_t width) const;
};
