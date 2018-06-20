#ifndef UNZIP_H
#define UNZIP_H

#include <QString>

class UnZip
{
public:
	static bool extractFile(const QString &path, const QString &zipPath, const QString &extractPath);
};

#endif // UNZIP_H
