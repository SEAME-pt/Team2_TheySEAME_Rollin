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

    auto& track          = _tracks[mappedSign];
    track.signClass      = mappedSign;
    track.framesDetected += 1;
    track.seenThisFrame = true;
    track.marker_id     = det.marker_id;
    
    if (isCarClass(mappedSign)) {
        track.framesSinceLastDetection = 0;
    } else {
        track.framesSinceLastDetection++;
    }
    return ;
}

HazardResult HazardDetector::evaluate()
{
    HazardResult result;

    for (auto& [cls, track] : _tracks) {
        const bool shortTime = track.framesDetected <= _cfg.shortTimeFrames;
        const bool longTime  = track.framesDetected >  _cfg.longTimeFrames;
        if (cls == TrafficSign::UNKNOWN){
            if (!_ourMoving && longTime) {
                HazardResult result;

                result.hazard      = HazardType::OUR_CAR_STOPPED;
                result.triggerClass = cls;
                result.marker_id    = track.marker_id;
                return result;
            }
            continue;
        }
        // static object
        if (isObjectClass(cls)) {
            if (track.framesDetected >= _cfg.minStableFrames) {
                result.hazard      = HazardType::OBJECT_ON_TRACK;
                result.triggerClass = cls;
                result.marker_id    = track.marker_id;;

                return result;
            }
            continue;
        }

        // car
        if (isCarClass(cls)) {
            if (_ourMoving) {
                const bool isCandidate = track.framesDetected >= _cfg.shortTimeFrames;

                if (isCandidate) {
                    if (track.framesSinceLastDetection == 0) {
                        if (track.framesDetected >= _cfg.shortTimeFrames + _cfg.confirmMoveFrames) {
                            track.framesDetected = 0;
                        }
                    } else if (track.framesSinceLastDetection >= _cfg.lostFrames) {
                        result.hazard        = HazardType::STOPPED_CAR;
                        result.triggerClass  = cls;
                        result.marker_id     = track.marker_id;
                        return result;
                    }
                }
            }
        }
            // else {
            //     if (longTime) {
            //         result.hazard       = HazardType::TWO_STOPPED_CARS;
            //         result.triggerClass = cls;
            //         result.marker_id    = track.marker_id;;

            //         return result;
            //     }
            // }
    }

    return result;
}

void HazardDetector::endFrame()
{
    for (auto it = _tracks.begin(); it != _tracks.end(); )
    {
        auto& track = it->second;

        if (!track.seenThisFrame) {
            track.framesSinceLastDetection++;

            // só apaga depois de ultrapassar o período de graça
            if (track.framesSinceLastDetection > _cfg.lostFrames) {
                it = _tracks.erase(it);
                continue;
            }
        } else {
            track.seenThisFrame = false;
        }
        ++it;
    }
    _framesSinceReset++;
}
