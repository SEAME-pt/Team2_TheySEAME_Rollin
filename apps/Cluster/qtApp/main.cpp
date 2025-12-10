#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> 
#include <iostream>
#include <chrono>
#include <ctime>
#include "src/generalInfo.hpp"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    generalInfo info;

    engine.rootContext()->setContextProperty("generalInfo", &info);

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
