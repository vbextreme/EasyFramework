#include "easyhttp.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>
#include <easythread.h>
#include <easybenchmark.h>
#include <easystring.h>
#include <sys/stat.h>

typedef struct __HTPDOWNUP
{
	CHAR url[HTP_BUFFER];
	CHAR fname[HTP_BUFFER];
	HTPPROG pfnc;
	FILE* f;
	CURL* h;
	THR t;
	INT32 complete;
	FLOAT64 ti;
}_HTPDOWNUP;

typedef struct __HTPFORM
{
	CHAR action[HTP_BUFFER];
	HTPDATA d;
	
	CURL* h;
	struct curl_httppost *formpost;
	struct curl_httppost *lastptr;
	struct curl_slist *headerlist;
}_HTPFORM;


/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// /////////////////////////// SUPPORT ///////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///


static INT32 _htp_progress(VOID* userp, FLOAT64 TotalToDownload, FLOAT64 NowDownloaded, FLOAT64 TotalToUpload, FLOAT64 NowUploaded)
{
    HTPDATA* d = (HTPDATA*)userp;

    if ( d->up ) d->up(userp,NowUploaded,TotalToUpload,0.0);
    if ( d->down ) d->down(userp,NowDownloaded,TotalToDownload,0.0);
    
    return 0;
}

static INT32 _htp_dprogress(VOID* userp, FLOAT64 TotalToDownload, FLOAT64 NowDownloaded, FLOAT64 TotalToUpload, FLOAT64 NowUploaded)
{
    _HTPDOWNUP* d = (_HTPDOWNUP*)userp;
	
	FLOAT64 eti = NowDownloaded / (bch_get() - d->ti) ;
	
    if ( d->pfnc ) d->pfnc(userp,NowDownloaded,TotalToDownload,eti);
    return 0;
}

static INT32 _htp_uprogress(VOID* userp, FLOAT64 TotalToDownload, FLOAT64 NowDownloaded, FLOAT64 TotalToUpload, FLOAT64 NowUploaded)
{
    _HTPDOWNUP* d = (_HTPDOWNUP*)userp;
	
	FLOAT64 eti = NowUploaded / (bch_get() - d->ti) ;
	
    if ( d->pfnc ) d->pfnc(userp,NowUploaded,TotalToUpload,eti);
    return 0;
}

static SIZET _htp_frecv(VOID* ptr, SIZET size, SIZET nmemb, VOID* userp) 
{
	_HTPDOWNUP* d = (_HTPDOWNUP*)userp;
	return fwrite(ptr,size,nmemb,d->f);
}

static SIZET _htp_recv(VOID* ptr, SIZET size, SIZET nmemb, VOID* userp) 
{
	HTPBUF* r = (HTPBUF*)userp;
	UINT32 maxs = size * nmemb;
	
	if ( r->size <= r->rem + maxs )
	{
		UINT32 nws = r->size + maxs + HTP_BUFFER;
		CHAR* nwd = malloc(nws);
		memcpy(nwd,r->data,r->size);
		free(r->data);
		r->data = nwd;
		r->pdata = nwd + r->rem;
		r->size = nws;
	}
	
	memcpy(r->pdata,ptr, maxs);
	r->rem += maxs;
	r->pdata += maxs;
	
	return maxs;
}

static size_t _htp_send(void *ptr, size_t size, size_t nmemb, void *userp)
{
	HTPBUF* s = (HTPBUF*)userp;
	UINT32 maxs = size * nmemb;
	
	if( maxs < 1) {return 0;}
 
	if( s->rem < s->size ) 
	{
		memcpy(ptr,s->pdata, maxs);
		s->rem += maxs;
		s->pdata += maxs;
		return maxs;
	}
	return 0;  
}

static size_t _htp_fsend(void *ptr, size_t size, size_t nmemb, void *userp)
{
	_HTPDOWNUP* s = (_HTPDOWNUP*)userp;
	return fread(ptr,size,nmemb,s->f);
}

VOID htp_init()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

VOID htp_terminate()
{
	curl_global_cleanup();
}

INT32 htp_errno()
{
	return errno;
}

const CHAR* htp_errstr()
{
	return curl_easy_strerror(errno);
}

CHAR* _url_root(CHAR* url)
{
	url = str_movetoc(url,'/');
		if ( !*url ) return NULL;
	++url;
		if ( *url != '/' ) return NULL;
	++url;
	url = str_movetoc(url,'/');
	if ( !*url )
	{
		*url = '/';
		*(url+1) = '\0';
	}
	
	return url;
}

BOOL url_walk(CHAR* url, CHAR* walk)
{
	if ( *walk == '.' )
	{
		++walk;
		if ( *walk == '.' )
		{
			++walk;
			if ( *walk == '/' )
			{
				CHAR* root = _url_root(url);
				CHAR* bk = strrchr(url,'/');
				*bk = '\0';
				bk = strrchr(url,'/');
					if ( !bk ) return FALSE;
					if ( bk < root ) return FALSE;
				strcpy(bk, walk);
			}
			else
			{
				CHAR* root = _url_root(url);
				CHAR* bk = strrchr(url,'/');
				*bk = '\0';
				bk = strrchr(url,'/');
					if ( !bk ) return FALSE;
					if ( bk < root ) return FALSE;
				++bk;
				*bk = '\0';
			}
		}
		else if ( *walk == '/' )
		{
			strcpy(&url[strlen(url)-1],walk);
		}
		else
		{
			return FALSE;
		}
	}
	else if ( *walk == '/' && *(walk+1) == '\0' )
	{
		CHAR* root = _url_root(url);
		*(root+1) = '\0';
	}
	else
	{
		strcpy(url,walk);
	}
	
	UINT32 l = strlen(url);
	if ( url[l-1] != '/' )
	{
		url[l] = '/';
		url[l+1] = '\0';
	}
	
	return TRUE;
}

/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////// HTTP ////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///


HTPDATA* htp_get(CHAR* url, BOOL ssl, HTPPROG scbk, HTPPROG rcbk)
{
	CURL* h;
		if ( !(h = curl_easy_init()) ) {return NULL;}
		
	HTPDATA* r = malloc(sizeof(HTPDATA));
		r->header.data = malloc(HTP_BUFFER);
		r->header.pdata = r->header.data;
		r->header.size = HTP_BUFFER;
		r->header.rem = 0;
		r->body.data = malloc(HTP_BUFFER);
		r->body.pdata = r->body.data;
		r->body.size = HTP_BUFFER;
		r->body.rem = 0;
		
	r->url = url;
	r->down = rcbk;
	r->up = scbk;
		
	curl_easy_setopt(h, CURLOPT_URL, url);
	if ( ssl )
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
	else
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, _htp_recv);
	curl_easy_setopt(h, CURLOPT_HEADERDATA, &r->header);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &r->body);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(h, CURLOPT_PROGRESSFUNCTION, _htp_progress);
    curl_easy_setopt(h, CURLOPT_PROGRESSDATA, r);
    
	CURLcode res;
	res = curl_easy_perform(h);
	
	curl_easy_cleanup(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		htp_data_free(r);
		return NULL;
	}
	return r;
}


HTPDATA* htp_post(CHAR* url, CHAR* post, UINT32 sz, BOOL ssl, HTPPROG scbk, HTPPROG rcbk)
{
	CURL* h;
		if ( !(h = curl_easy_init()) ) {return NULL;}
	
	HTPDATA s;
		s.body.data = post;
		s.body.pdata = post;
		s.body.size = sz;
		s.body.rem = 0;
		
	HTPDATA* r = malloc(sizeof(HTPDATA));
		r->header.data = malloc(HTP_BUFFER);
		r->header.pdata = r->header.data;
		r->header.size = HTP_BUFFER;
		r->header.rem = 0;
		r->body.data = malloc(HTP_BUFFER);
		r->body.pdata = r->body.data;
		r->body.size = HTP_BUFFER;
		r->body.rem = 0;
	
	r->url = url;
	r->down = rcbk;
	r->up = scbk;
	
	curl_easy_setopt(h, CURLOPT_URL, url);
	if ( ssl )
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
	else
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_POST, 1L);
	curl_easy_setopt(h, CURLOPT_READFUNCTION, _htp_send);
	curl_easy_setopt(h, CURLOPT_READDATA, &s.body);
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, _htp_recv);
	curl_easy_setopt(h, CURLOPT_HEADERDATA, &r->header);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &r->body);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(h, CURLOPT_PROGRESSFUNCTION, _htp_progress);
	curl_easy_setopt(h, CURLOPT_PROGRESSDATA, r);
	
	CURLcode res;
	res = curl_easy_perform(h);
	
	curl_easy_cleanup(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		htp_data_free(r);
		return NULL;
	}
	return r;
}

VOID htp_data_free(HTPDATA* d)
{
	free(d->header.data);
	free(d->body.data);
	free(d);
}

VOID* _htp_download(VOID* targ)
{
	THREAD_START(targ,_HTPDOWNUP*,h);
	
	h->ti = bch_get();
	
	CURLcode res;
	res = curl_easy_perform(h->h);

	curl_easy_cleanup(h->h);
	fclose(h->f);
	h->f = NULL;
	
	if ( res != CURLE_OK )
	{
		errno = res;
		h->complete = -1;
		THREAD_END(NULL);
	}
	h->complete = 1;
	THREAD_END(NULL);
}

HTPDOWNUP htp_download(CHAR* fname, CHAR* url, BOOL resume, BOOL ssl, HTPPROG rcbk)
{
	_HTPDOWNUP* h = malloc(sizeof(_HTPDOWNUP));
	
	if ( !(h->h = curl_easy_init()) ) {free(h);return NULL;}
	
	h->complete = 0;
	h->pfnc = rcbk;
	strcpy(h->url,url);
	strcpy(h->fname,fname);
	
	UINT32 res = 0;
	if ( resume )
	{
		
		h->f = fopen(fname,"r");
		if ( h->f )
		{
			fclose(h->f);
			h->f = fopen(fname,"r+");
			fseek(h->f,0,SEEK_END);
			res = ftell(h->f);
		}
		else
		{	
			h->f = fopen(fname,"w");
				if ( !h->f ) { curl_easy_cleanup(h->h);free(h); return NULL;}
		}
	}
	else
	{	
		h->f = fopen(fname,"w");
			if ( !h->f ) { curl_easy_cleanup(h->h);free(h); return NULL;}
	}

	curl_easy_setopt(h->h, CURLOPT_URL, url);
	if ( ssl )
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
	else
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h->h, CURLOPT_WRITEFUNCTION, _htp_frecv);
    curl_easy_setopt(h->h, CURLOPT_WRITEDATA, h);
    curl_easy_setopt(h->h, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(h->h, CURLOPT_PROGRESSFUNCTION, _htp_dprogress);
    curl_easy_setopt(h->h, CURLOPT_PROGRESSDATA, h);
    if ( res ) curl_easy_setopt(h->h, CURLOPT_RESUME_FROM , res);

	h->t = thr_new(_htp_download,0,0,0);
	thr_run(h->t,h);

	return h;
}

VOID htp_downup_pause(HTPDOWNUP h)
{
	curl_easy_pause(h->h, CURLPAUSE_ALL);
}

VOID htp_downup_resume(HTPDOWNUP h)
{
	curl_easy_pause(h->h, CURLPAUSE_CONT);
}

INT32 htp_downup_complete(HTPDOWNUP h, BOOL waitd)
{
	if (waitd)
		thr_waitthr(h->t);
	return h->complete;
}

CHAR* htp_downup_url(HTPDOWNUP h)
{
	return h->url;
}

CHAR* htp_downup_file(HTPDOWNUP h)
{
	return h->fname;
}

VOID htp_downup_free(HTPDOWNUP h)
{
	thr_stop(h->t,1000,1);
	thr_free(h->t);
	if ( h->f ) fclose(h->f);
	free(h);
}

HTPFORM htp_form_new(CHAR* action, BOOL ssl, HTPPROG scbk, HTPPROG rcbk)
{
	_HTPFORM* h = malloc(sizeof(_HTPFORM));
	
	strcpy(h->action,action);
	h->d.up = scbk;
	h->d.down = rcbk;
	h->d.header.data = malloc(HTP_BUFFER);
	h->d.header.pdata = h->d.header.data;
	h->d.header.size = HTP_BUFFER;
	h->d.header.rem = 0;
	h->d.body.data = malloc(HTP_BUFFER);
	h->d.body.pdata = h->d.body.data;
	h->d.body.size = HTP_BUFFER;
	h->d.body.rem = 0;
	h->formpost = NULL;
	h->lastptr = NULL;
	h->headerlist = NULL;
	
	if ( !(h->h = curl_easy_init()) ) {free(h); return NULL;}
	
	if ( ssl )
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
	else
		 curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
		 
	return h;
}
	
VOID htp_form_add(HTPFORM h, CHAR* namefield, CHAR* value, BOOL isfile)
{
	if ( isfile )
	{
		curl_formadd(&h->formpost,&h->lastptr, CURLFORM_COPYNAME, namefield,
					 CURLFORM_FILE, value, CURLFORM_END);
	}
	else
	{
		curl_formadd(&h->formpost,&h->lastptr, CURLFORM_COPYNAME, namefield,
					 CURLFORM_COPYCONTENTS, value, CURLFORM_END);
	}
}
	
HTPDATA* htp_form(HTPFORM h)
{
	//static const char buf[] = "Expect:";
	
	h->headerlist = curl_slist_append(h->headerlist, "Content-Type: multipart/form-data");
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h->h, CURLOPT_URL, h->action);
	curl_easy_setopt(h->h, CURLOPT_HTTPPOST, h->formpost);
	curl_easy_setopt(h->h, CURLOPT_WRITEFUNCTION, _htp_recv);
	curl_easy_setopt(h->h, CURLOPT_HEADERDATA, &h->d.header);
    curl_easy_setopt(h->h, CURLOPT_WRITEDATA, &h->d.body);
    curl_easy_setopt(h->h, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(h->h, CURLOPT_PROGRESSFUNCTION, _htp_progress);
	curl_easy_setopt(h->h, CURLOPT_PROGRESSDATA, &h->d);
	
    CURLcode res;
	res = curl_easy_perform(h->h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		return NULL;
	}
	return &h->d;
}
	
VOID htp_form_free(HTPFORM h)
{
	curl_easy_cleanup(h->h);
	curl_formfree(h->formpost);
	curl_slist_free_all(h->headerlist);
	free(h->d.header.data);
	free(h->d.body.data);
	free(h);
}

/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ////////////////////////////// FTP ////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///


FTPLIST* _ftp_gnu_parselist(HTPDATA* hd, CHAR* data)
{
	UINT32 nfile = 0;
	CHAR* s = data;
	
	for(; *s; ++s )
		if ( *s == '\n' ) ++nfile;
		
	FTPLIST* fl = malloc(sizeof(FTPLIST) * nfile);
	
	UINT32 i = 0;
	for ( s = data; i < nfile; ++i )
	{
		fl[i].rfile = nfile - i;
		fl[i].d = hd;
		
		switch (*s)
		{
			default: case '-': fl[i].ftype = FTP_FTYPE_REG; break;
			case 'l': fl[i].ftype = FTP_FTYPE_LINK; break;
			case 'd': fl[i].ftype = FTP_FTYPE_DIR; break;
		}
		++s;
		s = str_copytos(fl[i].pri,s," \t");
		s = str_skipspace(s);
		s = str_movetos(s," \t");
		s = str_skipspace(s);
		s = str_movetos(s," \t");
		s = str_skipspace(s);
		s = str_movetos(s," \t");
		s = str_skipspace(s);
		CHAR* es;
		fl[i].size = strtoul(s,&es,10); s = es + 1;
		
		s = str_copytos(&fl[i].date[3],s," \t");
		s = str_skipspace(s);
		
		s = str_copytos(fl[i].date,s," \t");
		s = str_skipspace(s);
		fl[i].date[2] = '/';
		fl[i].date[6] = '/';
		
		if ( *(s+2) == ':' )
		{
			s = str_copytos(&fl[i].date[12],s," \t");
			fl[i].date[7] = '-';
			fl[i].date[8] = '-';
			fl[i].date[9] = '-';
			fl[i].date[10] = '-';
		}
		else
		{
			s = str_copytos(&fl[i].date[7],s," \t");
			fl[i].date[12] = '-';
			fl[i].date[13] = '-';
			fl[i].date[14] = ':';
			fl[i].date[15] = '-';
			fl[i].date[16] = '-';
		}
		
		fl[i].date[11] = ' ';
		fl[i].date[17]= '\0';
		
		s = str_skipspace(s);
		s = str_copytos(fl[i].name,s," \n\t");
		
		if ( fl[i].ftype == FTP_FTYPE_LINK )
		{
			s = str_skipspace(s);
			s+=2;
			s = str_skipspace(s);
			s = str_copytos(fl[i].link,s," \n\t");
		}
		
		s = str_skipline(s);
	}
	
	return fl;
}

FTPLIST* _ftp_ms_parselist(HTPDATA* hd, CHAR* data)
{
	UINT32 nfile = 0;
	CHAR* s = data;
	
	for(; *s; ++s )
		if ( *s == '\n' ) ++nfile;
	
	FTPLIST* fl = malloc(sizeof(FTPLIST) * nfile);
	
	UINT32 i = 0;
	for ( s = data; i < nfile; ++i )
	{
		fl[i].rfile = nfile - i;
		fl[i].d = hd;
		fl[i].pri[0] = '\0';
		fl[i].link[0] = '\0';
		
		CHAR* es = s;
		s = str_movetoc(s,' ');
		*s = '\0';
		strcpy(fl[i].date,es);
		*s++ = ' ';
		es = s;
		++s;
		s = str_movetoc(s,' ');
		*s = '\0';
		strcat(fl[i].date,es);
		*s = ' ';
		++s;
		
		s = str_skipspace(s);
		if ( !strncmp(s,"<DIR>",5) )
		{
			fl[i].ftype = FTP_FTYPE_DIR;
			fl[i].size = 0;
			s = str_movetos(s," \t");
		}
		else
		{
			fl[i].ftype = FTP_FTYPE_REG;
			fl[i].size = strtoul(s,&es,10); s = es + 1;
		}
		
		s = str_skipspace(s);
		s = str_copytos(fl[i].name,s," \n\t");
		s = str_skipline(s);
	}
	return fl;
}

FTPLIST* ftp_list(CHAR* url, CHAR* user, CHAR* psw, BOOL ssl, HTPPROG scbk, HTPPROG rcbk)
{
	CURL* h;
		if ( !(h = curl_easy_init()) ) {return NULL;}
		
	HTPDATA* r = malloc(sizeof(HTPDATA));
		r->header.data = malloc(HTP_BUFFER);
		r->header.pdata = r->header.data;
		r->header.size = HTP_BUFFER;
		r->header.rem = 0;
		r->body.data = malloc(HTP_BUFFER);
		r->body.pdata = r->body.data;
		r->body.size = HTP_BUFFER;
		r->body.rem = 0;
		
	r->url = url;
	r->down = rcbk;
	r->up = scbk;
		
	curl_easy_setopt(h, CURLOPT_URL, url);
	if ( ssl ) curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	if ( user ) curl_easy_setopt(h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_CUSTOMREQUEST, "LIST");
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, _htp_recv);
	curl_easy_setopt(h, CURLOPT_HEADERDATA, &r->header);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &r->body);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(h, CURLOPT_PROGRESSFUNCTION, _htp_progress);
    curl_easy_setopt(h, CURLOPT_PROGRESSDATA, r);
    
	CURLcode res;
	res = curl_easy_perform(h);
	
	curl_easy_cleanup(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		htp_data_free(r);
		return NULL;
	}
	
	if ( !strncmp(r->header.data,"220 Microsoft",13) )
	{
		return _ftp_ms_parselist(r,r->body.data);
	}
	else if ( !strncmp(r->header.data,"220 GNU",7) )
	{
		return _ftp_gnu_parselist(r,r->body.data);
	}
	
	return _ftp_gnu_parselist(r,r->body.data);
}

VOID ftp_list_free(FTPLIST* fl)
{
	htp_data_free(fl[0].d);
	free(fl);
}

HTPDOWNUP ftp_download(CHAR* fname, CHAR* url, CHAR* user, CHAR* psw, BOOL resume, BOOL ssl, HTPPROG rcbk)
{
	_HTPDOWNUP* h = malloc(sizeof(_HTPDOWNUP));
	
	if ( !(h->h = curl_easy_init()) ) {free(h);return NULL;}
	
	h->complete = 0;
	h->pfnc = rcbk;
	strcpy(h->url,url);
	strcpy(h->fname,fname);
	
	UINT32 res = 0;
	if ( resume )
	{
		
		h->f = fopen(fname,"r");
		if ( h->f )
		{
			fclose(h->f);
			h->f = fopen(fname,"r+");
			fseek(h->f,0,SEEK_END);
			res = ftell(h->f);
		}
		else
		{	
			h->f = fopen(fname,"w");
				if ( !h->f ) { curl_easy_cleanup(h->h);free(h); return NULL;}
		}
	}
	else
	{	
		h->f = fopen(fname,"w");
			if ( !h->f ) { curl_easy_cleanup(h->h);free(h); return NULL;}
	}

	curl_easy_setopt(h->h, CURLOPT_URL, url);
	if ( ssl ) curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	if ( user ) curl_easy_setopt(h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h->h, CURLOPT_WRITEFUNCTION, _htp_frecv);
    curl_easy_setopt(h->h, CURLOPT_WRITEDATA, h);
    curl_easy_setopt(h->h, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(h->h, CURLOPT_PROGRESSFUNCTION, _htp_dprogress);
    curl_easy_setopt(h->h, CURLOPT_PROGRESSDATA, h);
    if ( res ) curl_easy_setopt(h->h, CURLOPT_RESUME_FROM , res);

	h->t = thr_new(_htp_download,0,0,0);
	thr_run(h->t,h);

	return h;
}

HTPDOWNUP ftp_upload(CHAR* url, CHAR* user, CHAR* psw, CHAR* fname, BOOL ssl, HTPPROG rcbk)
{
	_HTPDOWNUP* h = malloc(sizeof(_HTPDOWNUP));
	
	if ( !(h->h = curl_easy_init()) ) {free(h);return NULL;}
	
	h->complete = 0;
	h->pfnc = rcbk;
	strcpy(h->url,url);
	strcpy(h->fname,fname);
	
	
	h->f = fopen(fname,"r");
		if ( !h->f ) { curl_easy_cleanup(h->h); free(h); return NULL;}
	
	struct stat fi;
	if( fstat(fileno(h->f), &fi) != 0) 
	{
		fclose(h->f);
		curl_easy_cleanup(h->h); 
		free(h);
		return NULL;
	}
	
	CHAR* rfn = strrchr(fname,'/');
		if ( *rfn != '/' ) 
			rfn = fname;
		else
			++rfn;
	
	strcat(url,rfn);
	curl_easy_setopt(h->h, CURLOPT_URL, url);
	if ( user ) curl_easy_setopt(h->h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(h->h, CURLOPT_PASSWORD, psw);
	if ( ssl ) curl_easy_setopt(h->h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	curl_easy_setopt(h->h, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(h->h, CURLOPT_READDATA, h);
	curl_easy_setopt(h->h, CURLOPT_READFUNCTION, _htp_fsend);
	curl_easy_setopt(h->h, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fi.st_size);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h->h, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(h->h, CURLOPT_PROGRESSFUNCTION, _htp_uprogress);
    curl_easy_setopt(h->h, CURLOPT_PROGRESSDATA, h);
    
    h->t = thr_new(_htp_download,0,0,0);
	thr_run(h->t,h);

	return h;
}

INT32 ftp_rename(CHAR* url, CHAR* from, CHAR* to, CHAR* user, CHAR* psw, BOOL ssl)
{
	CURL* h;
	CURLcode res;
	CHAR* root = _url_root(url);
	
	CHAR buffr[1024];
		sprintf(buffr,"RNFR %s%s",root,from);
	CHAR bufto[1024];
		sprintf(bufto,"RNTO %s%s",root,to);
		
	if ( !(h = curl_easy_init()) ) return -1;
  
	struct curl_slist* headerlist = NULL;
    headerlist = curl_slist_append(headerlist, buffr);
    headerlist = curl_slist_append(headerlist, bufto);
	
    curl_easy_setopt(h,CURLOPT_URL, url);
    if ( ssl ) curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	if ( user ) curl_easy_setopt(h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	FILE *devnull = fopen("/dev/null", "w+");
	curl_easy_setopt(h, CURLOPT_WRITEDATA, devnull);
    curl_easy_setopt(h, CURLOPT_POSTQUOTE, headerlist);

    res = curl_easy_perform(h);
    if(res != CURLE_OK)
    {
		errno = res;
		curl_slist_free_all(headerlist);
		curl_easy_cleanup(h);
		return -1;
    }
    
    curl_slist_free_all (headerlist);
    curl_easy_cleanup(h);
    return 0;
}

INT32 ftp_delete(CHAR* url, CHAR* fname, CHAR* user, CHAR* psw, BOOL ssl)
{
	CURL* h;
	CURLcode res;
	
	
	CHAR dcmd[1024];
		sprintf(dcmd,"DELE %s",fname);
		
	if ( !(h = curl_easy_init()) ) return -1;
	
    curl_easy_setopt(h,CURLOPT_URL, url);
    if ( ssl ) curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	if ( user ) curl_easy_setopt(h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_CUSTOMREQUEST, dcmd);
	
    res = curl_easy_perform(h);
    if(res != CURLE_OK && res != 19)
    {
		errno = res;
		curl_easy_cleanup(h);
		return -1;
    }
    
    curl_easy_cleanup(h);
    return 0;
}

INT32 ftp_mkdir(CHAR* url, CHAR* dname, CHAR* user, CHAR* psw, BOOL ssl)
{
	CURL* h;
	CURLcode res;
	
	
	CHAR dcmd[1024];
		sprintf(dcmd,"MKD %s",dname);
		
	if ( !(h = curl_easy_init()) ) return -1;
	
    curl_easy_setopt(h,CURLOPT_URL, url);
    if ( ssl ) curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	if ( user ) curl_easy_setopt(h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_CUSTOMREQUEST, dcmd);
	
    res = curl_easy_perform(h);
    if(res != CURLE_OK && res != 19)
    {
		errno = res;
		curl_easy_cleanup(h);
		return -1;
    }
    
    curl_easy_cleanup(h);
    return 0;
}

INT32 ftp_rmdir(CHAR* url, CHAR* dname, CHAR* user, CHAR* psw, BOOL ssl)
{
	CURL* h;
	CURLcode res;
	
	
	CHAR dcmd[1024];
		sprintf(dcmd,"RMD %s",dname);
		
	if ( !(h = curl_easy_init()) ) return -1;
	
    curl_easy_setopt(h,CURLOPT_URL, url);
    if ( ssl ) curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	if ( user ) curl_easy_setopt(h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_CUSTOMREQUEST, dcmd);
	
    res = curl_easy_perform(h);
    if(res != CURLE_OK && res != 19)
    {
		errno = res;
		curl_easy_cleanup(h);
		return -1;
    }
    
    curl_easy_cleanup(h);
    return 0;
}

/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////// EMAIL ///////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// "imap://imap.example.com"

EMALIST* _ema_parsedirlist(HTPDATA* hd, CHAR* data)
{
	UINT32 nfile = 0;
	CHAR* s = data;
	
	for(; *s; ++s )
		if ( *s == '\n' ) ++nfile;
	
	EMALIST* fl = malloc(sizeof(EMALIST) * nfile);
	
	UINT32 i = 0;
	for ( s = data; i < nfile; ++i )
	{
		fl[i].rfile = nfile - i;
		fl[i].d = hd;
		
		s = str_movetoc(s,'\"'); ++s;
		s = str_movetoc(s,'\"'); ++s;
		s = str_movetoc(s,'\"'); ++s;
		s = str_copytoc(fl[i].name,s,'\"');++s;
		s = str_skipline(s);
	}
	return fl;
}

EMALIST* ema_dirlist(CHAR* url, CHAR* usr, CHAR* psw, BOOL ssl)
{
	CURL* h;
		if ( !(h = curl_easy_init()) ) {return NULL;}
		
	HTPDATA* r = malloc(sizeof(HTPDATA));
		r->header.data = malloc(HTP_BUFFER);
		r->header.pdata = r->header.data;
		r->header.size = HTP_BUFFER;
		r->header.rem = 0;
		r->body.data = malloc(HTP_BUFFER);
		r->body.pdata = r->body.data;
		r->body.size = HTP_BUFFER;
		r->body.rem = 0;
		
	r->url = url;
	r->down = NULL;
	r->up = NULL;
		
	curl_easy_setopt(h, CURLOPT_URL, url);
	if ( ssl )
	{
		curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 2L);
	}
	if ( usr ) curl_easy_setopt(h, CURLOPT_USERNAME, usr);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, _htp_recv);
	curl_easy_setopt(h, CURLOPT_HEADERDATA, &r->header);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &r->body);
    
	CURLcode res;
	res = curl_easy_perform(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		curl_easy_cleanup(h);
		htp_data_free(r);
		return NULL;
	}
	
	curl_easy_cleanup(h);
	return _ema_parsedirlist(r,r->body.data);
}

VOID ema_list_free(EMALIST* fl)
{
	htp_data_free(fl[0].d);
	free(fl);
}

EMALINFO* _ema_linfo(HTPDATA* hd, CHAR* data)
{
	EMALINFO* el = malloc(sizeof(EMALINFO));
	
	data = str_skipline(data);
	++data;
	data = str_skipspace(data);
	el->count = strtoul(data,NULL,10);
	
	data = str_skipline(data);
	++data;
	data = str_skipspace(data);
	el->recent = strtoul(data,NULL,10);
	
	el->d = hd;
	return el;
}

EMALINFO* ema_infodir(CHAR* url, CHAR* dir, CHAR* usr, CHAR* psw, BOOL ssl)
{
	CHAR exa[HTP_BUFFER];
		sprintf(exa,"EXAMINE %s",dir);
	
	CURL* h;
		if ( !(h = curl_easy_init()) ) {return NULL;}
		
	HTPDATA* r = malloc(sizeof(HTPDATA));
		r->header.data = malloc(HTP_BUFFER);
		r->header.pdata = r->header.data;
		r->header.size = HTP_BUFFER;
		r->header.rem = 0;
		r->body.data = malloc(HTP_BUFFER);
		r->body.pdata = r->body.data;
		r->body.size = HTP_BUFFER;
		r->body.rem = 0;
		
	r->url = url;
	r->down = NULL;
	r->up = NULL;
		
	curl_easy_setopt(h, CURLOPT_URL, url);
	if ( ssl )
	{
		curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 2L);
	}
	if ( usr ) curl_easy_setopt(h, CURLOPT_USERNAME, usr);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, _htp_recv);
	curl_easy_setopt(h, CURLOPT_HEADERDATA, &r->header);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &r->body);
    curl_easy_setopt(h, CURLOPT_CUSTOMREQUEST, exa);
    
    
	CURLcode res;
	res = curl_easy_perform(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		curl_easy_cleanup(h);
		htp_data_free(r);
		return NULL;
	}
	
	curl_easy_cleanup(h);
	return _ema_linfo(r,r->body.data);
}

VOID ema_linfo_free(EMALINFO* el)
{
	htp_data_free(el->d);
	free(el);
}


INT32 _ema_parsetofetch(CHAR** hdata)
{
	puts("PARSE TO FETCH");
	
	CHAR* p = *hdata;
	INT32 idf;
	CHAR* eidf;
	
	while ( *p )
	{
		if ( *p != '*' )
		{
			//printf("\t!P [%16.16s]\n",p);
			p = str_skipline(p);
			continue;
		}
			
		++p;
		p = str_skipspace(p);
		idf = strtol(p,&eidf,10);
		p = eidf;
		p = str_skipspace(p);
		if ( strncmp(p,"FETCH",5) )
		{
			//printf("\t!F [%16.16s]\n",p);
			p = str_skipline(p);
			continue;
		}
		break;
	}
	
	p = str_skipline(p);
	*hdata = p;
	//printf("\tFETCH [%16.16s]\n",p);
	return (!*p) ? -1 : idf;
}

CHAR* _parsern0(CHAR* hdata)
{
	hdata = str_movetos(hdata,"\r\n");
	if ( *hdata == '\r' )
	{
		*hdata = '\0';
		hdata += 2;
	}
	else if ( *hdata == '\0' )
	{
		++hdata;
	}
	else
	{
		*hdata = '\0';
		++hdata;
	}
	return hdata;
}

INT32 _ema_parseheader(CHAR** hdata, EEMAIL* ema)
{
	puts("PARSE HEADER");
	
	CHAR* p = *hdata;
	BOOL endheader = FALSE;
	BOOL havebody = FALSE;
	
	while ( *p && !endheader)
	{	
		
		//printf("\tPARSE[%32.32s]\n",p);
		switch ( *p )
		{	
			case 'D':
				if ( strncmp(p,"Date:",5) ) break;
				p += 5;
				p = str_skipspace(p);
				ema->date = p;
				p = _parsern0(p);
				//printf("\tDATE[%s]\n",ema->date);
			continue;
				
			case 'F':
				if ( strncmp(p,"From:",5) ) break;
				p += 5;
				p = str_skipspace(p);
				ema->from = p;
				p = _parsern0(p);
				//printf("\tFrom[%s]\n",ema->from);
			continue;
				
			case 'T':
				if ( strncmp(p,"To:",3) ) break;
				p += 3;
				p = str_skipspace(p);
				ema->to = p;
				p = _parsern0(p);	
				//printf("\tTO[%s]\n",ema->to);
			continue;
				
			case 'M':
				if ( strncmp(p,"Message-ID:",11) ) break;
				p += 11;
				p = str_skipspace(p);
				ema->messageid = p;
				p = _parsern0(p);
				//printf("\tMSID[%s]\n",ema->messageid);
			continue;
			
			case 'S':
				if ( strncmp(p,"Subject:",8) ) break;
				p += 8;
				p = str_skipspace(p);
				ema->subject = p;
				p = _parsern0(p);
				//printf("\tSubject[%s]\n",ema->subject);
			continue;
				
			case 'C':
				if ( strncmp(p,"Content-Type",12) ) break;
				p += 12;
				ema->body = p;
				endheader = TRUE;
				havebody = TRUE;
				//printf("\tPART[%s]\n",ema->part);
			continue;
			
			case ')':
				endheader = TRUE;
			break;
			
		}
			
		p = str_skipline(p);
	}
	
	*hdata = p;
	return (havebody) ? 1 : 0;
}

INT32 _ema_parsebody(CHAR** hdata, EEMAIL* ema)
{
	printf("PARSE BODY\n");
	
	CHAR* p = *hdata;
	
	ema->body = p;
	
	while ( *p )
	{	
		if ( *p == ')' && *(p-1) == 10 && *(p-2) == 13 ) 
		{
			*p = '\0';
			++p;
			p = str_skipline(p);
			break;
		}
		p = str_skipline(p);
	}
	
	*hdata = p;
	if ( !*p ) {ema->body = NULL; return -1;}
	return 0;
}

EEMAIL* _ema_parse(HTPDATA* hd, CHAR* hdata, UINT32 from, UINT32 to, BOOL onlyheader)
{
	UINT32 nfile = (to - from) + 1;
	
	EEMAIL* ema = malloc(sizeof(EEMAIL) * nfile);
	
	UINT32 i = 0;
	INT32 idf;
	
	while( i < nfile && (idf = _ema_parsetofetch(&hdata)) >= 0 )
	{
		ema[i].idf = idf;
		ema[i].rfile = nfile - i;
		ema[i].from = NULL;
		ema[i].to = NULL;
		ema[i].date = NULL;
		ema[i].subject = NULL;
		ema[i].messageid = NULL;
		ema[i].part = NULL;
		ema[i].body = NULL;
		ema[i].d = hd;
		
		if ( _ema_parseheader(&hdata,ema + i) && !onlyheader )
		{
			_ema_parsebody(&hdata,ema+i);
		}
		++i;
	}
	return ema;
}

EEMAIL* ema_list(CHAR* url,CHAR* dir, UINT32 from, UINT32 to, BOOL onlyheader, CHAR* usr, CHAR* psw, BOOL ssl)
{
	CHAR rurl[HTP_BUFFER];
		sprintf(rurl,"%s/%s/",url,dir);
	
	CHAR of[HTP_BUFFER];
	
	///sprintf(of,"FETCH %u:%u BODY.PEEK[HEADER.FIELDS (%s)]",from,to,field);	
	if ( onlyheader )
		sprintf(of,"FETCH %u:%u BODY[HEADER]",from,to);
	else
		sprintf(of,"FETCH %u:%u BODY[]",from,to);
	
	printf("RURL[%s]\n",rurl);
	
	CURL* h;
		if ( !(h = curl_easy_init()) ) {return NULL;}
		
	HTPDATA* r = malloc(sizeof(HTPDATA));
		r->header.data = malloc(HTP_BUFFER);
		r->header.pdata = r->header.data;
		r->header.size = HTP_BUFFER;
		r->header.rem = 0;
		r->body.data = malloc(HTP_BUFFER);
		r->body.pdata = r->body.data;
		r->body.size = HTP_BUFFER;
		r->body.rem = 0;
		
	r->url = url;
	r->down = NULL;
	r->up = NULL;
		
	curl_easy_setopt(h, CURLOPT_URL, rurl);
	if ( ssl )
	{
		curl_easy_setopt(h, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 2L);
	}
	if ( usr ) curl_easy_setopt(h, CURLOPT_USERNAME, usr);
	if ( psw ) curl_easy_setopt(h, CURLOPT_PASSWORD, psw);
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, _htp_recv);
	curl_easy_setopt(h, CURLOPT_HEADERDATA, &r->header);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &r->body);
    curl_easy_setopt(h, CURLOPT_CUSTOMREQUEST, of);
    
	CURLcode res;
	res = curl_easy_perform(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		curl_easy_cleanup(h);
		htp_data_free(r);
		return NULL;
	}
	
	curl_easy_cleanup(h);
	return _ema_parse(r,r->header.data,from,to,onlyheader);
}


















