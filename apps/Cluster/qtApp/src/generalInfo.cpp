#include "generalInfo.hpp"
#include <QObject>
#include <QTimer>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

generalInfo::generalInfo(QObject *parent)
    : QObject(parent),
      _weatherInfo("sun"),
      _temperature(0.0f),
      _localTime("00:00"),
      _currentDate(QDate::currentDate()),
    _manager(nullptr),
    _positionSource(nullptr),
    _latitude(0.0),
    _longitude(0.0),
    _hasLocation(false)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        QDateTime now = QDateTime::currentDateTime();

        QString newTime = now.toString("HH:mm");
        if (newTime != _localTime) {
            _localTime = newTime;
            qDebug() << "Local time updated to:" << _localTime;
            emit localTimeChanged();
        }

        QDate newDate = now.date();
        if (newDate != _currentDate) {
            _currentDate = newDate;
            qDebug() << "Current date updated to:" << _currentDate.toString("dd/MM/yyyy");
            emit currentDateChanged();
        }
    });
    timer->start(1000);

    _manager = new QNetworkAccessManager(this);
    connect(_manager, &QNetworkAccessManager::finished,
            this, &generalInfo::onWeatherDataReceived);

    QTimer *weatherTimer = new QTimer(this);
    connect(weatherTimer, &QTimer::timeout, this, &generalInfo::fetchWeatherData);
    weatherTimer->start(10 * 1000);

    _positionSource = QGeoPositionInfoSource::createDefaultSource(this);
    if (_positionSource) {
        connect(_positionSource, &QGeoPositionInfoSource::positionUpdated, this, [this](const QGeoPositionInfo &info) {
            _latitude = info.coordinate().latitude();
            _longitude = info.coordinate().longitude();
            _hasLocation = true;
            qDebug() << "Location updated:" << _latitude << _longitude;
            fetchWeatherData();
            _positionSource->stopUpdates();
        });
        connect(_positionSource, &QGeoPositionInfoSource::errorOccurred, this, [this](QGeoPositionInfoSource::Error err) {
            _latitude = 41.1496;
            _longitude = -8.6109;
            _hasLocation = false;
            fetchWeatherData();
        });
        _positionSource->setUpdateInterval(1000);
        _positionSource->requestUpdate(3000);
    } else {
        _latitude = 41.1496;
        _longitude = -8.6109;
        _hasLocation = false;
        fetchWeatherData();
    }
}

generalInfo::~generalInfo()
{
    if (_manager) {
        delete _manager;
        _manager = nullptr;
    }
    if (_positionSource) {
        delete _positionSource;
        _positionSource = nullptr;
    }
}

QString generalInfo::getWeatherInfo() const
{
    return _weatherInfo;
}

int generalInfo::getTemperature() const
{
    return _temperature;
}

QString generalInfo::getLocalTime() const
{
    return _localTime;
}

QString generalInfo::getCurrentDate() const
{
    return _currentDate.toString("dd/MM/yyyy");
}

void generalInfo::fetchWeatherData()
{
    QString url = QString("https://api.open-meteo.com/v1/forecast?latitude=%1&longitude=%2&current_weather=true")
        .arg(_latitude, 0, 'f', 6)
        .arg(_longitude, 0, 'f', 6);
    QNetworkRequest request{QUrl(url)};
    _manager->get(request);
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
        if (temp != _temperature) {
            _temperature = temp;
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

        if (desc != _weatherInfo) {
            _weatherInfo = desc;
            qDebug() << "Weather updated to:" << _weatherInfo << "with temperature:" << _temperature;
            emit weatherInfoChanged();
        }
    }
    reply->deleteLater();
}
