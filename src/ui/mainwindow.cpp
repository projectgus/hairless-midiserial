#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialenumerator.h"
#include "Settings.h"
#include "settingsdialog.h"
#include "aboutdialog.h"

const int LIST_REFRESH_RATE =20; // Hz

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
    workerThread(NULL),
    debugListTimer(),
    debugListMessages()
{
    ui->setupUi(this);
    // Fixed width, minimum height
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size().width(), 2000);
#ifdef Q_OS_LINUX
    this->setWindowIcon(QIcon(":/images/images/icon48.png"));
#endif

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

    // Set up timer for the display list
    debugListTimer.setSingleShot(true);
    debugListTimer.setInterval(1000 / LIST_REFRESH_RATE);

    // Plumb signals & slots
    connect(ui->cmbMidiIn, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiOut, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbSerial, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->chk_on, SIGNAL(clicked()), SLOT(onValueChanged()));
    connect(ui->chk_debug, SIGNAL(clicked(bool)), SLOT(onDebugClicked(bool)));
    connect(&debugListTimer, SIGNAL(timeout()), SLOT(refreshDebugList()));


    // Menu items
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(showAboutBox()));
    connect(ui->actionPreferences, SIGNAL(triggered()), SLOT(showPreferences()));

    // Get started
    onValueChanged();

#ifdef Q_OS_MAC
    // hack: avoid an empty dummy File menu on OS X
    // there might be a better way to do this, but hide() and clear() don't work.
    ui->menuFile->setTitle(""); // Doesn't do anything on OS X
#endif

}


MainWindow::~MainWindow()
{
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
    catch (RtMidiError err) {
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
        QString label = it->friendName.isEmpty() ? it->portName : it->friendName;
#ifdef Q_OS_LINUX
        QString portName = it->physName; // Bug workaround, Linux needs the /dev/ in front of port name
#else
        QString portName = it->portName;
#endif
        ui->cmbSerial->addItem(label, QVariant(portName));
        if(current == label) {
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
    if(bridge) {
        bridge->deleteLater();
        QThread::yieldCurrentThread(); // Try and get any signals from the bridge sent sooner not later
        QCoreApplication::processEvents();
        bridge = NULL;
    }
    Settings::setLastMidiIn(ui->cmbMidiIn->currentText());
    Settings::setLastMidiOut(ui->cmbMidiOut->currentText());
    Settings::setLastSerialPort(ui->cmbSerial->currentText());
    if(!ui->chk_on->isChecked()
            || ( ui->cmbSerial->currentIndex() == 0
                    && ui->cmbMidiIn->currentIndex() == 0
                    && ui->cmbMidiOut->currentIndex() == 0 )) {
        return;
    }
    ui->lst_debug->clear();
    int midiIn =ui->cmbMidiIn->currentIndex()-1;
    int midiOut = ui->cmbMidiOut->currentIndex()-1;
    ui->lst_debug->addItem("Starting MIDI<->Serial Bridge...");
    bridge = new Bridge();
    connect(bridge, SIGNAL(debugMessage(QString)), SLOT(onDebugMessage(QString)));
    connect(bridge, SIGNAL(displayMessage(QString)), SLOT(onDisplayMessage(QString)));
    connect(bridge, SIGNAL(midiReceived()), ui->led_midiin, SLOT(blinkOn()));
    connect(bridge, SIGNAL(midiSent()), ui->led_midiout, SLOT(blinkOn()));
    connect(bridge, SIGNAL(serialTraffic()), ui->led_serial, SLOT(blinkOn()));
    bridge->attach(ui->cmbSerial->itemData(ui->cmbSerial->currentIndex()).toString(), Settings::getPortSettings(), midiIn, midiOut, workerThread);
}

void MainWindow::onDisplayMessage(QString message)
{
    if(debugListMessages.size() == scrollbackSize)
        debugListMessages.removeFirst();
    debugListMessages.append(message);
    if(!debugListTimer.isActive())
        debugListTimer.start();
}

void MainWindow::onDebugMessage(QString message)
{
    if(ui->chk_debug->isChecked())
        onDisplayMessage(message);
}

/*
 * When the timer (started in onDisplayMessage) fires, we update lst_debug with the
 * contents of debugListMessages.
 *
 * This happens in the timer event in order to rate limit it to a number of redraws per second
 * (redrawing, and especially scrolling the list view, can be quite resource intensive.)
 */
void MainWindow::refreshDebugList()
{
    QListWidget *lst = ui->lst_debug;
    while(lst->count() + debugListMessages.size() - scrollbackSize > 0 && lst->count() > 0) {
      delete lst->item(0);
    }
    lst->addItems(debugListMessages);
    debugListMessages.clear();
    lst->scrollToBottom();
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    QListWidget *lst = ui->lst_debug;
    QRect geo = lst->geometry();
    geo.setHeight( this->height() - geo.top() - 20 );
    lst->setGeometry(geo);
}

