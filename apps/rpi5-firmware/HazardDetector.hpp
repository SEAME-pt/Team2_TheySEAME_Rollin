#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>

// ---------------------------------------------------------------------------
// Forward-declare TsrHeader so this header stays self-contained.
// Replace with your actual include if preferred.
// ---------------------------------------------------------------------------
struct TsrHeader;

// ---------------------------------------------------------------------------
// Hazard types that can be reported upstream (e.g. via MQTT to other Piracers)
// ---------------------------------------------------------------------------
enum class HazardType {
    NONE,
    STOPPED_CAR,        // moving + center-right + short detection time
    TWO_STOPPED_CARS,   // stopped + center-right + long detection time
    OUR_CAR_STOPPED,    // stopped + detected car in left zone
    OBJECT_ON_TRACK,    // static object, center-left zone, stable frames
};

// ---------------------------------------------------------------------------
// Per-track entry: one per unique detection (by class + approximate position)
// ---------------------------------------------------------------------------
struct DetectionTrack {
    uint16_t signClass;
    int      framesDetected = 0;
    bool     inCenterRight  = false;
};

// ---------------------------------------------------------------------------
// Result returned each frame — check hazard != NONE to act / publish
// ---------------------------------------------------------------------------
struct HazardResult {
    HazardType  hazard      = HazardType::NONE;
    std::string description;          // human-readable, useful for logging
    uint16_t    triggerClass = 0;     // which signClass triggered this
};

// ---------------------------------------------------------------------------
// HazardDetector
// ---------------------------------------------------------------------------
class HazardDetector {
public:
    // Tunable parameters
    struct Config {
        float    frameWidth         = 640.0f; // pixels — adjust to your resolution
        float    zoneBoundaryRatio  = 0.45f;  // left of this = "left zone"
        float    minConfidence      = 0.70f;
        int      minStableFrames    = 5;      // object-on-track threshold
        int      shortTimeFrames    = 20;     // below = "short" detection
        int      longTimeFrames     = 60;     // above = "long" detection
        float    stoppedSpeedThresh = 0.05f;  // m/s — below = our car stopped
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
