#include "HazardDetector.hpp"
#include "tsr.hpp"

bool HazardDetector::isObjectClass(uint16_t c)
{
    return c == static_cast<uint16_t>(TrafficSign::OBJECT);
}

bool HazardDetector::isCarClass(uint16_t c)
{
    return c == static_cast<uint16_t>(TrafficSign::CAR);
}

HazardDetector::HazardDetector(Config cfg)
    : _cfg(cfg)
{}

void HazardDetector::setOurSpeed(float speedMetersPerSecond)
{
    _ourSpeed  = speedMetersPerSecond;
    _ourMoving = (_ourSpeed > _cfg.stoppedSpeedThresh);
}

void HazardDetector::update(const TsrHeader& det)
{
    if (det.accuracy < _cfg.minConfidence)
        return;

    if (!isCarClass(det.trafficSign) && !isObjectClass(det.trafficSign))
        return;

    auto& track          = _tracks[det.trafficSign];
    track.signClass      = det.trafficSign;
    track.framesDetected += 1;
    track.seenThisFrame = true;
}

HazardResult HazardDetector::evaluate()
{
    HazardResult result;

    for (auto& [cls, track] : _tracks) {

        // static object
        if (isObjectClass(cls)) {
            if (track.framesDetected >= _cfg.minStableFrames) {
                result.hazard      = HazardType::OBJECT_ON_TRACK;
                result.triggerClass = cls;
                result.description  = "Static object on track (class="
                                        + std::to_string(cls)
                                        + ", frames="
                                        + std::to_string(track.framesDetected) + ")";
                return result;
            }
            continue;
        }

        //car
        if (isCarClass(cls)) {
            const bool shortTime = track.framesDetected <= _cfg.shortTimeFrames;
            const bool longTime  = track.framesDetected >  _cfg.longTimeFrames;

            if (_ourMoving) {
                if (shortTime) {
                    result.hazard       = HazardType::STOPPED_CAR;
                    result.triggerClass = cls;
                    result.description  = "Stopped car ahead (frames="
                                            + std::to_string(track.framesDetected) + ")";
                    return result;
                }
            } else {
                if (longTime) {
                    result.hazard       = HazardType::TWO_STOPPED_CARS;
                    result.triggerClass = cls;
                    result.description  = "Two stopped cars (frames="
                                            + std::to_string(track.framesDetected) + ")";
                    return result;
                } else {
                    result.hazard       = HazardType::OUR_CAR_STOPPED;
                    result.triggerClass = cls;
                    result.description  = "Our car is stopped (lateral reference detected)";
                    return result;
                }
            }
        }
    }

    return result;
}

void HazardDetector::endFrame()
{
    for (auto it = _tracks.begin(); it != _tracks.end(); )
    {
        if (!it->second.seenThisFrame)
            it = _tracks.erase(it);
        else
        {
            it->second.seenThisFrame = false;
            ++it;
        }
    }
    _framesSinceReset++;
}
