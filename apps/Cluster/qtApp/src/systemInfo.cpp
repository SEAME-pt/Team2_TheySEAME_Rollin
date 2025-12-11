#include "systemInfo.hpp"

systemInfo::systemInfo(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief systemInfo::start
 * Initializes CAN bus device on specified interface.
 * Connects to framesReceived signal to process incoming frames.
 * @param interfaceName CAN interface name (default "can0")
 * @return true if device started successfully, false otherwise
 */
bool systemInfo::start(const QString &interfaceName)
{
    device = QCanBus::instance()->createDevice("socketcan", interfaceName);

    if (!device || !device->connectDevice()) {
        qWarning() << "CAN not available";
        return false;
    }

    connect(device, &QCanBusDevice::framesReceived,
            this, &systemInfo::processFrames);

    return true;
}

/**
 * @brief systemInfo::processFrames
 * Reads available CAN frames and emits signals for speed and battery SOC updates.
 * 
 * =======================Requirements traceability========================
 *        impl->dsg~design-requirement-cluster-speed~1
 *       impl->dsg~design-requirement-cluster-battery~1
 *========================================================================
 */
void systemInfo::processFrames()
{
    while (device->framesAvailable()) {

        QCanBusFrame frame = device->readFrame();
        qint64 id = frame.frameId();
        QByteArray data = frame.payload();

        if (id == 0x123) {
            speed = static_cast<unsigned char>(data[0]);
            std::cout << "Speed: " << speed << " km/h" << std::endl;
            emit speedUpdated();
        }

        if (id == 0x456) {
            battery = static_cast<unsigned char>(data[1]);
            std::cout << "Battery : " << battery << " %" << std::endl;
            emit batteryUpdated();
        }
    }
}
