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
#include "val.grpc.pb.h"
#include "types.pb.h"
#include "KuksaLib.hpp"

 
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
    Q_PROPERTY(double frontCarOrientation READ getFrontCarOrientation NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(double leftCarOrientation READ getLeftCarOrientation NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(double rightCarOrientation READ getRightCarOrientation NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(double frontCarLateral READ getFrontCarLateral NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(double leftCarLateral READ getLeftCarLateral NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(double rightCarLateral READ getRightCarLateral NOTIFY vehicleDetectionUpdated)
    Q_PROPERTY(bool ldwWarningActive READ getLdwWarningActive NOTIFY adasWarningUpdated)
    Q_PROPERTY(bool bsdWarningActive READ getBsdWarningActive NOTIFY adasWarningUpdated)
    Q_PROPERTY(bool adasWarningVisible READ getAdasWarningVisible NOTIFY adasWarningUpdated)
    Q_PROPERTY(QString adasWarningMessage READ getAdasWarningMessage NOTIFY adasWarningUpdated)

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
    double getFrontCarOrientation() const;
    double getLeftCarOrientation() const;
    double getRightCarOrientation() const;
    double getFrontCarLateral() const;
    double getLeftCarLateral() const;
    double getRightCarLateral() const;

    bool getLdwWarningActive() const;
    bool getBsdWarningActive() const;
    bool getAdasWarningVisible() const;
    QString getAdasWarningMessage() const;

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

    std::atomic<bool> _liveDetectionActive{false};
    std::atomic<bool> _frontCarVisible{false};
    std::atomic<bool> _leftCarVisible{false};
    std::atomic<bool> _rightCarVisible{false};
    std::atomic<int> _frontCarDistance{0};
    std::atomic<int> _leftCarDistance{0};
    std::atomic<int> _rightCarDistance{0};
    std::atomic<double> _frontCarOrientation{180.0};
    std::atomic<double> _leftCarOrientation{180.0};
    std::atomic<double> _rightCarOrientation{180.0};
    std::atomic<double> _frontCarLateral{0.0};
    std::atomic<double> _leftCarLateral{0.0};
    std::atomic<double> _rightCarLateral{0.0};
    std::atomic<int> _battery{0};
    std::atomic<int> _speed{0};
    std::atomic<bool> _cruiseActive{false};
    std::atomic<int> _targetSpeed{0};
    bool _ldwWarningActive{false};
    bool _bsdWarningActive{false};
    bool _adasWarningVisible{false};
    QString _adasWarningMessage;
    kuksaLib _kuksa;
    std::thread _thread;
    std::atomic_bool _running{false};
    std::mutex _kuksaMutex;
};
