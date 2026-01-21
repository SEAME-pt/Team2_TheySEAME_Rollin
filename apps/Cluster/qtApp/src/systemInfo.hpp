#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
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
    explicit systemInfo(QObject *parent = nullptr);
    bool start();
    bool valueToInt(const kuksa::val::v2::Value& v, int& out);
    void setBattery(int battery);;
    int getBattery() const;
    void setSpeed(int speed);;
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