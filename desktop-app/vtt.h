#ifndef VTT_H
#define VTT_H

#include <QObject>
#include <QProcess>
#include <QTcpSocket>
#include <QtQml>

class vtt : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString text READ getText NOTIFY textChanged)
    Q_PROPERTY(QString commandText READ getCommandText NOTIFY textChanged)
public:
    vtt();
    ~vtt();
    QString getText();
    QString getCommandText();
    Q_INVOKABLE void buttonPressed();
    Q_INVOKABLE void buttonReleased();
    Q_INVOKABLE void textHasChanged(QString text);
signals:
    void textChanged();
    void newCommand(QString text, QString command);
    void commandTextChanged();
    void moveCaretToEnd(int pos);
private:
    QTcpSocket *sock;
    QString cumulative;
    QString curr;
    QString command;
    int currIdx;
    bool isCommand;
    QString commandText;
    bool ignoreTextChange;
private slots:
    void onMessage();
};

#endif // VTT_H
