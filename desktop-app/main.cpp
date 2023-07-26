#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QProcess>
#include <QtQml>

#include "vtt.h"
#include "edit.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterUncreatableType<vtt>("App", 1, 0, "Vtt", "");
    qmlRegisterUncreatableType<edit>("App", 1, 0, "Edit", "");

    vtt vtt;
    engine.rootContext()->setContextProperty("Vtt", &vtt);

    edit ed;
    engine.rootContext()->setContextProperty("Edit", &ed);

    qDebug() << "got to connect statement";

    QObject::connect(&vtt, &vtt::newCommand, &ed, &edit::commandRecvd);
    QObject::connect(&vtt, &vtt::newText, &ed, &edit::textRecvd);
    QObject::connect(&ed, &edit::startInserting, &vtt, &vtt::onStartInserting);

    qDebug() << "connected";

    qmlRegisterType<option>("com.voicetext", 1, 0, "Option");

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
