#include "pedal_manager.h"

pedal_manager::pedal_manager()
{

}

void pedal_manager::pedalChanged()
{
    pressed = !pressed;
    qDebug() << "pressed: " << pressed;

    if(pressed){
        emit pedalDown();
    }else{
        emit pedalUp();
    }

    if(pressed){
        long long currentTime = QDateTime::currentMSecsSinceEpoch();

        if(currentTime - lastTime < 1000){
            // double press
            //qDebug() << "double press";
            emit pedalDoublePress();
        }

        this->lastTime = currentTime;
    }
}
