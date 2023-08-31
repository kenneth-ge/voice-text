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

    idleTimer = new QTimer(this);
    idleTimer->setSingleShot(true);
    idleTimer->setInterval(2500);
    connect(idleTimer, &QTimer::timeout, this, [this]() {
        //qDebug() << "User hasn't typed for a while!";
        qDebug() << "resume due to not typing";
        this->unpause();
    });

    unpause();
}

void vtt::pause(){
    qDebug() << "pause";
    sock->write("pause\n");
    sock->flush();
    isPaused = true;
    idleTimer->stop();
}

void vtt::unpause(){
    qDebug() << "unpause";
    sock->write("resume\n");
    sock->flush();
    isPaused = false;
}

void vtt::endSeg(){
    this->sock->write("end_seg\n");
    this->sock->flush();
}

void vtt::textHasChanged(QString text){
    this->cumulative.check();

    emit newText(text);

    // Check if the text change was triggered programmatically
    if (ignoreTextChange) {
        ignoreTextChange = false;
        return;
    }

    if(oldText == text){
        qDebug() << "text is same!";
        return;
    }

    oldText = text;

    qDebug() << "text has changed";
    qDebug() << "new len: " << text.length();
    idleTimer->start();

    this->cumulative.set(text);
    this->curr.clear();
    this->currIdx++;
    endSeg();

    emit newText(text);
}

QString vtt::getText(){
    /*if(!isCommand)
        return cumulative + curr;*/
    return cumulative.get();
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

    QString text = message.mid(idxSpace);

    //qDebug() << "number: " << idxStr << " " << idx;
    //qDebug() << "text: " << text;

    if(!isCommand){
        // update & commit previous text if necessary
        this->cumulative.update(this->curr);
        if(idx != this->currIdx)
            this->cumulative.commit();
    }

    this->curr = text;
    this->currIdx = idx;

    if(!isCommand){
        this->cumulative.update(this->curr);
    }

    ignoreTextChange = true;

    if(!isCommand)
        emit textChanged();
    else
        emit commandTextChanged();
}

void vtt::onStartInserting(int pos){
    //cumulative.changeCaretPos(pos);
}

void vtt::buttonPressed(){
    this->cumulative.update(curr);
    this->cumulative.commit();
    this->curr.clear();
    this->isCommand = true;

    endSeg();
}

int max(int a, int b){
    if(a > b)
        return a;
    return b;
}

void vtt::buttonReleased(){
    this->isCommand = false;
    this->command = curr;
    auto text = this->cumulative.get();
    emit newCommand(text.mid(max(0, text.length() - 2048), 2048), this->command);
    this->curr.clear();
    this->currIdx++;
}

void vtt::pedalDoublePress(){
    doublePressed = true;
}

void vtt::pedalPressed(){
    wasPaused = isPaused;
    idleTimer->stop();
    unpause();
    doublePressed = false;
    buttonPressed();
}

void vtt::pedalReleased(){
    if(!doublePressed){
        buttonReleased();
    }
    if(wasPaused){
        pause();
    }
}

void vtt::caretPositionChanged(int start, int end){
    if(start == end){
        cumulative.changeCaretPos(start);
        endSeg();
        this->currIdx++;
    }
}

void vtt::setText(QString text){
    qDebug() << "set text to: " << text;
    this->cumulative.set(text);
    emit setTextArea(text);
}

vtt::~vtt(){
    delete sock;
    delete idleTimer;
}

/* texthandler */
void texthandler::commit(){
    before += curr;
    currentPos += curr.length();
    curr.clear();
}

void texthandler::changeCaretPos(int idx){
    QString total = get();

    before = total.mid(0, idx);
    after = total.mid(idx);
    curr = "";

    qDebug() << "change caret pos";

    /*
    if(idx < currentPos){
        qDebug() << "idx < currentPos";
        qDebug() << "before: " << before;
        qDebug() << "after: " << after;
        after.prepend(before.mid(0, currentPos - idx));
        before.chop(currentPos - idx);
    }else{
        // idx > currentPos
        before += after.first(idx - currentPos);
        after.remove(0, idx - currentPos);
    }*/
}

void texthandler::update(QString s){
    this->curr = s;
}

void texthandler::check(){

}

QString texthandler::get(){
    QString ret = before + curr + after;

    if(ret.length() >= prevSize * 2){
        qDebug() << "duplication glitch";
        check();
    }
    prevSize = ret.length();

    return ret;
}

static int min(int a, int b){
    if(a < b)
        return a;
    return b;
}

void texthandler::set(QString s){
    currentPos = min(currentPos, s.length());
    before = s.mid(0, currentPos);
    after = s.mid(currentPos);
    currentPos = s.length();
    curr.clear();
}
