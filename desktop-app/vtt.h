#ifndef VTT_H
#define VTT_H

#include <QObject>
#include <QProcess>
#include <QTcpSocket>

class vtt : public QObject
{
public:
    vtt();
    ~vtt();
private:
    void onMessage();
    QProcess *proc;
    //QTcpSocket *socket;
};

#endif // VTT_H
