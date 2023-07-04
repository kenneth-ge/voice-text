#include "vtt.h"

#include <QProcess>
#include <QDebug>
#include <QTcpSocket>
#include <QHostAddress>

vtt::vtt()
{
    sock = new QTcpSocket(this);

    connect(sock, &QTcpSocket::readyRead, this, &vtt::onMessage);

    sock->connectToHost(QHostAddress("127.0.0.1"), 5005);
    connect(sock, &QTcpSocket::connected, this, [](){
        qDebug() << "connected";
    });

    qDebug() << "Connecting to host...";

    // we need to wait...
    if(!sock->waitForConnected(5000)){
        qDebug() << "Error: " << sock->errorString();
    }
}

QString vtt::getText(){
    return cumulative + curr;
}

void vtt::onMessage(){
    QString message = QString::fromUtf8(sock->readAll().trimmed());

    int idxSpace = message.indexOf(' ');

    if(idxSpace <= 0){
        return;
    }

    QStringRef idxStr(&message, 0, idxSpace);
    int idx = idxStr.toInt();

    QStringRef text(&message, idxSpace, message.length() - 1);

    qDebug() << "number: " << idxStr << " " << idx;
    qDebug() << "text: " << text;

    if(idx != this->currIdx){
        this->cumulative += this->curr;
    }

    this->curr.clear();
    this->curr += text;
    this->currIdx = idx;

    emit textChanged();
}

vtt::~vtt(){
    delete sock;
}

