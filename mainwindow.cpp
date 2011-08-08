#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->refreshOutputs();
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::refreshOutputs()
{
  try {
      ui->cmbMidiOutput->clear();
      ports = m_midiout->getPortCount();
      for (int i = 0; i < ports; i++ ) {
        QString name = QString::fromStdString(m_midiout->getPortName(i));
        if (!name.startsWith(QSTR_VMPK))
            dlgMidiSetup()->addOutputPortName(name, i);
        }
    } catch (RtError& err) {
        ui->statusBar->showMessage(QString::fromStdString(err.getMessage()));
    }
}
