#pragma once
#include <QObject>
#include <QDebug>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath> 
#include <grpcpp/grpcpp.h>
#include <thread>
#include <atomic>
#include "../../../middleware/kuksa/val/v2/val.grpc.pb.h"
#include "../../../middleware/kuksa/val/v2/types.pb.h"


using kuksa::val::v2::VAL;

class systemInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int battery READ getBattery NOTIFY batteryUpdated)
    Q_PROPERTY(int speed READ getSpeed NOTIFY speedUpdated)

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
signals:
    void speedUpdated(int speed);
    void batteryUpdated(int battery);

public slots:
    bool updateFromKuksa();

private:
    QString _server = "0.0.0.0:55555";
    int _battery = -0;
    int _speed = 0;
    std::thread _thread;
    std::atomic_bool _running{false};
};
