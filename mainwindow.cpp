#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialport/qextserialenumerator.h"
#include <QTimer>

const int SCROLLBACK_LINES = 250;
const int LED_BLINKTIME= 100; // ms

const QString RES_LED_ON = ":/images/images/led-on.png";
const QString RES_LED_OFF = ":/images/images/led-off.png";

const QString TEXT_NOSELECTION = "";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bridge(NULL)
{
    ui->setupUi(this);
    // Fixed width, minimum height
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size().width(), 2000);

    // Load LED images
    this->pxLedOn = QPixmap::fromImage(QImage(RES_LED_ON));
    this->pxLedOff = QPixmap::fromImage(QImage(RES_LED_OFF));

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
    activeLeds.clear();
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
      combo->addItem(TEXT_NOT_CONNECTED, IDX_NOT_CONNECTED);
      combo->setCurrentIndex(0);
#ifdef ALLOW_VIRTUAL_MIDI
      combo->addItem(TEXT_NEW_PORT, IDX_NEW_PORT);
      combo->setCurrentIndex(1);
#endif
      for (int i = 0; i < ports; i++ ) {
        QString name = QString::fromStdString(midi->getPortName(i));
        if(name.length() && !name.startsWith(NAME_MIDI_IN) && !name.startsWith(NAME_MIDI_OUT))
        {
            combo->addItem(name, i);
            if(current == name) {
                combo->setCurrentIndex(combo->count() - 1);
           }
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
    ui->cmbSerial->addItem(TEXT_NOT_CONNECTED);
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for(QList<QextPortInfo>::iterator it = ports.begin(); it != ports.end(); it++) {
        ui->cmbSerial->addItem(it->friendName, QVariant(it->physName));
        if(current == it->friendName) {
            ui->cmbSerial->setCurrentIndex(ui->cmbSerial->count() - 1);
        }
    }
}

static QVariant currentData(QComboBox *widget)
{
    return widget->itemData(widget->currentIndex());
}

void MainWindow::onValueChanged()
{
    delete bridge;
    bridge = NULL;
    ui->lst_debug->clear();
    if(!ui->chk_on->isChecked()
            || ( ui->cmbSerial->currentIndex() == 0
                    && ui->cmbMidiIn->currentIndex() == 0
                    && ui->cmbMidiOut->currentIndex() == 0 ))
        return; // No bridge
    PortSettings settings;
    settings.BaudRate = BAUD115200;
    settings.DataBits = DATA_8;
    settings.FlowControl = FLOW_OFF;
    settings.Parity = PAR_NONE;
    settings.StopBits = STOP_1;
    settings.Timeout_Millisec = 1;
    ui->lst_debug->addItem("Starting MIDI<->Serial Bridge...");
    bridge = new Bridge(this);
    connect(bridge, SIGNAL(debugMessage(QString)), SLOT(onDebugMessage(QString)));
    connect(bridge, SIGNAL(displayMessage(QString)), SLOT(onDisplayMessage(QString)));
    connect(bridge, SIGNAL(midiReceived()), SLOT(onMidiReceived()));
    connect(bridge, SIGNAL(midiSent()), SLOT(onMidiSent()));
    connect(bridge, SIGNAL(serialTraffic()), SLOT(onSerialTraffic()));
    bridge->attach(currentData(ui->cmbSerial).toString(), settings, currentData(ui->cmbMidiIn).toInt(), currentData(ui->cmbMidiOut).toInt());
}

void MainWindow::onDisplayMessage(QString message)
{
    QListWidget *lst = ui->lst_debug;
    if(lst->count() > SCROLLBACK_LINES) {
        lst->removeItemWidget(lst->item(lst->count()-1));
    }
    lst->addItem(message);
    lst->scrollToBottom();
}

void MainWindow::onDebugMessage(QString message)
{
    if(ui->chk_debug->isChecked())
        onDisplayMessage(message);
}

void MainWindow::onMidiReceived()
{
    ledOn(ui->led_midiin);
}

void MainWindow::onMidiSent()
{
    ledOn(ui->led_midiout);
}

void MainWindow::onSerialTraffic()
{
    ledOn(ui->led_serial);
}

void MainWindow::ledOn(QLabel *led)
{
    led->setPixmap(pxLedOn);
    activeLeds.push_back(led);
    QTimer::singleShot(LED_BLINKTIME, this, SLOT(ledOffTimer()));
}

void MainWindow::ledOffTimer()
{
    if(activeLeds.empty())
        return;
    QLabel *led = activeLeds.front();
    activeLeds.pop_front();
    if(!activeLeds.contains(led))
        led->setPixmap(pxLedOff);
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    QListWidget *lst = ui->lst_debug;
    QRect geo = lst->geometry();
    geo.setHeight( this->height() - geo.top() - 20 );
    lst->setGeometry(geo);
}

