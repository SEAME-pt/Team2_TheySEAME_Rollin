#include "infoProvider.hpp"

infoProvider::infoProvider(QObject *parent)
    : QObject(parent),
      m_weatherInfo("Unknown"),
      m_temperature(0.0f),
      m_localTime("00:00"),
      m_currentDate(QDate::currentDate())
{
}

QString infoProvider::getWeatherInfo() const
{
    return m_weatherInfo;
}

float infoProvider::getTemperature() const
{
    return m_temperature;
}

QString infoProvider::getLocalTime() const
{
    return m_localTime;
}

QDate infoProvider::getCurrentDate() const
{
    return m_currentDate;
}

void infoProvider::fetchWeatherData()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &infoProvider::onWeatherDataReceived);

    QNetworkRequest request(QUrl("http://api.weatherapi.com/v1/current.json?key=YOUR_API_KEY&q=YOUR_LOCATION"));
    manager->get(request);
}


