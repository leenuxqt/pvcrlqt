#include "slaveconfdialog.h"
#include "ui_slaveconfdialog.h"

SlaveConfDialog::SlaveConfDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SlaveConfDialog)
{
    ui->setupUi(this);
}

SlaveConfDialog::~SlaveConfDialog()
{
    delete ui;
}
