#include <QFileDialog>
#include "dialogselectinstalllocation.h"
#include "ui_dialogselectinstalllocation.h"
#include "rexuiz.h"

DialogSelectInstallLocation::DialogSelectInstallLocation(QWidget *parent) :
    QDialog(parent)
{
        this->ui = new Ui::DialogSelectInstallLocation();
        this->ui->setupUi(this);
        QObject::connect(this->ui->pushButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));

}

void DialogSelectInstallLocation::selectDirectory() {
	QString dir = QFileDialog::getExistingDirectory(this, QString("Select directory"), this->ui->lineEdit->text(), QFileDialog::ShowDirsOnly);
	if (dir.isEmpty())
		return;

	if (!Rexuiz::presentInDirectory(dir))
		dir = QDir(dir).filePath("Rexuiz");

	this->ui->lineEdit->setText(dir);
}

DialogSelectInstallLocation::~DialogSelectInstallLocation() {
    delete ui;
}
