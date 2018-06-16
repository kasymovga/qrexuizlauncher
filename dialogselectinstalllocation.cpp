#include <QFileDialog>
#include "dialogselectinstalllocation.h"
#include "ui_dialogselectinstalllocation.h"

DialogSelectInstallLocation::DialogSelectInstallLocation(QWidget *parent) :
    QDialog(parent)
{
        this->ui = new Ui::DialogSelectInstallLocation();
        this->ui->setupUi(this);
        QObject::connect(this->ui->pushButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));

}

void DialogSelectInstallLocation::selectDirectory() {
    this->ui->lineEdit->setText(QDir(
            QFileDialog::getExistingDirectory(this, QString("Select directory"), QString(""), QFileDialog::ShowDirsOnly)).filePath("Rexuiz"));
}

DialogSelectInstallLocation::~DialogSelectInstallLocation() {
    delete ui;
}
