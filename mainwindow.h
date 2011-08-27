#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include "src/RtMidi.h"
#include "src/Bridge.h"

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

private:
    Ui::MainWindow *ui;
    Bridge *bridge;

    void refresh();
    void refreshSerial();
    void refreshMidiIn();
    void refreshMidiOut();
    void refreshMidi(QComboBox *combo, RtMidi *midi);

private slots:
    void onValueChanged();
    void onDisplayMessage(QString message);
    void onDebugMessage(QString message);
    void onMidiSent();
    void onMidiReceived();
    void onSerialTraffic();
};

#endif // MAINWINDOW_H
