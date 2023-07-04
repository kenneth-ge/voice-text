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
public:
    vtt();
    ~vtt();
    QString getText();
signals:
    void textChanged();
private:
    QTcpSocket *sock;
    QString cumulative;
    QString curr;
    int currIdx;
private slots:
    void onMessage();
};

#endif // VTT_H
