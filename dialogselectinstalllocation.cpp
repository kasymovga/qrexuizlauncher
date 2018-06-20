#include <QFileDialog>
#include <QApplication>
#include <QDesktopWidget>
#include "dialogselectinstalllocation.h"
#include "ui_dialogselectinstalllocation.h"
#include "rexuiz.h"

DialogSelectInstallLocation::DialogSelectInstallLocation(QWidget *parent) :
    QDialog(parent)
{
	this->ui = new Ui::DialogSelectInstallLocation();
	this->ui->setupUi(this);
	this->ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	QObject::connect(this->ui->pushButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));
	this->setGeometry(
		QStyle::alignedRect(
			Qt::LeftToRight,
			Qt::AlignCenter,
			this->size(),
			QApplication::desktop()->availableGeometry()
		)
	);

}

void DialogSelectInstallLocation::selectDirectory() {
	QFileDialog fileDialog(this, tr("Select directory"));
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setLabelText(QFileDialog::Reject, tr("Cancel"));
	fileDialog.setLabelText(QFileDialog::Accept, tr("Select"));
	//QString dir = QFileDialog::getExistingDirectory(this, QString("Select directory"), this->ui->lineEdit->text(), QFileDialog::ShowDirsOnly);
	if (fileDialog.exec() == QDialog::Accepted) {
		QString dir = fileDialog.directory().absoluteFilePath(fileDialog.selectedFiles()[0]);
		if (dir.isEmpty())
			return;

		if (!Rexuiz::presentInDirectory(dir))
			dir = QDir(dir).filePath("Rexuiz");

		this->ui->lineEdit->setText(dir);
	}
}

DialogSelectInstallLocation::~DialogSelectInstallLocation() {
    delete ui;
}
