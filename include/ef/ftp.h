#ifndef __EF_FTP_H__
#define __EF_FTP_H__

#include <ef/type.h>
#include <ef/www.h>
#include <ef/vector.h>

#define FTP_FTYPE_REG 0
#define FTP_FTYPE_LINK 1
#define FTP_FTYPE_DIR 2

typedef struct ftpStat{
	char* name;
	char* link;
	size_t size;
	time_t time;
	int mode;
}ftpStat_s;

typedef struct ftp{
	www_s www;
	char* site;
	size_t lenSite;
}ftp_s;

ftp_s* ftp_new(size_t bufferupsize, int flags, wwwProgress_s* prog);

void ftp_free(ftp_s* ftp);

void ftp_user_password(ftp_s* ftp, const char* user, const char* pass);

err_t ftp_site(ftp_s* ftp, const char* url);

void ftp_cd(ftp_s* ftp, const char* path);

ftpStat_s* ftp_list(ftp_s* ftp);

void ftp_stat_free(void* v);

err_t ftp_rename(ftp_s* ftp, const char* from, const char* to);

err_t ftp_unlink(ftp_s* ftp, const char* fname);

err_t ftp_mkdir(ftp_s* ftp, const char* name);

err_t ftp_rmdir(ftp_s* ftp, const char* name);


#endif // EASYHTTP_H_INCLUDED
