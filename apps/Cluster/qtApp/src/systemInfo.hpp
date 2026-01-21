#pragma once
#include <QObject>
#include <QCanBus>
#include <QCanBusDevice>
#include <QDebug>
#include <iostream>

class systemInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int speed READ getSpeed NOTIFY speedUpdated)
    Q_PROPERTY(int battery READ getBattery NOTIFY batteryUpdated)

public:

    /**
    * @brief Constructs the systemInfo helper for CAN telemetry.
    * @param parent Optional QObject parent for ownership.
    *
    */
    explicit systemInfo(QObject *parent = nullptr);

    /**
    * @brief Initializes CAN bus device on specified interface and hooks frame handler.
    * @param interfaceName CAN interface name (default "can0")
    *
	* Requirement traceability:
    * [impl->dsn~design-requirement-cluster-telemetry~1]
    *
    * @return true if device started successfully, false otherwise
    *
    */
    bool start(const QString &interfaceName);

signals:
    void speedUpdated();
    void batteryUpdated();

public slots:
    /**
    * @brief Reads available CAN frames and emits signals for speed and battery SOC updates.
    *
	* Requirement traceability:
    * [impl->dsn~design-requirement-cluster-speed~1] [impl->dsn~design-requirement-cluster-battery~1]
    *
    */
    void processFrames();

private:
    QCanBusDevice *device = nullptr;
    int speed = 0;
    int battery = 0;
};
