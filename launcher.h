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
	MainWindow *mainWindow = nullptr;
	QVector<QString *> repos;
	QString selectedRepo;
	LauncherIndexHash *index = nullptr;
	static LauncherIndexItem *parseIndexLine(const char *line);
	bool download(const QString &url, const QString &dest, qint64 expectedSize = 0);
	bool downloadLauncherIndexItem(LauncherIndexItem *item, QVector<QString> *tempFiles);
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
	QEventLoop loop;
	QFile *outFile;
	QNetworkReply *reply;
	Launcher *launcher;
public slots:
	void finished(QNetworkReply *_reply) {
		launcher->setSubProgress(outFile->write(_reply->readAll()));
		QMetaObject::invokeMethod(&loop, "quit");
	}
	void readyRead() {
		if (launcher->isInterruptionRequested())
			reply->abort();
		else {
			launcher->setSubProgress(outFile->write(reply->readAll()));
		}
	}
};

#endif // LAUNCHER_H
