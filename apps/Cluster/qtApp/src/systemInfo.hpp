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
    explicit systemInfo(QObject *parent = nullptr);
    bool start(const QString &interfaceName);
    int getSpeed() const { return speed; }
    int getBattery() const { return battery; }

signals:
    void speedUpdated();
    void batteryUpdated();

public slots:
    void processFrames();

private:
    QCanBusDevice *device = nullptr;
    int speed = 0;
    int battery = 0;
};