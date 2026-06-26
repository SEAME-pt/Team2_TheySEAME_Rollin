#include "HazardDetector.hpp"

bool HazardDetector::isObjectClass(TrafficSign c)
{
    return c == TrafficSign::OBJECT;
}

bool HazardDetector::isCarClass(TrafficSign c )
{
    return c == TrafficSign::CAR;
}

HazardDetector::HazardDetector()
    : HazardDetector(Config{})
{
}

HazardDetector::HazardDetector(Config cfg)
    : _cfg(std::move(cfg))
{
}

void HazardDetector::setOurSpeed(float speedMetersPerSecond)
{
    _ourSpeed  = speedMetersPerSecond;
    _ourMoving = (_ourSpeed > 0.1f);
}

void HazardDetector::update(const TsrHeader& det)
{
    TrafficSign mappedSign = mapModelClassToTrafficSign(det.trafficSign);
    if (mappedSign == TrafficSign::UNKNOWN)
        return;
    if (det.accuracy < _cfg.minConfidence)
        return;

    if (!isCarClass(mappedSign) && !isObjectClass(mappedSign))
        return;
    auto& track          = _tracks[mappedSign];
    track.signClass      = mappedSign;
    track.framesDetected += 1;
    track.seenThisFrame = true;
}

HazardResult HazardDetector::evaluate()
{
    HazardResult result;

    std::cout << "[HazardDetector] evaluate() — tracks=" << _tracks.size()
               << " ourMoving=" << _ourMoving << std::endl;

    for (auto& [cls, track] : _tracks) {

        std::cout << "[HazardDetector] track class=" << static_cast<uint16_t>(cls)
                   << " framesDetected=" << track.framesDetected << std::endl;

        // static object
        if (isObjectClass(cls)) {
            std::cout << "[HazardDetector] -> classified as OBJECT (minStableFrames="
                       << _cfg.minStableFrames << ")" << std::endl;

            if (track.framesDetected >= _cfg.minStableFrames) {
                result.hazard      = HazardType::OBJECT_ON_TRACK;
                result.triggerClass = cls;
                result.description  = "Static object on track (class="
                    + std::to_string(static_cast<uint16_t>(cls))
                    + ", frames="
                    + std::to_string(track.framesDetected) + ")";

                std::cout << "[HazardDetector] TRIGGER: " << result.description << std::endl;
                return result;
            }
            continue;
        }

        // car
        if (isCarClass(cls)) {
            const bool shortTime = track.framesDetected <= _cfg.shortTimeFrames;
            const bool longTime  = track.framesDetected >  _cfg.longTimeFrames;

            std::cout << "[HazardDetector] -> classified as CAR shortTime=" << shortTime
                       << " longTime=" << longTime
                       << " (shortTimeFrames=" << _cfg.shortTimeFrames
                       << ", longTimeFrames=" << _cfg.longTimeFrames << ")" << std::endl;

            if (_ourMoving) {
                if (shortTime) {
                    result.hazard       = HazardType::STOPPED_CAR;
                    result.triggerClass = cls;
                    result.description  = "Stopped car ahead (frames="
                        + std::to_string(track.framesDetected) + ")";

                    std::cout << "[HazardDetector] TRIGGER: " << result.description << std::endl;
                    return result;
                }
                std::cout << "[HazardDetector] our car moving, not shortTime -> no trigger for this track" << std::endl;
            } else {
                if (longTime) {
                    result.hazard       = HazardType::TWO_STOPPED_CARS;
                    result.triggerClass = cls;
                    result.description  = "Two stopped cars (frames="
                        + std::to_string(track.framesDetected) + ")";

                    std::cout << "[HazardDetector] TRIGGER: " << result.description << std::endl;
                    return result;
                } else {
                    result.hazard       = HazardType::OUR_CAR_STOPPED;
                    result.triggerClass = cls;
                    result.description  = "Our car is stopped (lateral reference detected)";

                    std::cout << "[HazardDetector] TRIGGER: " << result.description << std::endl;
                    return result;
                }
            }
        }
    }

    std::cout << "[HazardDetector] evaluate() — no hazard triggered" << std::endl;
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
