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
#include <iostream>

class generalInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString weatherInfo READ getWeatherInfo NOTIFY weatherInfoChanged)
    Q_PROPERTY(int temperature READ getTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(QString localTime READ getLocalTime NOTIFY localTimeChanged)
    Q_PROPERTY(QString currentDate READ getCurrentDate NOTIFY currentDateChanged)
public:
    /**
    * @brief The generalInfo class
    *
    * Provides current system information such as:
    * - Local time
    * - Current date
    * - Weather information (temperature + icon)
    *
    * Data is updated periodically:
    * - Time and date every second
    * - Weather via HTTP request to Open-Meteo API
    *
    * @param parent Optional parent QObject
    *
	  * Requirement traceability:
    * [impl->dsn~design-requirement-cluster-datetime~1]
    * [impl->dsn~design-requirement-cluster-weather~1]
    *
    */
    explicit generalInfo(QObject *parent = nullptr);

    /**
    * @brief Returns the current weather icon filename.
    *
	  * Requirement traceability:
    * [impl->dsn~design-requirement-cluster-weather~1]
    *
    * @return QString representing icon (e.g., "sun-256.png")
    *
    */
    QString getWeatherInfo() const;

    /**
    * @brief Returns the current temperature in Celsius.
    *
	  * Requirement traceability:
    * [impl->dsn~design-requirement-cluster-weather~1]
    *
    * @return int temperature
    *
    */
    int getTemperature() const;

    /**
    * @brief Returns the local time in HH:mm format.
    *
	  * Requirement traceability:
    * [impl->dsn~design-requirement-cluster-datetime~1]
    *
    * @return QString current time
    *
    */
    QString getLocalTime() const;

    /**
    * @brief Returns the current date in dd/MM/yyyy format.
    *
	  * Requirement traceability:
    * [impl->dsn~design-requirement-cluster-datetime~1]
    *
    * @return QString current date
    *
    */
    QString getCurrentDate() const;
    
private:
    QString m_weatherInfo;
    int m_temperature;
    QString m_localTime;
    QDate   m_currentDate;
    QNetworkAccessManager* m_manager;

signals:
    void weatherInfoChanged();
    void temperatureChanged();
    void localTimeChanged();
    void currentDateChanged();

public slots:

    /**
    * @brief Fetches current weather data from Open-Meteo API.
    *
	  * Requirement traceability:
    * [impl->dsn~design-requirement-cluster-weather~1]
    *
    */
    void fetchWeatherData();

    /**
    * @brief Handles the network reply for weather API.
    * Parses JSON and updates temperature and weather icon.
    * Emits temperatureChanged() and weatherInfoChanged() if necessary.
    * @param reply QNetworkReply* from QNetworkAccessManager
    *
	  * Requirement traceability:
    * [impl->dsn~design-requirement-cluster-weather~1]
    *
    */
    void onWeatherDataReceived(QNetworkReply* reply);
};
