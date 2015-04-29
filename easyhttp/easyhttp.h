#ifndef EASYHTTP_H_INCLUDED
#define EASYHTTP_H_INCLUDED

//FARE UN TIMEOTU
//EMAIL

#include <easytype.h>

#define FTP_FTYPE_REG 0
#define FTP_FTYPE_LINK 1
#define FTP_FTYPE_DIR 2

#define EMA_FTYPE_DIR 0

#define HTP_URL_MAX 2048
#define HTP_SUBJECT_MAX 256
#define HTP_DATE_MAX 64
#define HTP_BUFFER 512
#define FTP_NAME_MAX 128

typedef VOID(*HTPPROG)(VOID* data, FLOAT64 c, FLOAT64 t, FLOAT64 sp);

typedef struct _HTPBUFFER
{
	CHAR* data;
	CHAR* pdata;
	UINT32 size;
	UINT32 rem;
}HTPBUF;

typedef struct _HTPDATA
{
	CHAR* url;
	HTPBUF header;
	HTPBUF body;
	HTPPROG up;
	HTPPROG down;
}HTPDATA;

typedef struct _FTPLIST
{
	UINT32 rfile;
	CHAR pri[10];
	INT32 ftype;
	UINT32 size;
	CHAR name[FTP_NAME_MAX];
	CHAR link[FTP_NAME_MAX];
	CHAR date[25];
	HTPDATA* d;
}FTPLIST;

typedef struct _EMALIST
{
	UINT32 rfile;
	INT32 ftype;
	CHAR name[FTP_NAME_MAX];
	HTPDATA* d;
}EMALIST;

typedef struct _EEMAIL
{
	UINT32 rfile;
	UINT32 idf;
	CHAR* from;
	CHAR* to;
	CHAR* date;
	CHAR* subject;
	CHAR* messageid;
	CHAR* part;
	CHAR* body;
	HTPDATA* d;
}EEMAIL;


typedef struct _EMALINFO
{
	UINT32 count;
	UINT32 recent;
	HTPDATA* d;
}EMALINFO;
	
typedef struct __HTPDOWNUP* HTPDOWNUP;
typedef struct __HTPFORM* HTPFORM;

VOID htp_init();
VOID htp_terminate();
INT32 htp_errno();
const CHAR* htp_errstr();
BOOL url_walk(CHAR* url, CHAR* walk);

///for php ?filed=value&field=value
HTPDATA* htp_get(CHAR* url, BOOL ssl, HTPPROG scbk, HTPPROG rcbk);
///postexample "field=value&field=value"
HTPDATA* htp_post(CHAR* url, CHAR* post, UINT32 sz, BOOL ssl, HTPPROG scbk, HTPPROG rcbk);
VOID htp_data_free(HTPDATA* d);

HTPDOWNUP htp_download(CHAR* fname, CHAR* url, BOOL resume, BOOL ssl, HTPPROG rcbk);
VOID htp_downup_pause(HTPDOWNUP h);
VOID htp_downup_resume(HTPDOWNUP h);
INT32 htp_downup_complete(HTPDOWNUP h, BOOL waitd);
CHAR* htp_downup_url(HTPDOWNUP h);
CHAR* htp_downup_file(HTPDOWNUP h);
VOID htp_downup_free(HTPDOWNUP h);

HTPFORM htp_form_new(CHAR* action, BOOL ssl, HTPPROG scbk, HTPPROG rcbk);
VOID htp_form_add(HTPFORM h, CHAR* namefield, CHAR* value, BOOL isfile);
HTPDATA* htp_form(HTPFORM h);
VOID htp_form_free(HTPFORM h);

FTPLIST* ftp_list(CHAR* url, CHAR* user, CHAR* psw, BOOL ssl, HTPPROG scbk, HTPPROG rcbk);
VOID ftp_list_free(FTPLIST* fl);
HTPDOWNUP ftp_download(CHAR* fname, CHAR* url, CHAR* user, CHAR* psw, BOOL resume, BOOL ssl, HTPPROG rcbk);
HTPDOWNUP ftp_upload(CHAR* url, CHAR* user, CHAR* psw, CHAR* fname, BOOL ssl, HTPPROG rcbk);
INT32 ftp_rename(CHAR* url, CHAR* from, CHAR* to, CHAR* user, CHAR* psw, BOOL ssl);
INT32 ftp_delete(CHAR* url, CHAR* fname, CHAR* user, CHAR* psw, BOOL ssl);
INT32 ftp_mkdir(CHAR* url, CHAR* dname, CHAR* user, CHAR* psw, BOOL ssl);
INT32 ftp_rmdir(CHAR* url, CHAR* dname, CHAR* user, CHAR* psw, BOOL ssl);

///"imaps://imap-email.outlook.com:993/"
//"imaps://imap-email.outlook.com:993/[nomecartella]/;uid=numeromesaggiodaprendere"
	//"imaps://imap-email.outlook.com:993/[nomecartella]/;uid=n/;SECTION=TEXT"
EMALIST* ema_dirlist(CHAR* url, CHAR* usr, CHAR* psw, BOOL ssl);
VOID ema_list_free(EMALIST* fl);
EMALINFO* ema_infodir(CHAR* url, CHAR* dir, CHAR* usr, CHAR* psw, BOOL ssl);
VOID ema_linfo_free(EMALINFO* el);
EEMAIL* ema_list(CHAR* url,CHAR* dir, UINT32 from, UINT32 to, BOOL onlyheader, CHAR* usr, CHAR* psw, BOOL ssl);

#endif // EASYHTTP_H_INCLUDED
