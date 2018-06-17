#ifndef REXUIZ_H
#define REXUIZ_H

#include <QString>

class Rexuiz
{
public:
	static const char *binary();
	static QStringList repos();
	static bool presentInDirectory(QString path);
};

#endif // REXUIZ_H
