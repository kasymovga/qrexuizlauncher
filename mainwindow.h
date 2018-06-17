#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QString>

class Launcher;
class DialogSelectInstallLocation;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	Launcher *launcher;
public:
	explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	DialogSelectInstallLocation *dialogSelectInstallLocation;
    Ui::MainWindow *ui;
public slots:
	void setProgress(qint64 done, qint64 total);
	void setSubProgress(qint64 done, qint64 total);
	void setStatus(const QString &str);
	void setSubStatus(const QString &str);
	QString askDirectory(const QString &title, const QString &defPath);
	void errorMessage(const QString &title, const QString &message);
	bool askYesNo(const QString &title, const QString &question);
};

#endif // MAINWINDOW_H
