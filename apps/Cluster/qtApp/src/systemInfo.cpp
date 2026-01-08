#include "systemInfo.hpp"

systemInfo::systemInfo(QObject *parent)
    : QObject(parent)
{
}

bool CanManager::start(const QString &interfaceName)
{
    device = QCanBus::instance()->createDevice("socketcan", interfaceName);

    if (!device || !device->connectDevice()) {
        qWarning() << "CAN not available";
        return false;
    }

    connect(device, &QCanBusDevice::framesReceived,
            this, &CanManager::processFrames);

    return true;
}

void CanManager::processFrames()
{
    while (device->framesAvailable()) {

        QCanBusFrame frame = device->readFrame();
        quint32 id = frame.frameId();
        QByteArray data = frame.payload();

        if (id == 0x123) {
            int speed = static_cast<unsigned char>(data[0]);
            emit speedUpdated(speed);
        }

        if (id == 0x456) {
            int soc = static_cast<unsigned char>(data[1]);
            emit batteryUpdated(soc);
        }
    }
}
