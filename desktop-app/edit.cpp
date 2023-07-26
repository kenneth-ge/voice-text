#include "edit.h"
#include <unordered_set>

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
    exitCmds.insert("exit");
    exitCmds.insert("clear");

    // each of these numbers arrays contains different
    // words that could indicate a number selection
    // each set is disjoint
    numbers[0].insert("0");
    numbers[0].insert("zero");

    numbers[1].insert("1");
    numbers[1].insert("one");
    numbers[1].insert("won");
    numbers[1].insert("what");

    numbers[2].insert("2");
    numbers[2].insert("two");
    numbers[2].insert("too");

    numbers[3].insert("3");
    numbers[3].insert("three");
    numbers[3].insert("tree");

    numbers[4].insert("4");
    numbers[4].insert("four");
    numbers[4].insert("for");
    numbers[4].insert("fore");

    numbers[5].insert("5");
    numbers[5].insert("five");

    numbers[6].insert("6");
    numbers[6].insert("six");

    numbers[7].insert("7");
    numbers[7].insert("seven");

    numbers[8].insert("8");
    numbers[8].insert("ate");
    numbers[8].insert("eight");

    numbers[9].insert("9");
    numbers[9].insert("nine");

    remove.insert("remove");
    remove.insert("delete");

    insert.insert("append");
    insert.insert("insert");

    snackbarTimer = new QTimer(this);
    snackbarTimer->setInterval(5000);

    connect(snackbarTimer, &QTimer::timeout, this, [this]() {
        this->snackbarOpacity = 0.0;
        emit emitSnackbarOpacity();
    });
}

bool edit::isLoading(){
    return loadingScrn;
}

float edit::getSnackbarOpacity(){
    return this->snackbarOpacity;
}

QList<option*> edit::getOptions(){
    return options;
}

void edit::onMessage(){
    loadingScrn = false;
    emit emitLoading();

    QString message = QString::fromUtf8(sock->readAll().trimmed());

    qDebug() << "Message: " << message;

    QStringList l = message.split(QStringLiteral("\0"));
    std::unordered_set<QString> fragments;

    for(QString &s : l){
        s = s.trimmed();

        if(s.length() == 0)
            continue;

        if(fragments.find(s) != fragments.end())
            continue;

        if(!text.contains(s))
            continue;

        option* o = new option(s);

        this->options.append(o);

        fragments.insert(s);
    }

    emit newOptions();
    this->showingOptions = true;
}

void edit::textRecvd(QString text){
    this->text = text;
}

void edit::commandRecvd(QString text, QString command){
    // this->selecting if this->showingOptions
    if(this->showingOptions/* || this->selecting*/){
        command = command.toLower().trimmed();

        auto idx = command.indexOf(' ');
        if(idx == -1)
            idx = command.length();

        auto puncIdx = command.indexOf(QRegularExpression("\\p{P}"));
        if(puncIdx != -1 && puncIdx < idx){
            idx = puncIdx;
        }

        auto firstWord = command.mid(0, idx);
        if(exitCmds.find(firstWord) != exitCmds.end()){
            this->showingOptions = false;
            this->options.clear();
            this->selecting = false;
            emit newOptions();
        }else{
            // if selecting, then deal with those commands
            if(this->selecting){
                if(remove.find(firstWord) != remove.end()){
                    // remove command
                    emit removeSelected();
                }
                if(insert.find(firstWord) != insert.end()){
                    emit startInserting(this->text.lastIndexOf(currentFrag) + currentFrag.length());
                }
            }else{
                // otherwise, make a selection
                int selection = -1;

                for(int i = 0; i < 10; i++){
                    if(numbers[i].find(firstWord) != numbers[i].end()){
                        selection = i;
                        break;
                    }
                }

                if(selection == -1){
                    this->snackbarOpacity = 1.0;
                    emit emitSnackbarOpacity();
                    snackbarTimer->start();
                    return;
                }

                qDebug() << "selection: " << selection;

                //search from end, and select
                auto frag = this->options[selection]->frag;

                emit emitSelect(frag);
                this->selecting = true;
                this->currentFrag = frag;
            }
        }

        return;
    }

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
    delete snackbarTimer;
}
