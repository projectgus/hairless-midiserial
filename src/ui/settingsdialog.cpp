#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "Settings.h"

static void selectByData(QComboBox *box, int dataValue) {
  for(int i = 0; i < box->count(); i++) {
      if(box->itemData(i) == dataValue) {
          box->setCurrentIndex(i);
          return;
      }
  }
}

static void populateBaudRate(QComboBox *box, BaudRateType initialValue)
{
#ifdef Q_OS_UNIX
    box->addItem("50", BAUD50);
    box->addItem("75", BAUD75);
#endif
    box->addItem("110", BAUD110);
#ifdef Q_OS_UNIX
    box->addItem("134", BAUD134);
    box->addItem("150", BAUD150);
    box->addItem("200", BAUD200);
#endif
    box->addItem("300", BAUD300);
    box->addItem("600", BAUD600);
    box->addItem("1200", BAUD1200);
#ifdef Q_OS_UNIX
    box->addItem("1800", BAUD1800);
#endif
    box->addItem("2400", BAUD2400);
    box->addItem("4800", BAUD4800);
    box->addItem("9600", BAUD9600);
#ifdef Q_OS_WIN32
    box->addItem("14400", BAUD14400);
#endif
    box->addItem("19200", BAUD19200);
    box->addItem("38400", BAUD38400);
#ifdef Q_OS_WIN32
    box->addItem("56000", BAUD56000);
#endif
    box->addItem("57600", BAUD57600);
    box->addItem("115200", BAUD115200);
#ifdef Q_OS_WIN32
    box->addItem("128000", BAUD128000);
    box->addItem("256000", BAUD256000);
#endif
    selectByData(box, initialValue);
}

static void populateDataBits(QComboBox *box, DataBitsType initialValue)
{
    box->addItem("5", DATA_5);
    box->addItem("6", DATA_6);
    box->addItem("7", DATA_7);
    box->addItem("8", DATA_8);
    selectByData(box, initialValue);
}

static void populateParity(QComboBox *box, ParityType initialValue)
{
  box->addItem("None", PAR_NONE);
  box->addItem("Odd", PAR_ODD);
  box->addItem("Even", PAR_EVEN);
#ifdef Q_OS_WIN32
  box->addItem("Mark", PAR_MARK);
#endif
  box->addItem("Space", PAR_SPACE);
  selectByData(box, initialValue);
}

static void populateStopBits(QComboBox *box, StopBitsType initialValue)
{
  box->addItem("1", STOP_1);
#ifdef Q_OS_WIN32
  box->addItem("1 1/2", STOP_1_5);
#endif
  box->addItem("2", STOP_2);
  selectByData(box, initialValue);
}

static void populateFlowControl(QComboBox *box, FlowType initialValue)
{
  box->addItem("None", FLOW_OFF);
  box->addItem("Hardware", FLOW_HARDWARE);
  box->addItem("XON/XOFF", FLOW_XONXOFF);
  selectByData(box, initialValue);
}


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->spn_scrollback->setValue( Settings::getScrollbackSize() );

    PortSettings port = Settings::getPortSettings();
    populateBaudRate(ui->cmb_baud, port.BaudRate);
    populateDataBits(ui->cmb_dataBits, port.DataBits);
    populateParity(ui->cmb_parity, port.Parity);
    populateStopBits(ui->cmb_stop, port.StopBits);
    populateFlowControl(ui->cmb_flow, port.FlowControl);

    connect(ui->btn_defaults, SIGNAL(clicked()), SLOT(resetSerialPort()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

static int currentData(QComboBox *box) {
    return box->itemData(box->currentIndex()).toInt();
}

void SettingsDialog::accept()
{
    Settings::setScrollbackSize(ui->spn_scrollback->value());
    PortSettings port;
    port.BaudRate = (BaudRateType) currentData( ui->cmb_baud );
    port.DataBits = (DataBitsType) currentData( ui->cmb_dataBits );
    port.Parity = (ParityType) currentData( ui->cmb_parity );
    port.StopBits = (StopBitsType) currentData( ui->cmb_stop );
    port.FlowControl = (FlowType) currentData( ui->cmb_flow );
    Settings::setPortSettings(port);
    QDialog::accept();
}

void SettingsDialog::resetSerialPort()
{
    selectByData(ui->cmb_baud, BAUD115200);
    selectByData(ui->cmb_dataBits, DATA_8);
    selectByData(ui->cmb_parity, PAR_NONE);
    selectByData(ui->cmb_stop, STOP_1);
    selectByData(ui->cmb_flow, FLOW_OFF);
}
