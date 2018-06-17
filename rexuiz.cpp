#include "rexuiz.h"
#include <QString>
#include <QFile>
#include <QDir>
#include <QSysInfo>

const char *Rexuiz::binary() {
	bool is64bit = false;
	if (QSysInfo::currentCpuArchitecture().compare("x86_64") == 0)
		is64bit = true;

#ifdef Q_OS_LINUX
	return is64bit ? "rexuiz-linux-sdl-x86_64" : "rexuiz-linux-sdl-i686";
#else
#ifdef Q_OS_WIN32
	retrun is64bit ? "rexuiz-x86_64.exe" : "rexuiz-i686.exe";
#else
#ifdef Q_OS_MACOS
	return "Rexuiz.app/Contents/MacOS/rexuiz-dprm-sdl";
#endif
#endif
#endif
	return "unknown";
}

QStringList Rexuiz::repos() {
	QStringList list;
	list.append(QString("http://bnse.rexuiz.top/RexuizLauncher"));
	list.append(QString("http://rexuiz.top/maps/RexuizLauncher"));
	list.append(QString("http://nexuiz.mooo.com/RexuizLauncher"));
	list.append(QString("http://108.61.164.188/RexuizLauncher"));
	list.append(QString("http://104.238.159.167/RexuizLauncher"));
	return list;
}

bool Rexuiz::presentInDirectory(QString path) {
	if (QFile(QDir(path).filePath("index.lst")).exists())
		return true;

	if (QFile(QDir(path).filePath(Rexuiz::binary())).exists())
		return true;

	return false;
}
