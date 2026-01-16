#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> 
#include <iostream>
#include <chrono>
#include <ctime>
#include "src/generalInfo.hpp"
#include "src/systemInfo.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    generalInfo info;
    systemInfo sysInfo;
    if (!sysInfo.start("can0")) {
        std::cerr << "Failed to start CAN bus on can0" << std::endl;
    }

    engine.rootContext()->setContextProperty("generalInfo", &info);
    engine.rootContext()->setContextProperty("systemInfo", &sysInfo);
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
