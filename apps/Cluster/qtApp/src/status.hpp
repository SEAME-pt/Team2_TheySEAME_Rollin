#pragma once
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>

class status
{
    Q_OBJECT
    Q_PROPERTY(float percentage READ percentage WRITE setPercentage NOTIFY updated)

    private:
        float batteryLevel;
        
    public:
        explicit status(QObject *parent = nullptr);

        float getBattery() const;

    private slots:
        void updateBatteryLevel();
    signals:
            void batteryChanged();
};
