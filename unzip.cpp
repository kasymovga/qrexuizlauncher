#include "unzip.h"
#include "miniz.h"
#ifndef Q_OS_WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

#include <QFile>

extern "C" {
static size_t write_cb(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n) {
	QFile *file = (QFile *)pOpaque;
	return file->write((const char*)pBuf, n);
}
}

bool UnZip::extractFile(const QString &path, const QString &zipPath, const QString &extractPath) {
	bool r = false;
	bool zip_archive_opened = false;
	QFile file(path);
	QFile extractFile(extractPath);
	FILE *cFile = NULL;
	int fileHandler;
	if (!file.open(QFile::ReadOnly))
		goto finish;

	if (!extractFile.open(QFile::WriteOnly))
		goto finish;

	mz_zip_archive zip_archive;
	memset(&zip_archive, 0, sizeof(zip_archive));
	fileHandler = file.handle();
	if (fileHandler < 0)
		goto finish;

#ifndef Q_OS_WIN32
	fileHandler = dup(fileHandler);
#else
	fileHandler = _dup(fileHandler);
#endif
	if (fileHandler < 0)
		goto finish;

	cFile = fdopen(fileHandler, "r");
	if (!(zip_archive_opened =
			mz_zip_reader_init_cfile(&zip_archive, cFile, file.size(), 0)))
		goto finish;

	if (!mz_zip_reader_extract_file_to_callback(&zip_archive, zipPath.toLatin1(), write_cb, (void *)&extractFile, 0))
		goto finish;

	r = true;
finish:
	if (zip_archive_opened)
		mz_zip_end(&zip_archive);

	if (file.isOpen())
		file.close();

	if (extractFile.isOpen())
		extractFile.close();

	if (cFile)
		fclose(cFile);

	return r;
}
