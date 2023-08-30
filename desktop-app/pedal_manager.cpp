#include "pedal_manager.h"

pedal_manager::pedal_manager()
{
    timer = new QTimer(this);
    timer->setInterval(500);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this](){
        if(pressed && lastPress >= lastRelease){
            if(lastHeld <= lastRelease){
                holding = true;
                lastHeld = QDateTime::currentMSecsSinceEpoch();
                emit pedalHeld();
            }
        }
    });
}

void pedal_manager::pedalChanged()
{
    pressed = !pressed;

    if(!timer->isActive()){
        timer->start();
    }

    if(pressed){
        lastPress = QDateTime::currentMSecsSinceEpoch();
        emit pedalDown();
    }else{
        lastRelease = QDateTime::currentMSecsSinceEpoch();
        emit pedalUp(holding);

        if(holding){
            emit pedalUpAfterHold();
        }
    }

    if(pressed){
        long long currentTime = QDateTime::currentMSecsSinceEpoch();

        if(currentTime - lastTime < 250){
            // double press
            //qDebug() << "double press";
            emit pedalDoublePress();
        }

        this->lastTime = currentTime;
    }

    holding = false;
}

pedal_manager::~pedal_manager()
{
    delete timer;
}
