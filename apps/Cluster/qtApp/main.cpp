#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> 
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include "src/generalInfo.hpp"
#include "src/systemInfo.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    generalInfo info;
    systemInfo sysInfo;
    if (!sysInfo.start()) {
        std::cerr << "Failed to communicate with kuksa" << std::endl;
    }

    // Enable test mode if TEST_ANIMATION environment variable is set
    if (std::getenv("TEST_ANIMATION")) {
        sysInfo.setTestMode(true);
        std::cout << "Test mode enabled: car distance will animate automatically" << std::endl;
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
