#include "ActuatorKuksa.hpp"

ActuatorKuksa::ActuatorKuksa(CarActuator *car, kuksaLib &kuksa) : ActuatorDecorator(car), _kuksa(kuksa) {}

ActuatorKuksa::~ActuatorKuksa() {}

void ActuatorKuksa::setThrottle(const int throttle) {
	ActuatorDecorator::setThrottle(throttle);
	_kuksa.sendValueToKuksa("Vehicle.Control.Throttle.Value", (float)abs(throttle));
}

void ActuatorKuksa::setSteering(const int steering) {
	ActuatorDecorator::setSteering(steering);
	_kuksa.sendValueToKuksa("Vehicle.Control.Steering.Angle", (float)steering);
}

void ActuatorKuksa::setGear(const short gear) {
	ActuatorDecorator::setGear(gear);
	_kuksa.sendValueToKuksa("Vehicle.Control.Gear.Value", (uint8_t)gear);
}

void ActuatorKuksa::setCruiseControl(const bool flag, const int targetSpeed) {
	ActuatorDecorator::setCruiseControl(flag, targetSpeed);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.CruiseControl.Enabled", (bool)flag);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.CruiseControl.TargetSpeed", (float)targetSpeed);
}

void ActuatorKuksa::setTrafficSign(const int trafficSign, const float distance) {
	ActuatorDecorator::setTrafficSign(trafficSign, distance);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSignType", static_cast<uint8_t>(trafficSign));
	_kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSignDistance", distance);
}

void ActuatorKuksa::setSpeedLimit(const int speedLimit) {
	ActuatorDecorator::setSpeedLimit(speedLimit);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.TrafficSignRecognition.DetectedSpeedLimit", static_cast<float>(speedLimit));
}

void ActuatorKuksa::setAccLeadVehicleDistance(const float distanceMeters) {
	ActuatorDecorator::setAccLeadVehicleDistance(distanceMeters);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleDistance", distanceMeters);
}

void ActuatorKuksa::setAccLeadVehicleOrientation(const float yawDegrees) {
	ActuatorDecorator::setAccLeadVehicleOrientation(yawDegrees);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleOrientation", yawDegrees);
}

void ActuatorKuksa::setBsdLeftOccupied(const bool occupied) {
	ActuatorDecorator::setBsdLeftOccupied(occupied);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.BlindSpotDetection.LeftOccupied", occupied);
}

void ActuatorKuksa::setBsdRightOccupied(const bool occupied) {
	ActuatorDecorator::setBsdRightOccupied(occupied);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.BlindSpotDetection.RightOccupied", occupied);
}

void ActuatorKuksa::setBsdLeftDistance(const float distanceMeters) {
	ActuatorDecorator::setBsdLeftDistance(distanceMeters);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.BlindSpotDetection.LeftDistance", distanceMeters);
}

void ActuatorKuksa::setBsdRightDistance(const float distanceMeters) {
	ActuatorDecorator::setBsdRightDistance(distanceMeters);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.BlindSpotDetection.RightDistance", distanceMeters);
}

void ActuatorKuksa::setBsdLeftVehicleOrientation(const float yawDegrees) {
	ActuatorDecorator::setBsdLeftVehicleOrientation(yawDegrees);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.BlindSpotDetection.LeftVehicleOrientation", yawDegrees);
}

void ActuatorKuksa::setBsdRightVehicleOrientation(const float yawDegrees) {
	ActuatorDecorator::setBsdRightVehicleOrientation(yawDegrees);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.BlindSpotDetection.RightVehicleOrientation", yawDegrees);
}

void ActuatorKuksa::setAEb_Enabled(const bool autonomous) {
	ActuatorDecorator::setAEb_Enabled(autonomous);
	_kuksa.sendValueToKuksa("Vehicle.ADAS.AutomaticEmergencyBraking.Enabled", (bool)autonomous);
}
