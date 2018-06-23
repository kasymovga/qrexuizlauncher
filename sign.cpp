#include "sign.h"
#include <mbedtls/error.h>
#include <mbedtls/md.h>
#include <mbedtls/md_internal.h>
#include <mbedtls/pk.h>
#include <mbedtls/platform.h>
#include <QFile>

int mbedtls_md_file_qt( const mbedtls_md_info_t *md_info, const QString &path, unsigned char *output )
{
	int ret;
	size_t n;
	mbedtls_md_context_t ctx;
	unsigned char buf[1024];
	QFile f(path);

	if( md_info == NULL )
		return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

	if (!f.open(QFile::ReadOnly))
		return( MBEDTLS_ERR_MD_FILE_IO_ERROR );

	mbedtls_md_init( &ctx );

	if( ( ret = mbedtls_md_setup( &ctx, md_info, 0 ) ) != 0 )
		goto cleanup;

	if( ( ret = md_info->starts_func( ctx.md_ctx ) ) != 0 )
		goto cleanup;

	while( ( n = f.read( (char *)buf, sizeof( buf )) ) > 0 )
		if( ( ret = md_info->update_func( ctx.md_ctx, buf, n ) ) != 0 )
			goto cleanup;

	if(f.error() != QFile::NoError)
		ret = MBEDTLS_ERR_MD_FILE_IO_ERROR;
	else
		ret = md_info->finish_func( ctx.md_ctx, output );

cleanup:
	f.close();
	mbedtls_md_free( &ctx );
	return( ret );
}

bool Sign::verify(const QString &path, const QString &signPath)
{
	bool r = false;
	mbedtls_pk_context pk;
	mbedtls_pk_init(&pk);
	unsigned char hash[32];
	QByteArray signData;
	QFile signFile(signPath);
	QFile pubKeyFile(":/cfg/rexuiz_pub.key");
	QByteArray keyData;
	if (!pubKeyFile.open(QFile::ReadOnly)) {
		qDebug("public key file open failed");
		goto finish;
	}
	keyData = pubKeyFile.readAll();
	keyData.append('\0');
	int error_code;
	if ((error_code = mbedtls_pk_parse_public_key(&pk, (const unsigned char *)keyData.data(), keyData.length())) != 0) {
		qDebug("mbedtls_pk_parse_public_key failed");
		qDebug(keyData.data());
		goto finish;
	}
	if (!signFile.open(QFile::ReadOnly)) {
		qDebug("sign file open failed");
		goto finish;
	}
	signData = signFile.readAll();
	if (mbedtls_md_file_qt(mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ), path, hash) != 0) {
		qDebug("mbedtls_md_file_qt failed");
		goto finish;
	}
	if (mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, 0, (const unsigned char *)signData.data(), signData.length()) != 0) {
		qDebug("mbedtls_pk_verify failed");
		goto finish;
	}
	r = true;
finish:
	mbedtls_pk_free(&pk);
	if (signFile.isOpen())
		signFile.close();

	if (pubKeyFile.isOpen())
		pubKeyFile.close();

	return r;
}
