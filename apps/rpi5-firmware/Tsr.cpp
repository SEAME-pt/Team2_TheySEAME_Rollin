#include "Tsr.hpp"
#include <iostream>

namespace {

enum class ModelTrafficSignClass : uint16_t {
    SPEED_LIMIT_50 = 0,
    SPEED_LIMIT_80 = 1,
    GATE = 2,
    CROSSWALK = 3,
    STOP = 4,
    YIELD = 5,
    CAR = 6,
    DANGER = 7,
    OBSTACLE = 8,
    TRAFFIC_LIGHT_GREEN = 9,
    TRAFFIC_LIGHT_OFF = 10,
    TRAFFIC_LIGHT_RED = 11,
    TRAFFIC_LIGHT_YELLOW = 12,
};

bool mapModelClassToSpeedLimit(uint16_t classId, int &speedLimit)
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

TrafficSign mapModelClassToTrafficSign(uint16_t classId)
{
    switch (classId) {
        case static_cast<uint16_t>(ModelTrafficSignClass::STOP):
            return TrafficSign::STOP;
        case static_cast<uint16_t>(ModelTrafficSignClass::CROSSWALK):
            return TrafficSign::PEDESTRIAN;
        case static_cast<uint16_t>(ModelTrafficSignClass::YIELD):
            return TrafficSign::YIELD;
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_GREEN):
            return TrafficSign::TRAFFIC_LIGHT;
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_OFF):
            return TrafficSign::TRAFFIC_LIGHT;
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_RED):
            return TrafficSign::TRAFFIC_LIGHT;
        case static_cast<uint16_t>(ModelTrafficSignClass::TRAFFIC_LIGHT_YELLOW):
            return TrafficSign::TRAFFIC_LIGHT;
        default:
            return TrafficSign::UNKNOWN;
    }
}

}

Tsr::Tsr() {}

// Valores iniciais para FX/FY (podem ser recalibrados em tempo de execução)
float FX_PX = 606.34f; // CAM_STANDARD default
float FY_PX = 1081.08f; // CAM_STANDARD default

Tsr::~Tsr()
{
}

void Tsr::clearDetectedSigns() {
    _detectedSigns.clear();
}

const TsrHeader& Tsr::getLastDetection() {
    return _lastDetection;
}

const std::vector<uint16_t>& Tsr::getDetectedSigns() const {
    return _detectedSigns;
}

int Tsr::getSpeedLimit() const {
    return _speedLimit;
}

bool Tsr::isStopBrakeActive() const {
    return _stopBrakeActive;
}

void Tsr::handleTrafficSign(const TsrHeader &tsrData)
{
    int speedLimit = 0;    
    _lastSignalTime = std::chrono::steady_clock::now();
    _hasSignal      = true;
    _lastDetection = tsrData;

    float distance = estimateDistance(tsrData);
    TrafficSign mappedSign = mapModelClassToTrafficSign(tsrData.trafficSign);
    _detectedSigns.push_back(static_cast<uint16_t>(mappedSign));

    if (mappedSign == TrafficSign::UNKNOWN) {
        // std::cout << "[TSR] Detected unknown sign class " << tsrData.trafficSign << " — ignoring" << std::endl;
    }
    // std::cout << "[TSR] Detected sign: " << static_cast<int>(mappedSign) << " at estimated distance " << distance << " cm" << std::endl;
    notify(Events::CAR_TRAFFIC_SIGN);
    
    if (mapModelClassToSpeedLimit(tsrData.trafficSign, speedLimit)) {
        if (speedLimit == -1)
            speedLimit = _speedLimit;
        _speedLimit = speedLimit;
        // std::cout << "Traffic sign: " << tsrData.trafficSign << " mapped to speed limit: " << _speedLimit << " km/h" << std::endl;
        // std::cout << "[TSR] Updated speed limit to " << _speedLimit << " km/h" << std::endl;
        notify(Events::CAR_SPEED_LIMIT);
        return;
    }
}

void Tsr::tick()
{
    if (!_hasSignal)
        return;

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - _lastSignalTime
    ).count();

    if (elapsed >= TSR_TIMEOUT_MS) {
        std::cout << "[TSR] Timeout (" << elapsed
                  << " ms sem sinal) — reset KUKSA para 0" << std::endl;
        resetKuksa();
        _hasSignal = false;
        _distBuffer.clear();
        _distance.clear();
    }
}

float Tsr::getStopDistance() const {
    for (const auto& [sign, dist] : _distance) {
        if (sign == static_cast<uint16_t>(TrafficSign::STOP)) {
            return dist;
        }
    }
    return -1.0f;
}

void Tsr::resetKuksa()
{
    _speedLimit = 0;
    std::cout << "kuksa reset: speed limit 0, traffic sign UNKNOWN" << std::endl;
}

float Tsr::estimateDistance(const TsrHeader& det)
{
    int speedLimit = 0;
    TrafficSign mapped;

    if (det.width < 8 || det.height < 8)
        return -1.0f;
    mapped = mapModelClassToTrafficSign(det.trafficSign);
    if (static_cast<int>(mapped) == 0)
        mapped = mapModelClassToSpeedLimit(det.trafficSign, speedLimit) ? static_cast<TrafficSign>(det.trafficSign) : TrafficSign::UNKNOWN;
    uint16_t signKey = static_cast<uint16_t>(mapped);

    if (mapped == TrafficSign::UNKNOWN)
        return -1.0f;

    auto it = SIGN_SIZES.find(signKey);
    if (it == SIGN_SIZES.end())
        return -1.0f;

    const SignSize& s = it->second;

    float dist_x = (s.width_cm  * FX_PX) / static_cast<float>(det.width);
    float dist_y = (s.height_cm * FY_PX) / static_cast<float>(det.height);
    float dist = (dist_x + dist_y) * 0.5f;

    _distBuffer.push_back(dist);
    if (_distBuffer.size() > DIST_FILTER_SIZE)
        _distBuffer.pop_front();

    float smoothed = 0.0f;
    for (float d : _distBuffer) smoothed += d;
    smoothed /= static_cast<float>(_distBuffer.size());

    // std::cout << "Estimated distance to sign (raw): " << dist << " cm, smoothed: " << smoothed << " cm"
    //     << "FX_PX: " << FX_PX << "FY_PX: " << FY_PX << "bbox height: " << det.height  << "bbox width: " << det.width << std::endl;
    float corrected = smoothed;
    if (smoothed > SIGN_HEIGHT_OFFSET_CM) {
        corrected = sqrtf(smoothed * smoothed 
                        - SIGN_HEIGHT_OFFSET_CM * SIGN_HEIGHT_OFFSET_CM);
    }

    _distance.push_back(std::make_pair(signKey, corrected));
    return corrected;
}

void Tsr::applyScaleCalibration(float measured_dist, float true_dist_cm)
{
    if (measured_dist <= 0.0f) {
        std::cout << "applyScaleCalibration: measured_dist invalido" << std::endl;
        return;
    }
    float scale = true_dist_cm / measured_dist;
    FX_PX *= scale;
    FY_PX *= scale;
    std::cout << "applyScaleCalibration: escala=" << scale << ", FX_PX=" << FX_PX << ", FY_PX=" << FY_PX << std::endl;
}

float Tsr::lookupDistance(float bboxPx) {
    if (bboxPx >= DIST_LUT.front().first) return DIST_LUT.front().second;
    if (bboxPx <= DIST_LUT.back().first)  return DIST_LUT.back().second;
    
    for (int i = 0; i < DIST_LUT.size() - 1; i++) {
        auto &a = DIST_LUT[i], &b = DIST_LUT[i+1];
        if (bboxPx <= a.first && bboxPx >= b.first) {
            float t = (bboxPx - a.first) / (b.first - a.first);
            return a.second + t * (b.second - a.second);
        }
    }
    return -1.0f;
}