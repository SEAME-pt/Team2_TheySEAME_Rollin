#include "Tsr.hpp"
#include <algorithm>
#include <iostream>

namespace {

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
    DANGER = 10,
    OBSTACLE = 11,
    TRAFFIC_LIGHT_GREEN = 12,
    TRAFFIC_LIGHT_OFF = 13,
    TRAFFIC_LIGHT_RED = 14,
    TRAFFIC_LIGHT_YELLOW = 15,
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

enum class Lane { LEFT, CENTER, RIGHT };

bool isVehicleClass(uint16_t classId)
{
    return classId == static_cast<uint16_t>(ModelTrafficSignClass::CAR_FRONT)
        || classId == static_cast<uint16_t>(ModelTrafficSignClass::CAR_BACK)
        || classId == static_cast<uint16_t>(ModelTrafficSignClass::CAR_LEFT)
        || classId == static_cast<uint16_t>(ModelTrafficSignClass::CAR_RIGHT)
        || classId == static_cast<uint16_t>(ModelTrafficSignClass::OBSTACLE);
}

float clamp(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

float normalizeYaw(float yaw)
{
    while (yaw > 180.f)
        yaw -= 360.f;
    while (yaw < -180.f)
        yaw += 360.f;
    return yaw;
}

float yawFromBBox(uint16_t classId, float centerX, float width, float height, float frameWidth)
{
    constexpr float kAspectEndOn = 1.0f;
    constexpr float kAspectSideOn = 2.2f;
    const float aspect = width / std::max(height, 1.f);
    const float sideFactor = clamp((aspect - kAspectEndOn) / (kAspectSideOn - kAspectEndOn), 0.f, 1.f);

    // Only car_front is treated as oncoming; side classes are usually 3/4 views ahead.
    const float base = (classId == static_cast<uint16_t>(ModelTrafficSignClass::CAR_FRONT)) ? 0.f : 180.f;
    const float lateral = clamp((centerX - frameWidth * 0.5f) / (frameWidth * 0.5f), -1.f, 1.f);
    return normalizeYaw(base + lateral * sideFactor * 90.f);
}

bool isSignClass(uint16_t classId)
{
    if (isVehicleClass(classId))
        return false;
    int speedLimit = 0;
    if (mapModelClassToSpeedLimit(classId, speedLimit))
        return true;
    return mapModelClassToTrafficSign(classId) != TrafficSign::UNKNOWN;
}

Lane classifyLane(float centerX, float frameWidth)
{
    const float leftBound = frameWidth * 0.35f;
    const float rightBound = frameWidth * 0.65f;
    if (centerX < leftBound)
        return Lane::LEFT;
    if (centerX > rightBound)
        return Lane::RIGHT;
    return Lane::CENTER;
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

// Valores iniciais para FX/FY (podem ser recalibrados em tempo de execução)
float FX_PX = 606.34f; // CAM_STANDARD default
float FY_PX = 1081.08f; // CAM_STANDARD default

Tsr::~Tsr()
{
}

const TsrHeader& Tsr::getLastDetection() {
    return _lastDetection;
}

void Tsr::publishVehicleState(float frontMeters, float leftMeters, float rightMeters,
                              float frontYaw, float leftYaw, float rightYaw)
{
    _car->setAccLeadVehicleDistance(frontMeters >= 0.f ? frontMeters : 0.f);
    _car->setAccLeadVehicleOrientation(frontMeters >= 0.f ? frontYaw : 180.f);
    _car->setBsdLeftOccupied(leftMeters >= 0.f);
    _car->setBsdRightOccupied(rightMeters >= 0.f);
    _car->setBsdLeftDistance(leftMeters >= 0.f ? leftMeters : 0.f);
    _car->setBsdRightDistance(rightMeters >= 0.f ? rightMeters : 0.f);
    _car->setBsdLeftVehicleOrientation(leftMeters >= 0.f ? leftYaw : 180.f);
    _car->setBsdRightVehicleOrientation(rightMeters >= 0.f ? rightYaw : 180.f);
}

void Tsr::handleFrame(const std::vector<TsrHeader> &detections)
{
    if (detections.empty())
        return;

    _lastSignalTime = std::chrono::steady_clock::now();
    _hasSignal = true;

    constexpr float FRAME_W = 640.f;
    float frontDistM = -1.f;
    float leftDistM = -1.f;
    float rightDistM = -1.f;
    float frontYaw = 180.f;
    float leftYaw = 180.f;
    float rightYaw = 180.f;

    TsrHeader bestSign{};
    bool hasSign = false;
    float bestSignAcc = 0.f;

    for (const auto &d : detections) {
        if (isVehicleClass(d.trafficSign)) {
            const float distM = estimateVehicleDistance(d);
            if (distM < 0.f)
                continue;

            const float centerX = static_cast<float>(d.x) + static_cast<float>(d.width) * 0.5f;
            const Lane lane = classifyLane(centerX, FRAME_W);
            const float yaw = yawFromBBox(
                d.trafficSign, centerX,
                static_cast<float>(d.width), static_cast<float>(d.height), FRAME_W);

            if (lane == Lane::CENTER) {
                if (frontDistM < 0.f || distM < frontDistM) {
                    frontDistM = distM;
                    frontYaw = yaw;
                }
            } else if (lane == Lane::LEFT) {
                if (leftDistM < 0.f || distM < leftDistM) {
                    leftDistM = distM;
                    leftYaw = yaw;
                }
            } else if (rightDistM < 0.f || distM < rightDistM) {
                rightDistM = distM;
                rightYaw = yaw;
            }
            continue;
        }

        if (!isSignClass(d.trafficSign))
            continue;

        if (!hasSign || d.accuracy > bestSignAcc) {
            bestSign = d;
            bestSignAcc = d.accuracy;
            hasSign = true;
        }
    }

    publishVehicleState(frontDistM, leftDistM, rightDistM, frontYaw, leftYaw, rightYaw);

    if (hasSign)
        handleTrafficSign(bestSign);
}

void Tsr::handleTrafficSign(const TsrHeader &tsrData)
{
    int speedLimit = 0;
    
    _lastSignalTime = std::chrono::steady_clock::now();
    _hasSignal      = true;

    _lastDetection = tsrData;
    estimateDistance(tsrData);
    if (mapModelClassToSpeedLimit(tsrData.trafficSign, speedLimit)) {
        _car->setSpeedLimit(speedLimit);
        return;
    }
    _car->setTrafficSign(
        static_cast<int>(mapModelClassToTrafficSign(tsrData.trafficSign)),
        _distance.back().second
    );
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

void Tsr::resetKuksa()
{
    _car->setTrafficSign(static_cast<int>(TrafficSign::UNKNOWN), 0.0f);
    publishVehicleState(-1.f, -1.f, -1.f, 180.f, 180.f, 180.f);
    std::cout << "kuksa reset: speed limit 0, traffic sign UNKNOWN, vehicles cleared" << std::endl;
}

float Tsr::estimateVehicleDistance(const TsrHeader& det)
{
    if (det.width < 8 || det.height < 8)
        return -1.0f;

    const float bboxPx = static_cast<float>(std::max(det.width, det.height));
    return lookupDistance(bboxPx);
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

    _distance.push_back({signKey, smoothed / 100.0f});
    return smoothed / 100.0f;
}

void Tsr::applyScaleCalibration(float measured_dist, float true_dist_cm)
{
    if (measured_dist <= 0.0f)
        return;

    const float scale = true_dist_cm / measured_dist;
    FX_PX *= scale;
    FY_PX *= scale;
}

float Tsr::lookupDistance(float bboxPx) {
    if (bboxPx >= DIST_LUT[0].first)
        return DIST_LUT[0].second;
    if (bboxPx <= DIST_LUT.back().first)
        return DIST_LUT.back().second;

    for (size_t i = 0; i + 1 < DIST_LUT.size(); ++i) {
        const auto &a = DIST_LUT[i];
        const auto &b = DIST_LUT[i + 1];
        if (bboxPx <= a.first && bboxPx >= b.first) {
            const float t = (bboxPx - a.first) / (b.first - a.first);
            return a.second + t * (b.second - a.second);
        }
    }
    return -1.0f;
}

Tsr::Tsr(CarActuator *car) : _car(car)
{
}
