#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QTime>
#include <QMenuBar>
#include <QTimer>
#include "RtMidi.h"
#include "Bridge.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *, QEvent *);

    void resizeEvent(QResizeEvent *);

private:
    Ui::MainWindow *ui;
    Bridge *bridge;
    QPixmap pxLedOn;
    QPixmap pxLedOff;
    QThread *workerThread;
    int scrollbackSize;
    QTimer debugListTimer;
    QStringList debugListMessages;

    void refresh();
    void refreshSerial();
    void refreshMidiIn();
    void refreshMidiOut();
    void refreshMidi(QComboBox *combo, RtMidi *midi);

private slots:
    void onValueChanged();
    void onDisplayMessage(QString message);
    void onDebugMessage(QString message);
    void onDebugClicked(bool value);
    void refreshDebugList();
    void showPreferences();
    void showAboutBox();
};

#endif // MAINWINDOW_H
