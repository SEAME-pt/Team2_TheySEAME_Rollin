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
    if (!device->connectDevice()) {
        qWarning() << "Failed to connect CAN device";
        return false;
    }
    if (device){
        connect(device, &QCanBusDevice::framesReceived,
                this, &systemInfo::processFrames);
    }
    else{
        qWarning() << "CAN not available";
        return false;
    }
    std::cout << "CAN device started on interface: " << interfaceName.toStdString() << std::endl;
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
        std::cout << "Frame received" << std::endl;
        QCanBusFrame frame = device->readFrame();
        qint64 id = frame.frameId();
        QByteArray data = frame.payload();
        std::cout << "Frame ID: " << id  << std::endl;
        for (int i = 0; i < data.size(); ++i) {
            std::cout << "Data[" << i << "]: " << static_cast<int>(static_cast<unsigned char>(data[i])) << std::endl;
        }
        if (id == 66) {
            speed = static_cast<unsigned char>(data[0]);
            std::cout << "Speed: " << static_cast<int>(speed) << " km/h" << std::endl;
            emit speedUpdated();
        }

        if (id == 77) {
            battery = static_cast<unsigned char>(data[0]);
            std::cout << "Battery : " << battery << " %" << std::endl;
            emit batteryUpdated();
        }
    }
}
