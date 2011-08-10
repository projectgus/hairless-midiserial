#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include "src/RtMidi.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    RtMidiOut *m_midiout;
    RtMidiIn *m_midiin;

    void refresh();
    void refreshMidi(QComboBox *combo, RtMidi *midi);
    void refreshSerial();
};

#endif // MAINWINDOW_H
