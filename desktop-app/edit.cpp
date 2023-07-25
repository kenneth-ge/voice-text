#include "edit.h"

edit::edit(){
    sock = new QTcpSocket(this);

    connect(sock, &QTcpSocket::readyRead, this, &edit::onMessage);

    //proc->start("/bin/bash", QStringList() << "-i" << "conda activate && python editor.py");

    sock->connectToHost(QHostAddress("127.0.0.1"), 5010);
    connect(sock, &QTcpSocket::connected, this, [](){
        qDebug() << "connected";
    });

    qDebug() << "Connecting to host...";

    // we need to wait...
    if(!sock->waitForConnected(5000)){
        qDebug() << "Error: " << sock->errorString();
    }

    // initialize command and control commands
    //exitCmds.insert("exit");
    //exitCmds.insert("clear");
}

bool edit::isLoading(){
    return loadingScrn;
}

QList<option*> edit::getOptions(){
    return options;
}

void edit::onMessage()
{
    loadingScrn = false;
    emit emitLoading();

    QString message = QString::fromUtf8(sock->readAll().trimmed());

    qDebug() << "Message: " << message;

    QStringList l = message.split(QStringLiteral("\0"));

    for(QString &s : l){
        s = s.trimmed();

        if(s.length() == 0)
            continue;

        option* o = new option(s);

        this->options.append(o);
    }

    emit newOptions();
    //this->showingOptions = true;
}

/*bool edit::isShowing(){
    return this->showingSnackbar;
}*/

void edit::commandRecvd(QString text, QString command){
    /*if(this->showingOptions){
        auto firstWord = command.mid(0, command.indexOf(' '));
        if(exitCmds.find(firstWord) != exitCmds.end()){
            this->showingOptions = false;
            this->options.clear();
        }else{
            //this->showingSnackbar = true;
        }
    }*/

    qDebug() << "command recvd: " << text << " | " << command;

    loadingScrn = true;
    emit emitLoading();

    qDebug() << "Loading screen: " << loadingScrn;

    options.clear();

    //auto textCh = text.toUtf8();
    //auto commandCh = command.toUtf8();
    auto msg = (text + QStringLiteral("\0") + command + QStringLiteral("\0") + QStringLiteral("\0") + QStringLiteral("\0") + QStringLiteral("\0")).toUtf8();
    qDebug() << msg.length() << " " << text.length() << " " << command.length();
    int len = msg.length();//textCh.length() + 1 + commandCh.length() + 4;

    this->sock->write(msg, len);
    this->sock->flush();

    emit newOptions();
}

edit::~edit(){
    delete sock;
}
