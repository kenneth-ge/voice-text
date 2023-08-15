#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QProcess>
#include <QtQml>

#include "vtt.h"
#include "edit.h"
#include "pedal_manager.h"

int main(int argc, char *argv[])
{
    // commented because deprecated
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterUncreatableType<vtt>("App", 1, 0, "Vtt", "");
    qmlRegisterUncreatableType<edit>("App", 1, 0, "Edit", "");
    qmlRegisterUncreatableType<edit>("App", 1, 0, "PM", "");

    vtt vtt;
    engine.rootContext()->setContextProperty("Vtt", &vtt);

    edit ed;
    engine.rootContext()->setContextProperty("Edit", &ed);

    qDebug() << "got to connect statement";

    QObject::connect(&vtt, &vtt::newCommand, &ed, &edit::commandRecvd);
    QObject::connect(&vtt, &vtt::newText, &ed, &edit::textRecvd);
    QObject::connect(&ed, &edit::startInserting, &vtt, &vtt::onStartInserting);
    QObject::connect(&ed, &edit::removeSelected, &vtt, &vtt::deleteSelected);
    QObject::connect(&ed, &edit::setText, &vtt, &vtt::setText);

    qDebug() << "connected";

    qmlRegisterType<option>("com.voicetext", 1, 0, "Option");

    // monitor pedal file
    QFileSystemWatcher watcher;

    watcher.addPath("/dev/shm/footpedal");

    pedal_manager pm;
    engine.rootContext()->setContextProperty("PM", &pm);
    QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, &pm, &pedal_manager::pedalChanged);
    QObject::connect(&pm, &pedal_manager::pedalDown, &vtt, &vtt::pedalPressed);
    QObject::connect(&pm, &pedal_manager::pedalUp, &vtt, &vtt::pedalReleased);
    QObject::connect(&pm, &pedal_manager::pedalDoublePress, &ed, &edit::pedalDoublePress);
    QObject::connect(&pm, &pedal_manager::pedalDoublePress, &vtt, &vtt::pedalDoublePress);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
