#ifndef SIGN_H
#define SIGN_H
#include <QString>

class Sign
{
public:
	static bool verify(const QString &path, const QString &signPath);
};

#endif // SIGN_H
