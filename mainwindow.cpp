#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_dialogselectinstalllocation.h"
#include <QApplication>
#include <QString>
#include <QStyle>
#include <QDesktopWidget>
#include "launcher.h"
#include "dialogselectinstalllocation.h"
#include "ui_dialogselectinstalllocation.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	Q_INIT_RESOURCE(qrl);
	ui->setupUi(this);
	this->launcher = new Launcher();
	this->launcher->setMainWindow(this);
	this->launcher->start();
	this->dialogSelectInstallLocation = new DialogSelectInstallLocation();
	this->setWindowIcon(QIcon(":/images/icon.png"));
	this->dialogSelectInstallLocation->setWindowIcon(QIcon(":/images/icon.png"));
	this->setGeometry(
		QStyle::alignedRect(
			Qt::LeftToRight,
			Qt::AlignCenter,
			this->size(),
			QApplication::desktop()->availableGeometry()
		)
	);
}

void MainWindow::setStatus(const QString &str) {
	ui->labelStatus->setText(str);
}

void MainWindow::setProgress(qint64 done, qint64 total) {
	ui->progressBarTotal->setValue(done);
	if (ui->progressBarTotal->maximum() != total)
		ui->progressBarTotal->setMaximum(total);
}

void MainWindow::setSubStatus(const QString &str) {
	ui->labelSubStatus->setText(str);
}

void MainWindow::setSubProgress(qint64 done, qint64 total) {
	ui->progressBarDownload->setValue(done);
	if (ui->progressBarDownload->maximum() != total)
		ui->progressBarDownload->setMaximum(total);

}

bool MainWindow::askYesNo(const QString &title, const QString &question) {
	QMessageBox messageBox(QMessageBox::Question, title, question, QMessageBox::Yes|QMessageBox::No, this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Yes"));
	messageBox.setButtonText(QMessageBox::No, tr("No"));
	return QMessageBox::Yes == messageBox.exec();
}

QString MainWindow::askDirectory(const QString &title, const QString &defPath) {
	this->dialogSelectInstallLocation->setWindowTitle(title);
	this->dialogSelectInstallLocation->ui->lineEdit->setText(defPath);
	qDebug(("Set default path to" + defPath).toLocal8Bit());
	int code = dialogSelectInstallLocation->exec();
	if (code == QDialog::Accepted)
		return dialogSelectInstallLocation->ui->lineEdit->text();
	else
		return QString("");
}

void MainWindow::errorMessage(const QString &title, const QString &message) {
	QMessageBox::warning(this, title, message, QMessageBox::Ok);
}

MainWindow::~MainWindow()
{
	this->launcher->requestInterruption();
	this->launcher->wait();
	delete this->launcher;
    delete ui;
	delete dialogSelectInstallLocation;
}
