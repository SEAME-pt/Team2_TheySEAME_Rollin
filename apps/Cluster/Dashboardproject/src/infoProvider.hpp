#pragma once
#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>


class infoProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString weatherInfo READ weatherInfo NOTIFY weatherInfoChanged)
    Q_PROPERTY(float temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(QString localTime READ localTime NOTIFY localTimeChanged)
    Q_PROPERTY(QDate currentDate READ currentDate NOTIFY currentDateChanged)
public:
    explicit infoProvider(QObject *parent = nullptr);

    QString getWeatherInfo() const { return m_weatherInfo; }
    float getTemperature() const { return m_temperature; }
    QString getLocalTime() const { return m_localTime; }
    QDate getCurrentDate() const { return m_currentDate; }
private:
    QString m_weatherInfo;
    float m_temperature;
    QString m_localTime;
    QDate   m_currentDate;

signals:
    void weatherInfoChanged();
    void temperatureChanged();
    void localTimeChanged();
    void currentDateChanged();

public slots:
    void fetchWeatherData();
    void onWeatherDataReceived(QNetworkReply* reply);
};
