#include "vtt.h"

#include <QProcess>
#include <QDebug>
#include <QTcpSocket>

vtt::vtt()
{
    //QTcpSocket socket;

    qDebug() << "Hi, testing" << Qt::endl;
    QString program = "python";
    QStringList arguments;

    arguments << "./transcribe.py";

    proc = new QProcess(this);
    proc->start(program, arguments);

    connect(proc, &QProcess::readyRead, this, &vtt::onMessage);
}

void vtt::onMessage(){
    QString output(proc->readAllStandardOutput());

    qDebug() << "output: " << output;

    QString error(proc->readAllStandardError());

    qDebug() << "error: " << error;
}

vtt::~vtt(){
    delete proc;
}

