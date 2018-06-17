#include "launcher.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QSettings>
#include <QDir>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QProcess>
#ifdef USE_QUAZIP
#include <JlCompress.h>
#endif
#include "mainwindow.h"
#include "rexuiz.h"

Launcher::Launcher()
{

}

void Launcher::setMainWindow(MainWindow *w) {
	this->mainWindow = w;
}

bool Launcher::download(const QString &url, const QString &dest, qint64 expectedSize) {
	this->resetSubProgress(expectedSize);
	qDebug((QString("Downloading ") + url + " to " + dest).toLocal8Bit());
	if (!QFileInfo(dest).dir().mkpath(".")) {
		QMetaObject::invokeMethod(this->mainWindow, "errorMessage",
				Qt::BlockingQueuedConnection,
				Q_ARG(QString, "Error"),
				Q_ARG(QString, QString("Can't create path for: ") + dest));

		return false;
	}
	QFile outFile;
	outFile.setFileName(dest);
	if (!outFile.open(QIODevice::WriteOnly)) {
		QMetaObject::invokeMethod(this->mainWindow, "errorMessage",
				Qt::BlockingQueuedConnection,
				Q_ARG(QString, "Error"),
				Q_ARG(QString, QString("Can't open file for writing: ") + dest));

		return false;
	}
	QNetworkRequest req{QUrl(url)};
	req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	req.setHeader(QNetworkRequest::UserAgentHeader, "wget/1.19.5");
	QNetworkAccessManager nam;
	LauncherDownloadHandler dh;
	dh.outFile = &outFile;
	dh.launcher = this;
	connect(&nam, SIGNAL(finished(QNetworkReply*)), &dh, SLOT(finished(QNetworkReply*)));
	QNetworkReply *reply = nam.get(req);
	dh.reply = reply;
	connect(reply, SIGNAL(readyRead()), &dh, SLOT(readyRead()));
	QMetaObject::invokeMethod(this->mainWindow, "setSubStatus",
			Qt::BlockingQueuedConnection,
			Q_ARG(QString, tr("Downloading")));
	dh.loop.exec();
	bool success = (reply->error() == QNetworkReply::NoError);
	delete reply;
	outFile.flush();
	outFile.close();
	return success;
}

bool Launcher::downloadLauncherIndexItem(LauncherIndexItem *item, QVector<QString> *tempFiles) {
	QString url = this->selectedRepo + "/" + item->path;
	QString path = this->buildPath(item->path);
	if (QFile::exists(path) && QFile(path).size() == item->size && this->MD5Verify(path, item)) {
		this->setProgress(item->size);
		return true;
	}
#ifdef USE_QUAZIP
	if (!item->zipHash.isEmpty() &&
			!item->zipSourceUrl.isEmpty() &&
			!item->zipSubPath.isEmpty() && !item->zipTempName.isEmpty()) {
		qDebug("Trying zip source");
		this->resetSubProgress(item->zipSize);
		QString zipTempName = this->buildPath(item->zipTempName);
		LauncherIndexItem fakeItem;
		fakeItem.hash = item->zipHash;
		fakeItem.size = item->zipSize;
		fakeItem.path = zipTempName;
		if ((QFile::exists(zipTempName) && this->MD5Verify(zipTempName, &fakeItem)) ||
				(this->download(item->zipSourceUrl, zipTempName, item->zipSize) &&
				this->MD5Verify(zipTempName, &fakeItem))) {
			QMetaObject::invokeMethod(this->mainWindow, "setSubStatus",
					Qt::BlockingQueuedConnection,
					Q_ARG(QString, "Unpacking"));

			tempFiles->append(zipTempName);
			this->resetSubProgress(0);
			JlCompress::extractFile(zipTempName, item->zipSubPath, path);
			if (this->MD5Verify(path, item)) {
				this->setProgress(item->size);
				return true;
			}
		}
	}
#endif
	this->resetSubProgress(item->size);
	if (this->download(url, path, item->size)) {
		if (this->MD5Verify(path, item)) {
			this->setProgress(item->size);
			return true;
		} else {
			QMetaObject::invokeMethod(this->mainWindow, "errorMessage",
					Qt::BlockingQueuedConnection,
					Q_ARG(QString ,"Error"),
					Q_ARG(QString, QString("File hash sum failed: " + path)));
		}
	}
	return false;
}

void Launcher::launch() {
	this->resetProgress(1);
	this->resetSubProgress(1);
	this->setProgress(1);
	this->setSubProgress(1);
	QMetaObject::invokeMethod(mainWindow, "setSubStatus", Q_ARG(QString, ""));
	QMetaObject::invokeMethod(mainWindow, "setStatus", Q_ARG(QString, tr("Running")));
	QString binaryPath = this->buildPath(Rexuiz::binary());
	QFile::setPermissions(binaryPath,
			QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther | QFile::ExeOwner |
			QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther | QFile::ReadOwner |
			QFile::WriteOwner
			);

	QProcess process;
	process.start(binaryPath);
	process.waitForFinished(-1);
}

QString Launcher::buildPath(const QString &path) {
	QString fullPath = this->installPath;
	QStringList pathComponents = path.split("/");
	foreach(QString pathComponent, pathComponents) {
		fullPath = QDir(fullPath).filePath(pathComponent);
	}
	return fullPath;
}

bool Launcher::MD5Verify(const QString &path, LauncherIndexItem *item) {
	QFile f{path};
	QMetaObject::invokeMethod(mainWindow, "setSubStatus", Qt::BlockingQueuedConnection, Q_ARG(QString, "Verifying"));
	this->resetSubProgress(0);
	if (f.open(QFile::ReadOnly)) {
		QCryptographicHash hash{QCryptographicHash::Md5};
		if (hash.addData(&f)) {
			return QString(hash.result().toHex()).compare(item->hash) == 0;
		}
	}
	return false;
}

void Launcher::checkFiles(LauncherIndexHash *brokenFiles) {
	if (!this->index)
		return;

	for (auto i = this->index->begin(); i != this->index->end(); i++) {
		LauncherIndexItem *item = i.value();
		if (QFile(this->buildPath(item->path)).size() != item->size) {
			qDebug((QString("File ") + item->path + " broken!").toLocal8Bit());
			brokenFiles->insert(item->path, item);
		}
	}
}

void Launcher::run() {
	this->installPath = QDir(QCoreApplication::applicationDirPath()).absolutePath();
	bool updaterMode = false;
	if (Rexuiz::presentInDirectory(this->installPath)) {
		updaterMode = true;
	} else {
		this->installPath = QDir(this->installPath).filePath("Rexuiz");
	}
	bool isUpdate = false;
	QMetaObject::invokeMethod(mainWindow, "setStatus", Q_ARG(QString, tr("Preparing")));
	QMetaObject::invokeMethod(mainWindow, "setSubStatus", Q_ARG(QString, ""));
	QSettings settings("RexuizDev", "RexuizLauncher");
	bool reply;
	settings.beginGroup("main");
	if (!updaterMode)
		this->installPath = settings.value("install_path", this->installPath).toString();

	settings.endGroup();
	if (this->installPath.isEmpty() || (!QFile::exists(QDir(installPath).filePath("index.lst")) && !updaterMode)) {
		QMetaObject::invokeMethod(mainWindow, "askDirectory",
				Qt::BlockingQueuedConnection,
				Q_RETURN_ARG(QString, this->installPath),
				Q_ARG(QString, tr("Select install folder")),
				Q_ARG(QString, this->installPath));
	}
	if (this->installPath.isEmpty()) {
		QMetaObject::invokeMethod(this->mainWindow, "close", Qt::BlockingQueuedConnection);
		return;
	} else {
		settings.beginGroup("main");
		if (!updaterMode)
			settings.setValue("install_path", this->installPath);

		settings.endGroup();
	}
	this->index = this->loadIndex(QDir(installPath).filePath("index.lst"));
	if (index != nullptr) {
		isUpdate = true;
	}
	LauncherIndexHash brokenFiles;
	this->checkFiles(&brokenFiles);
	QString newIndexPath = QDir(installPath).filePath("index.lst.new");
	QMetaObject::invokeMethod(mainWindow, "setStatus", Qt::BlockingQueuedConnection, Q_ARG(QString, tr("Get update information")));
	QStringList repos = Rexuiz::repos();
	foreach (QString repo, repos) {
		if (download(repo + "/index.lst", newIndexPath)) {
			selectedRepo = repo;
			break;
		}
	}
	if (selectedRepo.isEmpty()) {
		if (!isUpdate) {
			QMetaObject::invokeMethod(this->mainWindow, "errorMessage", Qt::BlockingQueuedConnection, Q_ARG(QString ,"Error"), Q_ARG(QString, tr("Repositories unavailable")));
			QMetaObject::invokeMethod(this->mainWindow, "close", Qt::BlockingQueuedConnection);
			return;
		}
	} else {
		auto newIndex = Launcher::loadIndex(newIndexPath);
		if (newIndex && newIndex->begin() != newIndex->end()) {
			//We have new index, try update
			LauncherIndexHash newFiles; //new files, that we want update
			LauncherIndexHash oldFiles; //old files, that we want delete
			if (this->index) {
				for (auto i = newIndex->begin(); i != newIndex->end(); i++) {
					auto item = this->index->value(i.key(), nullptr);
					if (item == nullptr || item->hash.compare(i.value()->hash) != 0 || brokenFiles.value(i.key(), nullptr) != nullptr)
						newFiles.insert(i.key(), i.value());
				}
				for (auto i = this->index->begin(); i != this->index->end(); i++) {
					auto item = this->index->value(i.key(), nullptr);
					if (item == nullptr)
						oldFiles.insert(i.key(), i.value());
				}
			} else {
				for (auto i = newIndex->begin(); i != newIndex->end(); i++)
					newFiles.insert(i.key(), i.value());
			}
			if (newFiles.begin() != newFiles.end()) {
				qint64 expectedDataSize = 0;
				for (auto i = newFiles.begin(); i != newFiles.end(); i++) {
					expectedDataSize += i.value()->size;
				}
				QLocale locale;
				QMetaObject::invokeMethod(this->mainWindow, "askYesNo", Qt::BlockingQueuedConnection,
						Q_RETURN_ARG(bool, reply),
						Q_ARG(QString, tr("Confirmation")),
						Q_ARG(QString, tr("Update size is ") + locale.formattedDataSize(expectedDataSize) + tr(". Install it?")));

				if (reply) {
					this->resetProgress(expectedDataSize);
					QVector<QString> tempFiles;
					for (auto i = newFiles.begin(); i != newFiles.end(); i++) {
						QMetaObject::invokeMethod(mainWindow, "setStatus", Qt::BlockingQueuedConnection, Q_ARG(QString, tr("Updating")));
						if (!downloadLauncherIndexItem(i.value(), &tempFiles)) {
							if (this->isInterruptionRequested()) {
								Launcher::deleteIndex(newIndex);
								return;
							}

							QMetaObject::invokeMethod(this->mainWindow, "errorMessage",
									Qt::BlockingQueuedConnection,
									Q_ARG(QString ,tr("Error")),
									Q_ARG(QString, QString(tr("File download failed: ") + i.value()->path)));

							QMetaObject::invokeMethod(this->mainWindow, "close", Qt::BlockingQueuedConnection);
							return;
						}
					}
					for (auto i = tempFiles.begin(); i != tempFiles.end(); i++) {
						QFile::remove(*i);
					}
					for (auto i = oldFiles.begin(); i != oldFiles.end(); i++) {
						//QFile::remove(this->buildPath(i.value()->path));
					}
				}
			}
			Launcher::saveIndex(newIndex);
			Launcher::deleteIndex(newIndex);
			QFile::remove(newIndexPath);
		}
	}
	this->launch();
	QMetaObject::invokeMethod(this->mainWindow, "close", Qt::BlockingQueuedConnection);
}

LauncherIndexItem *Launcher::parseIndexLine(const char *line) {
	QStringList tokens = QString(line).split("|");
	if (tokens.length() < 3) {
		return nullptr;
	}
	auto *item = new LauncherIndexItem;
	item->hash = tokens[0];
	item->size = tokens[1].toLongLong();
	item->path = tokens[2];
	if (tokens.length() >= 8) {
		item->zipSourceUrl = tokens[3];
		item->zipTempName = tokens[4];
		item->zipSubPath = tokens[5];
		item->zipHash = tokens[6];
		item->zipSize = tokens[7].toLongLong();
	}
	return item;
}

void Launcher::saveIndex(LauncherIndexHash *index) {
	QFile file{this->buildPath("index.lst")};
	file.open(QFile::WriteOnly);
	for(auto i = index->begin(); i != index->end(); i++) {
		file.write(QString("%1|%2|%3\n").arg(i.value()->hash, QString::number(i.value()->size), i.value()->path).toLocal8Bit());
	}
	file.close();
}

void Launcher::deleteIndex(LauncherIndexHash *index) {
	for(auto i = index->begin(); i != index->end(); i++) {
		delete i.value();
	}
	delete index;
}

LauncherIndexHash *Launcher::loadIndex(const QString &path) {
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		return nullptr;

	auto index = new LauncherIndexHash;

	char line[2048];
	qint64 readed;
	while (!file.atEnd()) {
		readed = file.readLine(line, 2047);
		if (readed < 0) {
			QMetaObject::invokeMethod(this->mainWindow, "errorMessage",
					Qt::BlockingQueuedConnection,
					Q_ARG(QString , tr("Error")),
					Q_ARG(QString, QString(tr("Failed load index: ")) + path + tr("\nRead error: ") + file.errorString()));

			Launcher::deleteIndex(index);
			return nullptr;
		}
		if (readed > 0) {
			if (line[readed - 1] == '\n')
				line[readed - 1] = '\0';

			if (line[0] == '\0')
				continue;

			LauncherIndexItem *item = Launcher::parseIndexLine(line);
			if (item == nullptr) {
				QMetaObject::invokeMethod(this->mainWindow, "errorMessage",
						Qt::BlockingQueuedConnection,
						Q_ARG(QString, tr("Error")),
						Q_ARG(QString, QString(tr("Failed parse index: ")) + path + tr("\nLine: ") + line));

				Launcher::deleteIndex(index);
				return nullptr;
			}
			auto check = index->find(item->path);
			if (check != index->end()) {
				delete check.value();
				index->erase(check);
			}
			index->insert(item->path, item);
		}
	}
	return index;
}

Launcher::~Launcher() {
	if (this->index)
		Launcher::deleteIndex(this->index);

	foreach(QString *repo, repos) {
		delete repo;
	}
}

void Launcher::setSubProgress(qint64 writed) {
	if (this->subProgressTotal > 0) {
		this->subProgressDone += writed;
		QMetaObject::invokeMethod(this->mainWindow, "setSubProgress", Qt::BlockingQueuedConnection, Q_ARG(qint64, this->subProgressDone), Q_ARG(qint64, this->subProgressTotal));
	}
}

void Launcher::setProgress(qint64 writed) {
	if (this->progressTotal > 0) {
		this->progressDone += writed;
		QMetaObject::invokeMethod(this->mainWindow, "setProgress", Qt::BlockingQueuedConnection, Q_ARG(qint64, this->progressDone), Q_ARG(qint64, this->progressTotal));
	}

}

void Launcher::resetProgress(qint64 total) {
	this->progressTotal = total;
	this->progressDone = 0;
	QMetaObject::invokeMethod(this->mainWindow, "setProgress", Qt::BlockingQueuedConnection, Q_ARG(qint64, this->progressDone), Q_ARG(qint64, this->progressTotal));
}

void Launcher::resetSubProgress(qint64 total) {
	this->subProgressTotal = total;
	this->subProgressDone = 0;
	QMetaObject::invokeMethod(this->mainWindow, "setSubProgress", Qt::BlockingQueuedConnection, Q_ARG(qint64, this->subProgressDone), Q_ARG(qint64, this->subProgressTotal));
}
