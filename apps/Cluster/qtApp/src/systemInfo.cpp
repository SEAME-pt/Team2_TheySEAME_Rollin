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
    if (!device) {
        qWarning() << "CAN not available";
        return false;
    }
    if (!device->connectDevice()) {
        qWarning() << "Failed to connect CAN device";
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
 *        [impl->dsn~design-requirement-cluster-speed~1]
 *        [impl->dsn~design-requirement-cluster-battery~1]
 *========================================================================
 */
void systemInfo::processFrames()
{
    while (device && device->framesAvailable()) {
        QCanBusFrame frame = device->readFrame();
        qint64 id = frame.frameId();
        QByteArray data = frame.payload();
        if (id == 66) {
            speed = static_cast<unsigned char>(data[0]);
            qDebug() << "Speed updated to:" << speed;
            emit speedUpdated();
        }

        if (id == 77) {
            battery = static_cast<unsigned char>(data[0]);
            qDebug() << "Battery SOC updated to:" << battery;
            emit batteryUpdated();
        }
    }
}
