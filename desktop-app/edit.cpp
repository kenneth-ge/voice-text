#include "edit.h"
#include <unordered_set>
#include <QGuiApplication>

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
    numbers[2].insert("to");
    numbers[2].insert("second");

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

    clipboard = QGuiApplication::clipboard();
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

    //qDebug() << "Message: " << message;

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

void edit::paste(){
    QProcess javaProcess;

    // Set the Java executable and arguments
    QString javaExecutable = "java"; // You might need to provide the full path to the java executable
    QStringList javaArguments;
    javaArguments << "paste";

    // Start the Java process
    javaProcess.start(javaExecutable, javaArguments);
    if (!javaProcess.waitForStarted()) {
        qDebug() << "Failed to start Java process.";
    }

    // Wait for the Java process to finish
    if (!javaProcess.waitForFinished()) {
        qDebug() << "Java process didn't finish.";
    }

    // Get the output and error output from the Java process
    QString output = QString::fromUtf8(javaProcess.readAllStandardOutput());
    QString errorOutput = QString::fromUtf8(javaProcess.readAllStandardError());

    qDebug() << "Java process output:" << output;
    qDebug() << "Java process error output:" << errorOutput;
}

void edit::clearOptions(){
    this->showingOptions = false;
    this->options.clear();
    this->selecting = false;
    emit newOptions();
}

void edit::pedalDoublePress(){
    // clear options (quick exit command)
    doublePress = true;
    clearOptions();

}

void edit::parseActionWord(QString action){
    if(remove.find(action) != remove.end()){
        // remove command
        emit removeSelected();

        qDebug() << "remove";

        clearOptions();
    }
    if(insert.find(action) != insert.end()){
        emit startInserting(this->text.lastIndexOf(currentFrag) + currentFrag.length());

        qDebug() << "insert after";

        clearOptions();
    }
}

void edit::commandRecvd(QString text, QString command){
    // this->selecting if this->showingOptions
    if(this->showingOptions/* || this->selecting*/){
        command = command.toLower().trimmed();
        command = command.replace(QRegularExpression("[^\\w\\s]"), "");

        auto idx = command.indexOf(' ');
        if(idx == -1)
            idx = command.length();

        auto puncIdx = command.indexOf(QRegularExpression("\\p{P}"));
        if(puncIdx != -1 && puncIdx < idx){
            idx = puncIdx;
        }

        //qDebug() << "command len: " << command.length();

        auto firstWord = command.mid(0, idx);
        if(exitCmds.find(firstWord) != exitCmds.end()){
            clearOptions();
        }else{
            // if selecting, then deal with those commands
            if(this->selecting){
                parseActionWord(firstWord);
            }else{
                // otherwise, make a selection, or make a selection while also doing an action
                int selection = -1;

                for(int i = 0; i < 10; i++){
                    if(numbers[i].find(firstWord) != numbers[i].end()){
                        selection = i;
                        break;
                    }
                }

                if(firstWord == "" || firstWord == "you"){
                    return;
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

                if(firstWord.length() + 1 < command.length()){
                    auto secondWord = command.mid(firstWord.length() + 1);

                    parseActionWord(secondWord);
                }
            }
        }

        return;
    }else{
        command = command.toLower().trimmed();
        command = command.replace(QRegularExpression("[^\\w\\s]"), "");

        auto idx = command.indexOf(' ');
        if(idx == -1)
            idx = command.length();

        auto puncIdx = command.indexOf(QRegularExpression("\\p{P}"));
        if(puncIdx != -1 && puncIdx < idx){
            idx = puncIdx;
        }

        //qDebug() << "command len: " << command.length();
        //qDebug() << "command: " << command;

        auto firstWord = command.mid(0, idx);

        if(firstWord == "paste"){
            clipboard->setText(this->text);

            // paste text
            paste();

            if(command == "paste and clear" || command == "paste and clear"){
                emit setText("");
            }

            return;
        }else if(command == "new line"){
            emit setText(text + "\n\n");
            return;
        }
    }

    //qDebug() << "command recvd: " << text << " | " << command;

    loadingScrn = true;
    emit emitLoading();

    //qDebug() << "Loading screen: " << loadingScrn;

    options.clear();

    //auto textCh = text.toUtf8();
    //auto commandCh = command.toUtf8();
    auto msg = (text + QStringLiteral("\0") + command + QStringLiteral("\0") + QStringLiteral("\0") + QStringLiteral("\0") + QStringLiteral("\0")).toUtf8();
    //qDebug() << msg.length() << " " << text.length() << " " << command.length();
    int len = msg.length();//textCh.length() + 1 + commandCh.length() + 4;

    this->sock->write(msg, len);
    this->sock->flush();

    emit newOptions();
}

edit::~edit(){
    delete sock;
    delete snackbarTimer;
}
