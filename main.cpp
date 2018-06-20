#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
	QTranslator translator;
	QApplication a(argc, argv);
	QString translationPath = QString(":/translations/RexuizLauncher_") + QLocale::system().name().left(2) + ".qm";
	qDebug(translationPath.toLocal8Bit());
	if (translator.load(translationPath))
		qDebug("Translations loaded");

	a.installTranslator(&translator);
	MainWindow w;
    w.show();
    return a.exec();
}
