#ifndef VTT_H
#define VTT_H

#include <QObject>
#include <QProcess>
#include <QTcpSocket>
#include <QtQml>

class texthandler {
public:
    void commit();
    void changeCaretPos(int idx);
    void update(QString s);
    QString get();
    void set(QString s);
    void check();
private:
    int currentPos = 0;
    QString curr;
    QString before, after;
    int prevSize = 0;
};

class vtt : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString text READ getText NOTIFY textChanged)
    Q_PROPERTY(QString commandText READ getCommandText NOTIFY commandTextChanged)
public:
    vtt();
    ~vtt();
    QString getText();
    QString getCommandText();
    Q_INVOKABLE void buttonPressed();
    Q_INVOKABLE void buttonReleased();
    Q_INVOKABLE void textHasChanged(QString text);
public slots:
    void onStartInserting(int pos);
    void caretPositionChanged(int start, int end);
    void pedalPressed();
    void pedalReleased();
    void pedalDoublePress();
    void setText(QString text);
    void pause();
    void unpause();
    void endSeg();
signals:
    void textChanged();
    void newCommand(QString text, QString command);
    void commandTextChanged();
    void newText(QString text);
    void setTextArea(QString text);
private:
    QTcpSocket *sock;
    QTimer *idleTimer;
    texthandler cumulative;
    QString curr;
    QString command;
    int currIdx;
    bool isCommand;
    QString commandText;
    bool ignoreTextChange;
    bool doublePressed = false;
    bool isPaused = false;
    bool wasPaused = false;
private slots:
    void onMessage();
};

#endif // VTT_H
