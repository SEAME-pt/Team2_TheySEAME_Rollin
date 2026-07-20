#pragma once
#include <QObject>
#include <QString>
#include <QDebug>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath> 
#include <grpcpp/grpcpp.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <google/protobuf/stubs/common.h>
#include <chrono>
#include <memory>
#include "val.grpc.pb.h"
#include "types.pb.h"
#include "KuksaLib.hpp"
#include "MqttHazardClient.hpp"

 
using kuksa::val::v2::VAL;

class systemInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int battery READ getBattery NOTIFY batteryUpdated)
    Q_PROPERTY(int speed READ getSpeed NOTIFY speedUpdated)
    Q_PROPERTY(bool cruiseActive READ getCruiseActive NOTIFY cruiseActiveUpdated)
    Q_PROPERTY(int targetSpeed READ getTargetSpeed NOTIFY targetSpeedUpdated)
    Q_PROPERTY(QString targetSpeedDisplay READ getTargetSpeedDisplay NOTIFY targetSpeedUpdated)
    Q_PROPERTY(bool liveDetectionActive READ getLiveDetectionActive NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(bool frontCarVisible READ getFrontCarVisible NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(bool leftCarVisible READ getLeftCarVisible NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(bool rightCarVisible READ getRightCarVisible NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(int frontCarDistance READ getFrontCarDistance NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(int leftCarDistance READ getLeftCarDistance NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(int rightCarDistance READ getRightCarDistance NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(bool ldwWarningActive READ getLdwWarningActive NOTIFY adasWarningUpdated)
    Q_PROPERTY(bool bsdWarningActive READ getBsdWarningActive NOTIFY adasWarningUpdated)
    Q_PROPERTY(bool adasWarningVisible READ getAdasWarningVisible NOTIFY adasWarningUpdated)
    Q_PROPERTY(QString adasWarningMessage READ getAdasWarningMessage NOTIFY adasWarningUpdated)
    Q_PROPERTY(bool adasWarningTimed READ getAdasWarningTimed NOTIFY adasWarningUpdated)
    Q_PROPERTY(int adasWarningRemainingMs READ getAdasWarningRemainingMs NOTIFY adasWarningUpdated)
    Q_PROPERTY(int adasWarningEpoch READ getAdasWarningEpoch NOTIFY adasWarningUpdated)

public:
    /**
     * @brief Constructs the systemInfo class.
     * @param parent Optional parent QObject
     *
     * Requirement traceability:
     *
     */
    explicit systemInfo(QObject *parent = nullptr);

    /**
     * @brief Destructs the systemInfo class, stops the data collection thread.
     *
     */
    ~systemInfo();
    /**
     * @brief Initializes the data collection thread and starts listening for updates from Kuksa.
     *
     * Requirement traceability:
     *
     * @return true if started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Converts a Kuksa value to integer.
     * @param v Kuksa value
     * @param out Output integer
     *
     * Requirement traceability:
     *
     * @return true if conversion is successful
     */
    bool valueToInt(const kuksa::val::v2::Value& v, int& out);

    /**
     * @brief Sets the battery value.
     * @param battery Battery value
     *
     * Requirement traceability:
     *
     */
    void setBattery(int battery);

    /**
     * @brief Returns the current battery value.
     *
     * Requirement traceability:
     *
     * @return int battery value
     */
    int getBattery() const;

    /**
     * @brief Sets the speed value.
     * @param speed Speed value
     *
     * Requirement traceability:
     *
     */
    void setSpeed(int speed);

    /**
     * @brief Returns the current speed value.
     *
     * Requirement traceability:
     *
     * @return int speed value
     */
    int getSpeed() const;

    /**
     * @brief Sets the cruise control active state.
     * @param active true if cruise control is engaged
     *
     * Requirement traceability:
     *
     */
    void setCruiseActive(bool active);

    /**
     * @brief Returns whether cruise control is currently active.
     *
     * Requirement traceability:
     *
     * @return bool cruise control state
     */
    bool getCruiseActive() const;

    /**
     * @brief Sets the cruise control target speed.
     * @param speed Target speed in hm/h
     */
    void setTargetSpeed(int speed);

    /**
     * @brief Returns the cruise control target speed.
     *
     * @return int target speed in hm/h
     */
    int getTargetSpeed() const;

    /**
     * @brief Returns the cruise control target speed display text (formatted).
     * Returns "---" if inactive, or "XXX hm/h" if active.
     *
     * @return QString formatted target speed display
     */
    QString getTargetSpeedDisplay() const;

    bool getLiveDetectionActive() const;
    bool getFrontCarVisible() const;
    bool getLeftCarVisible() const;
    bool getRightCarVisible() const;
    int getFrontCarDistance() const;
    int getLeftCarDistance() const;
    int getRightCarDistance() const;

    bool getLdwWarningActive() const;
    bool getBsdWarningActive() const;
    bool getAdasWarningVisible() const;
    QString getAdasWarningMessage() const;
    bool getAdasWarningTimed() const;
    int getAdasWarningRemainingMs() const;
    int getAdasWarningEpoch() const;

signals:
    void speedUpdated(int speed);
    void batteryUpdated(int battery);
    void cruiseActiveUpdated(bool active);
    void targetSpeedUpdated(int speed);
    void trafficSignUpdated(int sign);
    void speedLimitUpdated(int speedLimit);
    void vehicleDetectionUpdated();
    void adasWarningUpdated();
    void adasWarningTriggered(const QString &message);
    void adasWarningCleared();
private:
    static int metersToDisplayPercent(float meters);
    void updateVehicleDetection();
    void updateAdasWarnings();
    void startMobilityMqtt();
    void handleMobilityMqttMessage(const std::string &topic, const std::string &body);

    std::atomic<bool> _liveDetectionActive{false};
    std::atomic<bool> _frontCarVisible{false};
    std::atomic<bool> _leftCarVisible{false};
    std::atomic<bool> _rightCarVisible{false};
    std::atomic<int> _frontCarDistance{0};
    std::atomic<int> _leftCarDistance{0};
    std::atomic<int> _rightCarDistance{0};
    std::atomic<int> _battery{0};
    std::atomic<int> _speed{0};
    std::atomic<bool> _cruiseActive{false};
    std::atomic<int> _targetSpeed{0};
    bool _ldwWarningActive{false};
    bool _bsdWarningActive{false};
    bool _adasWarningVisible{false};
    QString _adasWarningMessage;
    bool _adasWarningTimed{false};
    int _adasWarningRemainingMs{0};
    int _adasWarningEpoch{0};
    bool _mobilityHazardActive{false};
    QString _mobilityHazardMessage;
    std::chrono::steady_clock::time_point _mobilityHazardUntil{};
    // Pending MQTT hazard (crash location); shown only when ego marker is close.
    bool _pendingHazardValid{false};
    int _pendingHazardMarkerId{-1};
    QString _pendingHazardMessage;
    std::chrono::steady_clock::time_point _pendingHazardExpires{};
    bool _wasNearHazard{false};
    static constexpr int kMobilityHazardDurationMs = 12000;
    static constexpr int kPendingHazardTtlMs = 10 * 60 * 1000; // remember crash for 10 min
    static constexpr int kDefaultMarkerProximity = 1; // same or adjacent ArUco marker
    kuksaLib _kuksa;
    std::unique_ptr<MqttHazardClient> _mqttClient;
    std::thread _thread;
    std::atomic_bool _running{false};
    std::mutex _kuksaMutex;
    std::mutex _mobilityMutex;
};
