#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QWidget>
#include <QThread>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#define LAUNCHERVERSION 20210121

class MainWindow;

struct LauncherIndexItem {
	QString path;
	QString hash;
	qint64 size;
	QString zipTempName;
	QString zipHash;
	QString zipSourceUrl;
	QString zipSubPath;
	qint64 zipSize;
};

typedef QHash<QString, LauncherIndexItem *> LauncherIndexHash;

class Launcher : public QThread
{
	Q_OBJECT
	QString installPath;
	QString launcherUpdateLink;
	MainWindow *mainWindow = nullptr;
	QVector<QString *> repos;
	QString selectedRepo;
	LauncherIndexHash *index = nullptr;
	static LauncherIndexItem *parseIndexLine(const char *line);
	bool download(const QString &url, const QString &dest, qint64 expectedSize = 0, int timeout = 0);
	bool downloadLauncherIndexItem(LauncherIndexItem *item, QVector<QString> *tempFiles);
	bool checkNewVersion();
	LauncherIndexHash *loadIndex(const QString &path);
	static void deleteIndex(LauncherIndexHash *index);
	void saveIndex(LauncherIndexHash *index);
	void launch();
	bool MD5Verify(const QString &path, LauncherIndexItem *item);
	QString buildPath(const QString &path);
	qint64 progressDone = 0, progressTotal = 0, subProgressDone = 0, subProgressTotal = 0;
	bool progressCount = true;
	void resetProgress(qint64 total);
	void resetSubProgress(qint64 total);
	void checkFiles(LauncherIndexHash *brokenFiles);
	void selectRepo(const QString &newIndexPath);
	bool update(const QString &newIndexPath, LauncherIndexHash *brokenFiles);
	bool updateHappened;
public:
    Launcher();
	~Launcher();
	void setProgress(qint64 writed);
	void setSubProgress(qint64 writed);
	void run();
	void setMainWindow(MainWindow *w);
public slots:
};

class LauncherDownloadHandler : public QObject {
	Q_OBJECT
public:
	QFile *outFile;
	QNetworkReply *reply;
	Launcher *launcher;
	bool failed = false;
public slots:
	void finished(QNetworkReply *_reply) {
		launcher->setSubProgress(outFile->write(_reply->readAll()));
		QMetaObject::invokeMethod(launcher, "quit");
	}
	void timeout() {
		failed = true;
		QMetaObject::invokeMethod(reply, "abort");
	}
	void readyRead() {
		if (launcher->isInterruptionRequested()) {
			QMetaObject::invokeMethod(reply, "abort");
			failed = true;
		} else {
			launcher->setSubProgress(outFile->write(reply->readAll()));
		}
	}
};

#endif // LAUNCHER_H
