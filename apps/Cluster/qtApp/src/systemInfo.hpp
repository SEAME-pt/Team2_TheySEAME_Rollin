#pragma once
#include <QObject>
#include <QCanBus>
#include <QCanBusDevice>

class systemInfo : public QObject
{
    Q_OBJECT

public:
    explicit systemInfo(QObject *parent = nullptr);
    bool start(const QString &interfaceName = "can0");

signals:
    void speedUpdated(int speed);
    void batteryUpdated(int soc);

private slots:
    void processFrames();

private:
    QCanBusDevice *device = nullptr;
};
    