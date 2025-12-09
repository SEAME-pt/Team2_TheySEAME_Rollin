#include "status.hpp"

status::status(QObject *parent)
    : batteryLevel(0)
{
}

float status::getBattery() const
{
    return batteryLevel;
}
void status::updateBatteryLevel()
{
    float newLevel =

    if (batteryLevel != newLevel) {
        batteryLevel = newLevel;
        emit batteryChanged();
    }
}

