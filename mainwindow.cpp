#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialport/qextserialenumerator.h"
#include "src/Settings.h"
#include "src/settingsdialog.h"
#include "src/aboutdialog.h"
#include <QTimer>

const int LED_BLINKTIME= 75; // ms

const QString RES_LED_ON = ":/images/images/led-on.png";
const QString RES_LED_OFF = ":/images/images/led-off.png";

static void selectIfAvailable(QComboBox *box, QString itemText)
{
    for(int i = 0; i < box->count(); i++) {
        if(box->itemText(i) == itemText) {
            box->setCurrentIndex(i);
            return;
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bridge(NULL),
    workerThread(NULL)
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

    // Load initial state
    this->workerThread = new QThread();
    this->workerThread->start(QThread::HighestPriority);
    refresh();
    scrollbackSize=Settings::getScrollbackSize();
    ui->chk_debug->setChecked( Settings::getDebug() );
    selectIfAvailable(ui->cmbMidiIn, Settings::getLastMidiIn());
    selectIfAvailable(ui->cmbMidiOut, Settings::getLastMidiOut());
    selectIfAvailable(ui->cmbSerial, Settings::getLastSerialPort());

    // Plumb signals & slots
    connect(ui->cmbMidiIn, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiOut, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbSerial, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->chk_on, SIGNAL(clicked()), SLOT(onValueChanged()));
    connect(ui->chk_debug, SIGNAL(clicked(bool)), SLOT(onDebugClicked(bool)));

    // Menu items
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(showAboutBox()));
    connect(ui->actionPreferences, SIGNAL(triggered()), SLOT(showPreferences()));

    // Get started
    onValueChanged();
}

MainWindow::~MainWindow()
{
    activeLeds.clear();
    bridge->deleteLater();
    delete ui;
}

void MainWindow::showPreferences()
{
    SettingsDialog dialog;
    if(dialog.exec() == QDialog::Accepted) {
        scrollbackSize=Settings::getScrollbackSize();
        onValueChanged();
    }
}

void MainWindow::showAboutBox()
{
    AboutDialog().exec();
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
    try
    {
      int ports = midi->getPortCount();
      combo->addItem(TEXT_NOT_CONNECTED);
      for (int i = 0; i < ports; i++ ) {
        QString name = QString::fromStdString(midi->getPortName(i));
        combo->addItem(name);
            if(current == name) {
                combo->setCurrentIndex(combo->count() - 1);
           }
        }
    }
    catch (RtError err) {
        ui->lst_debug->addItem("Failed to scan for MIDI ports:");
        ui->lst_debug->addItem(QString::fromStdString(err.getMessage()));
        ui->lst_debug->scrollToBottom();
    }
}

void MainWindow::refreshSerial()
{
    QString current = ui->cmbSerial->currentText();
    ui->cmbSerial->clear();
    ui->cmbSerial->addItem(TEXT_NOT_CONNECTED);
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for(QList<QextPortInfo>::iterator it = ports.begin(); it != ports.end(); it++) {
#ifdef Q_OS_MAC
        QString label = it->portName; // OS X has no friendly name set
#else
        QString label = it->friendName;
#endif
#ifdef Q_OS_LINUX
        QString portName = it->physName; // Bug workaround, Linux needs the /dev/ in front of port name
#else
        QString portName = it->portName;
#endif

        ui->cmbSerial->addItem(label, QVariant(portName));
        if(current == it->friendName) {
            ui->cmbSerial->setCurrentIndex(ui->cmbSerial->count() - 1);
        }
    }
}

void MainWindow::onDebugClicked(bool value)
{
    Settings::setDebug(value);
}

void MainWindow::onValueChanged()
{
    bridge->deleteLater();
    QThread::yieldCurrentThread(); // Try and get any signals from the bridge sent sooner not later
    bridge = NULL;
    Settings::setLastMidiIn(ui->cmbMidiIn->currentText());
    Settings::setLastMidiOut(ui->cmbMidiOut->currentText());
    Settings::setLastSerialPort(ui->cmbSerial->currentText());
    if(!ui->chk_on->isChecked()
            || ( ui->cmbSerial->currentIndex() == 0
                    && ui->cmbMidiIn->currentIndex() == 0
                    && ui->cmbMidiOut->currentIndex() == 0 ))
        return; // No bridge
    ui->lst_debug->clear();
    int midiIn =ui->cmbMidiIn->currentIndex()-1;
    int midiOut = ui->cmbMidiOut->currentIndex()-1;
    ui->lst_debug->addItem("Starting MIDI<->Serial Bridge...");
    bridge = new Bridge(this);
    connect(bridge, SIGNAL(debugMessage(QString)), SLOT(onDebugMessage(QString)));
    connect(bridge, SIGNAL(displayMessage(QString)), SLOT(onDisplayMessage(QString)));
    connect(bridge, SIGNAL(midiReceived()), SLOT(onMidiReceived()));
    connect(bridge, SIGNAL(midiSent()), SLOT(onMidiSent()));
    connect(bridge, SIGNAL(serialTraffic()), SLOT(onSerialTraffic()));
    bridge->attach(ui->cmbSerial->itemData(ui->cmbSerial->currentIndex()).toString(), Settings::getPortSettings(), midiIn, midiOut, workerThread);
}

void MainWindow::onDisplayMessage(QString message)
{
    QListWidget *lst = ui->lst_debug;
    if(lst->count() > scrollbackSize) {
        lst->takeItem(0);
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

