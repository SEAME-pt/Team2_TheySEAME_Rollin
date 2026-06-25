#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>

// ---------------------------------------------------------------------------
// Forward-declare TsrHeader so this header stays self-contained.
// Replace with your actual include if preferred.
// ---------------------------------------------------------------------------
struct TsrHeader;


enum class HazardType {
    NONE,
    STOPPED_CAR,        // moving + short detection time
    TWO_STOPPED_CARS,   // stopped + long detection time
    OUR_CAR_STOPPED,    // stopped
    OBJECT_ON_TRACK,    // static object, center-left zone, stable frames
};

struct DetectionTrack {
    uint16_t signClass;
    int      framesDetected = 0;
};

struct HazardResult {
    HazardType  hazard      = HazardType::NONE;
    std::string description;
    uint16_t    triggerClass = 0;
};


class HazardDetector {
public:
    // Tunable parameters
    struct Config {
        float    frameWidth         = 640.0f;
        float    minConfidence      = 0.70f;
        int      minStableFrames    = 5;
        int      shortTimeFrames    = 20;
        int      longTimeFrames     = 60;
    };

    explicit HazardDetector(Config cfg = {});

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

    std::unordered_map<uint16_t, DetectionTrack> _tracks;

    static bool isObjectClass(uint16_t c);

    static bool isCarClass(uint16_t c);

    bool inCenterRight(uint32_t x, uint32_t width) const;
    bool inCenterLeft (uint32_t x, uint32_t width) const;
};
