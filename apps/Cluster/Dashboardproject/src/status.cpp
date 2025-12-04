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
    // Logic to update battery level
    float newLevel = /* code to read battery level */ 0.0f; // Placeholder

    if (batteryLevel != newLevel) {
        batteryLevel = newLevel;
        emit batteryChanged();
    }
}

