#include "systemInfo.hpp"

systemInfo::systemInfo(QObject *parent)
    : QObject(parent)
{
    
}

systemInfo::~systemInfo()
{
    _running = false;
    if (_thread.joinable()) {
        _thread.join();
    }
}

void systemInfo::setBattery(int battery)
{
    if (_battery == battery) return;
        _battery = battery;
    emit batteryUpdated(battery);
}

void systemInfo::setSpeed(int speed)
{
    if (_speed == speed) return;
        _speed = speed;
    emit speedUpdated(speed);
}

int systemInfo::getBattery() const
{
    return _battery;
}

int systemInfo::getSpeed() const
{
    return _speed;
}

bool systemInfo::start()
{
    if (_running) return true;
    _running = true;

    _thread = std::thread([this]() {

        std::thread subThread([this]() {
            if (!_kuksa.subscribeFromKuksa()) {
                qWarning() << "Failed to subscribe to Kuksa";
            }
        });

        while (_running) {
            setSpeed(static_cast<int>(_kuksa.getSpeed()));
            setBattery(static_cast<int>(_kuksa.getBattery()));

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        subThread.join();
    });

    return true;
}
