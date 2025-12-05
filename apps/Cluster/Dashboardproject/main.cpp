#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> 
#include <iostream>
#include <chrono>
#include <ctime>
#include "src/infoProvider.hpp"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    infoProvider info;

    engine.rootContext()->setContextProperty("infoProvider", &info);

    engine.addImportPath("qrc:/qml");
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
