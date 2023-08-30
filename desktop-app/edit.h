#ifndef EDIT_H
#define EDIT_H

#include <QObject>
#include <QProcess>
#include <QtQml>
#include <QList>
#include <QVariantList>
#include <unordered_set>

#include <QClipboard>

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
    Q_PROPERTY(float snackbarOpacity READ getSnackbarOpacity NOTIFY emitSnackbarOpacity)
    Q_PROPERTY(int selected READ getSelected NOTIFY emitSelected)
public:
    edit();
    ~edit();
    QList<option*> getOptions();
    bool isLoading();
    float getSnackbarOpacity();
    int getSelected();
signals:
    void optionsChanged();
    void newOptions();
    void emitLoading();
    void emitSnackbarOpacity();
    void emitSelect(QString fragment);
    void removeSelected();
    void startInserting(int idx);
    void setText(QString text);
    void openSave();
    void emitSelected();
    void needsPause();
    void needsUnpause();
public slots:
    void commandRecvd(QString text, QString command);
    void textRecvd(QString text);
    void pedalDoublePress();
    void saveFile(QString filePath);
    void nextOption(bool wasHolding);
private:
    void paste();
    void clearOptions();
    // returns true if action parsed successfully
    bool parseActionWord(QString action);

    int selectedOption;
    bool doublePress = false;
    bool loadingScrn = false;
    bool showingOptions = false;
    float snackbarOpacity = false;
    bool selecting = false;
    QList<option*> options;
    QTcpSocket* sock;
    std::unordered_set<QString> exitCmds, numbers[10], remove, insert;
    QTimer* snackbarTimer;
    QString text;
    QString currentFrag;
    QClipboard* clipboard;
private slots:
    void onMessage();
};

#endif // EDIT_H
