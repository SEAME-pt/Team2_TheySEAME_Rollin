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

class infoProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString weatherInfo READ getWeatherInfo NOTIFY weatherInfoChanged)
    Q_PROPERTY(int temperature READ getTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(QString localTime READ getLocalTime NOTIFY localTimeChanged)
    Q_PROPERTY(QString currentDate READ getCurrentDate NOTIFY currentDateChanged)
public:
    explicit infoProvider(QObject *parent = nullptr);

    QString getWeatherInfo() const;
    int getTemperature() const;
    QString getLocalTime() const;
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
    void fetchWeatherData();
    void onWeatherDataReceived(QNetworkReply* reply);
};
