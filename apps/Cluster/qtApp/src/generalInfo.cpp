#include "generalInfo.hpp"

generalInfo::generalInfo(QObject *parent)
    : QObject(parent),
      m_weatherInfo("sun-256.png"),
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
            emit localTimeChanged();
        }

        QDate newDate = now.date();
        if (newDate != m_currentDate) {
            m_currentDate = newDate;
            emit currentDateChanged();
        }
    });

    timer->start(1000);
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &generalInfo::onWeatherDataReceived);

    fetchWeatherData();
}

QString generalInfo::getWeatherInfo() const
{
    return m_weatherInfo;
}

int generalInfo::getTemperature() const
{
    return m_temperature;
}

QString generalInfo::getLocalTime() const
{
    return m_localTime;
}

QString generalInfo::getCurrentDate() const
{
    return m_currentDate.toString("dd/MM/yyyy");;
}

void generalInfo::fetchWeatherData()
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &generalInfo::onWeatherDataReceived);
    QNetworkRequest request(QUrl("https://api.open-meteo.com/v1/forecast?latitude=41.1496&longitude=-8.6109&current_weather=true"));
    m_manager->get(request);
}

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
            emit temperatureChanged();
        }

        int code = currentWeather["weathercode"].toInt();
        QString desc;
        switch(code) {
            case 0: desc = "sun-256.png"; break;
            case 1: case 2: case 3: desc = "partly-cloudy-day-256.png"; break;
            case 45: case 48: desc = "clouds-256.png"; break;
            case 51: case 53: case 55: desc = "rain-256.png"; break;
            case 61: case 63: case 65: desc = "rain-256.png"; break;
            case 66: case 67: desc = "rain-256.png"; break;
            case 71: case 73: case 75: desc = "snow-256.png"; break;
            case 77: desc = "snow-256.png"; break;
            case 80: case 81: case 82: desc = "rain-256.png"; break;
            case 85: case 86: desc = "snow-256.png"; break;
            case 95: desc = "thunderstorm-256.png"; break;
            case 96: case 99: desc = "thunderstorm-256.png"; break;
            default: desc = "unknown.png"; break;
        }

        if (desc != m_weatherInfo) {
            m_weatherInfo = desc;
            emit weatherInfoChanged();
        }
    }
    reply->deleteLater();
}

