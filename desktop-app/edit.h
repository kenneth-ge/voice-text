#ifndef EDIT_H
#define EDIT_H

#include <QObject>
#include <QProcess>
#include <QtQml>
#include <QList>
#include <QVariantList>

class option : public QObject {
    Q_OBJECT
    Q_PROPERTY(const QString frag READ getFrag NOTIFY emitFrag)
public:
    const QString frag;
    option(QString opt) : frag(opt) {
        emit emitFrag();
    }
    QString getFrag(){
        return frag;
    }
signals:
    void emitFrag();
};

class edit : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QList<option*> options READ getOptions NOTIFY optionsChanged)
    Q_PROPERTY(bool loadingScrn READ isLoading NOTIFY emitLoading)
public:
    edit();
    ~edit();
    QList<option*> getOptions();
    bool isLoading();
signals:
    void optionsChanged();
    void newOptions();
    void emitLoading();
public slots:
    void commandRecvd(QString text, QString command);
private:
    bool loadingScrn = false;
    QList<option*> options;
    QTcpSocket* sock;
private slots:
    void onMessage();
};

#endif // EDIT_H
