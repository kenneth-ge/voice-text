#include "vtt.h"

#include <QProcess>
#include <QDebug>
#include <QTcpSocket>
#include <QHostAddress>
#include <QString>
#include <utility>
#include <algorithm>

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

void vtt::textHasChanged(QString text){
    // Check if the text change was triggered programmatically
    if (ignoreTextChange) {
        ignoreTextChange = false;
        return;
    }

    this->cumulative = text;
    this->curr.clear();
    this->currIdx++;
    qDebug() << "type";
}

QString vtt::getText(){
    if(!isCommand)
        return cumulative + curr;
    return cumulative;
}

QString vtt::getCommandText(){
    if(isCommand)
        return curr;
    return "";
}

void vtt::onMessage(){
    QString message = QString::fromUtf8(sock->readAll().trimmed());

    int idxSpace = message.indexOf(' ');

    if(idxSpace <= 0){
        return;
    }

    QString idxStr = message.mid(0, idxSpace);

    int idx = idxStr.toInt();

    QString text = message.mid(idxSpace, message.length() - idxSpace);

    qDebug() << "number: " << idxStr << " " << idx;
    qDebug() << "text: " << text;

    if(!isCommand && idx != this->currIdx){
        this->cumulative += this->curr;
    }

    this->curr.clear();
    this->curr += text;
    this->currIdx = idx;

    ignoreTextChange = true;
    emit textChanged();
    emit moveCaretToEnd(this->getText().length());
}

void vtt::buttonPressed(){
    this->cumulative += this->curr;
    this->curr.clear();
    this->isCommand = true;
}

int max(int a, int b){
    if(a > b)
        return a;
    return b;
}

void vtt::buttonReleased(){
    qDebug() << "button released";
    this->isCommand = false;
    this->command = curr;
    emit newCommand(this->cumulative.mid(max(0, this->cumulative.length() - 2048), 2048), this->command);
    this->curr.clear();
    this->currIdx++;
}

vtt::~vtt(){
    delete sock;
}

