#include "generalInfo.hpp"
#include <QObject>
#include <QTimer>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

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
 *=======================Requirements traceability========================
 *        [impl->dsn~design-requirement-cluster-datetime~1]
 *        [impl->dsn~design-requirement-cluster-weather~1]
 *========================================================================
 */
generalInfo::generalInfo(QObject *parent)
    : QObject(parent),
      m_weatherInfo("sun"),
      m_temperature(0.0f),
      m_localTime("00:00"),
      m_currentDate(QDate::currentDate()),
      m_manager(nullptr)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        QDateTime now = QDateTime::currentDateTime();

        QString newTime = now.toString("HH:mm");
        if (newTime != m_localTime) {
            m_localTime = newTime;
            qDebug() << "Local time updated to:" << m_localTime;
            emit localTimeChanged();
        }

        QDate newDate = now.date();
        if (newDate != m_currentDate) {
            m_currentDate = newDate;
            qDebug() << "Current date updated to:" << m_currentDate.toString("dd/MM/yyyy");
            emit currentDateChanged();
        }
    });
    timer->start(1000);

    // Setup network manager for weather data
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &generalInfo::onWeatherDataReceived);

    fetchWeatherData();
}

/**
 * @brief Returns the current weather icon filename.
 * @return QString representing icon (e.g., "sun-256.png")
 *
 *=======================Requirements traceability========================
 *        [impl->dsn~design-requirement-cluster-weather~1]
 *========================================================================
 */
QString generalInfo::getWeatherInfo() const
{
    return m_weatherInfo;
}

/**
 * @brief Returns the current temperature in Celsius.
 * @return int temperature
 *
 *=======================Requirements traceability========================
 *        [impl->dsn~design-requirement-cluster-weather~1]
 *========================================================================
 */
int generalInfo::getTemperature() const
{
    return m_temperature;
}

/**
 * @brief Returns the local time in HH:mm format.
 * @return QString current time
 *
 *=======================Requirements traceability========================
 *        [impl->dsn~design-requirement-cluster-datetime~1]
 *========================================================================
 */
QString generalInfo::getLocalTime() const
{
    return m_localTime;
}

/**
 * @brief Returns the current date in dd/MM/yyyy format.
 * @return QString current date
 *
 *=======================Requirements traceability========================
 *        [impl->dsn~design-requirement-cluster-datetime~1]
 *========================================================================
 */
QString generalInfo::getCurrentDate() const
{
    return m_currentDate.toString("dd/MM/yyyy");
}

/**
 * @brief Fetches current weather data from Open-Meteo API.
 *
 *=======================Requirements traceability========================
 *        [impl->dsn~design-requirement-cluster-weather~1]
 *========================================================================
 */
void generalInfo::fetchWeatherData()
{
    QNetworkRequest request(QUrl("https://api.open-meteo.com/v1/forecast?latitude=41.1496&longitude=-8.6109&current_weather=true"));
    m_manager->get(request);
}

/**
 * @brief Handles the network reply for weather API.
 * Parses JSON and updates temperature and weather icon.
 * Emits temperatureChanged() and weatherInfoChanged() if necessary.
 *
 * @param reply QNetworkReply* from QNetworkAccessManager
 *
 *=======================Requirements traceability========================
 *        [impl->dsn~design-requirement-cluster-weather~1]
 *========================================================================
 */
void generalInfo::onWeatherDataReceived(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isNull()) {
        QJsonObject obj = doc.object();
        QJsonObject currentWeather = obj["current_weather"].toObject();

        int temp = static_cast<int>(currentWeather["temperature"].toDouble());
        if (temp != m_temperature) {
            m_temperature = temp;
            qDebug() << "Temperature updated to:" << m_temperature; 
            emit temperatureChanged();
        }

        int code = currentWeather["weathercode"].toInt();
        QString desc;
        switch(code) {
            case 0: desc = "sun"; break;
            case 1: case 2: case 3: desc = "partly-cloudy-day"; break;
            case 45: case 48: desc = "clouds"; break;
            case 51: case 53: case 55: desc = "rain"; break;
            case 61: case 63: case 65: desc = "rain"; break;
            case 66: case 67: desc = "rain"; break;
            case 71: case 73: case 75: desc = "snow"; break;
            case 77: desc = "snow"; break;
            case 80: case 81: case 82: desc = "rain"; break;
            case 85: case 86: desc = "snow"; break;
            case 95: desc = "thunderstorm"; break;
            case 96: case 99: desc = "thunderstorm"; break;
            default: desc = "unknown"; break;
        }

        if (desc != m_weatherInfo) {
            m_weatherInfo = desc;
            qDebug() << "Weather updated to:" << m_weatherInfo << "with temperature:" << m_temperature;
            emit weatherInfoChanged();
        }
    }
    reply->deleteLater();
}
