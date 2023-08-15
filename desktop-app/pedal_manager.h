#ifndef PEDAL_MANAGER_H
#define PEDAL_MANAGER_H

#include <QObject>
#include <QProcess>
#include <QTcpSocket>
#include <QtQml>

class pedal_manager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    pedal_manager();
    bool pressed;
public slots:
    void pedalChanged();
signals:
    void pedalDown();
    void pedalUp();
    void pedalDoublePress();
private:
    long long lastTime;
};

#endif // PEDAL_MANAGER_H
