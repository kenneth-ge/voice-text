#ifndef EDIT_H
#define EDIT_H

#include <QObject>
#include <QProcess>
#include <QtQml>
#include <QList>
#include <QVariantList>
#include <unordered_set>

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
    //Q_PROPERTY(bool isShowingSnackbar READ isShowing NOTIFY emitShowing)
public:
    edit();
    ~edit();
    QList<option*> getOptions();
    bool isLoading();
    //bool isShowing();
signals:
    void optionsChanged();
    void newOptions();
    void emitLoading();
    //void emitShowing();
public slots:
    void commandRecvd(QString text, QString command);
private:
    bool loadingScrn = false;
    //bool showingOptions = false;
    //bool showingSnackbar = false;
    QList<option*> options;
    QTcpSocket* sock;
    //std::unordered_set<QString> exitCmds;
private slots:
    void onMessage();
};

#endif // EDIT_H
