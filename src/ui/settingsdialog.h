#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QTWidgets/QDialog>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private:
    Ui::SettingsDialog *ui;

public slots:
    virtual void accept();
private slots:
    void resetSerialPort();
};

#endif // SETTINGSDIALOG_H
