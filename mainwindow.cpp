#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialport/qextserialenumerator.h"

const int SCROLLBACK_LINES = 25;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bridge(NULL)
{
    ui->setupUi(this);
    // Fixed size
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size());

    // Plumb event filter for focus events
    ui->cmbMidiIn->installEventFilter(this);
    ui->cmbMidiOut->installEventFilter(this);
    ui->cmbSerial->installEventFilter(this);

    // Plumb signals & slots
    connect(ui->cmbMidiIn, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiOut, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbSerial, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->chk_on, SIGNAL(clicked()), SLOT(onValueChanged()));

    refresh();
}

MainWindow::~MainWindow()
{
    delete bridge;
    delete ui;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event) {
    // If user is triggering a dropdown, refresh it live
    if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyPress) {
        if(object == ui->cmbMidiIn) {
            RtMidiIn in;
            refreshMidi(ui->cmbMidiIn, &in);
        } else if (object == ui->cmbMidiOut) {
            RtMidiOut out;
            refreshMidi(ui->cmbMidiOut, &out);
        }
        else if (object == ui->cmbSerial) {
            refreshSerial();
        }
    }
    return false;
}

void MainWindow::refresh()
{
    refreshSerial();
    refreshMidiIn();
    refreshMidiOut();
}

void MainWindow::refreshMidiIn()
{
    RtMidiIn in = RtMidiIn();
    refreshMidi(ui->cmbMidiIn, &in);
}

void MainWindow::refreshMidiOut()
{
    RtMidiOut out = RtMidiOut();
    refreshMidi(ui->cmbMidiOut, &out);
}


void MainWindow::refreshMidi(QComboBox *combo, RtMidi *midi)
{
    QString current = combo->currentText();
    combo->clear();
    try {
      int ports = midi->getPortCount();
      combo->addItem(NOT_CONNECTED);
      for (int i = 0; i < ports; i++ ) {
        QString name = QString::fromStdString(midi->getPortName(i));
        combo->addItem(name);
        if(current == name) {
            combo->setCurrentIndex(combo->count() - 1);
        }
      }
    } catch (RtError& err) {
        ui->lst_debug->addItem("Failed to scan for MIDI ports:");
        ui->lst_debug->addItem(QString::fromStdString(err.getMessage()));
    }
}

void MainWindow::refreshSerial()
{
    QString current = ui->cmbSerial->currentText();
    ui->cmbSerial->clear();
    ui->cmbSerial->addItem(NOT_CONNECTED);
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for(QList<QextPortInfo>::iterator it = ports.begin(); it != ports.end(); it++) {
        ui->cmbSerial->addItem(it->friendName, QVariant(it->physName));
        if(current == it->friendName) {
            ui->cmbSerial->setCurrentIndex(ui->cmbSerial->count() - 1);
        }
    }
}

void MainWindow::onValueChanged()
{
    delete bridge;
    bridge = NULL;
    if(!ui->chk_on->isChecked())
        return; // No bridge
    int midiIn =ui->cmbMidiIn->currentIndex()-1;
    int midiOut = ui->cmbMidiOut->currentIndex()-1;
    PortSettings settings;
    settings.BaudRate = BAUD115200;
    settings.DataBits = DATA_8;
    settings.FlowControl = FLOW_OFF;
    settings.Parity = PAR_NONE;
    settings.StopBits = STOP_1;
    settings.Timeout_Millisec = 1;
    bridge = new Bridge(this, ui->cmbSerial->itemData(ui->cmbSerial->currentIndex()).toString(), settings, midiIn, midiOut);

    connect(bridge, SIGNAL(debugMessage(QString)), SLOT(onDebugMessage(QString)));
    connect(bridge, SIGNAL(displayMessage(QString)), SLOT(onDisplayMessage(QString)));
    connect(bridge, SIGNAL(midiReceived()), SLOT(onMidiReceived()));
    connect(bridge, SIGNAL(midiSent()), SLOT(onMidiSent()));
    connect(bridge, SIGNAL(serialTraffic()), SLOT(onSerialTraffic()));
}

void MainWindow::onDisplayMessage(QString message)
{
    if(ui->lst_debug->count() > SCROLLBACK_LINES) {
            printf("COUNT %d", ui->lst_debug->count());
        ui->lst_debug->removeItemWidget(ui->lst_debug->item(ui->lst_debug->count()-1));
    }
    ui->lst_debug->addItem(message);
    ui->lst_debug->scrollToBottom();
}

void MainWindow::onDebugMessage(QString message)
{
    if(ui->chk_debug->isChecked())
        onDisplayMessage(message);
}

void MainWindow::onMidiReceived()
{
}

void MainWindow::onMidiSent()
{
}

void MainWindow::onSerialTraffic()
{
}
