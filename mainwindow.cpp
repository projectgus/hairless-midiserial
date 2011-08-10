#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialport/qextserialenumerator.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Fixed size
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size());

    m_midiout = new RtMidiOut();
    m_midiin = new RtMidiIn();
    refresh();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_midiout;
    delete m_midiin;
}

void MainWindow::refresh()
{
    refreshMidi(ui->cmbMidiIn, m_midiin);
    refreshMidi(ui->cmbMidiOut, m_midiout);
    refreshSerial();
}

void MainWindow::refreshMidi(QComboBox *combo, RtMidi *midi)
{
  try {
      combo->clear();
      int ports = midi->getPortCount();
      combo->addItem("(Not Connected)");
      for (int i = 0; i < ports; i++ ) {
        QString name = QString::fromStdString(midi->getPortName(i));
        combo->addItem(name);
      }
    } catch (RtError& err) {
        ui->lst_debug->addItem("Failed to scan for MIDI ports:");
        ui->lst_debug->addItem(QString::fromStdString(err.getMessage()));
    }
}

void MainWindow::refreshSerial()
{
    ui->cmbSerial->addItem("(Not Connected)");
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for(QList<QextPortInfo>::iterator it = ports.begin(); it != ports.end(); it++) {
        ui->cmbSerial->addItem(it->friendName);
    }
}
