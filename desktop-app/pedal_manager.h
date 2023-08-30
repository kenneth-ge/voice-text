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
    ~pedal_manager();
    bool pressed;
    bool holding;
public slots:
    void pedalChanged();
signals:
    void pedalDown();
    void pedalUp(bool afterHold);
    void pedalDoublePress();
    void pedalHeld();
    void pedalUpAfterHold();
private:
    long long lastTime;
    long long lastPress;
    long long lastHeld;
    long long lastRelease;
    QTimer* timer;
};

#endif // PEDAL_MANAGER_H
