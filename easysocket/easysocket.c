#include "easysocket.h"
#include <easythread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>

#define INVALID_SOCKET -1

typedef struct _SCKINFO
{
    struct sockaddr_in adr;
	CHAR ip[SCK_IP_SIZE];
}SCKINFO;

typedef struct _SCKFNC
{
	SCKCALL connect;
    SCKCALL disconnect;
    SCKCALL accept;
    SCKCALL incoming;
    SCKCALL eerr;
    SCKCALL endsend;
}SCKFNC;

typedef struct _SCKARG
{
	SCKCALL argfree;
    void* arg;
    BOOL autofree;
}SCKARG;

typedef struct _SCKBUF
{
	PKMODEL mode;
	INT32 rsz;
	BYTE* buf;
}SCKBUF;

typedef struct __SCK
{
	HSCK h;
    SCKMODEL mod;
    SCKINFO  inf;
    SCKFNC   fnc;
    SCKARG   arg;
    SCKBUF   buf;
    
    THR th;
	
	struct __SCK* child;
	struct __SCK* server;
}_SCK;


static void _sck_add(_SCK* srv,_SCK* s)
{
    if ( !srv || srv->mod != SKSERVER ) return;
	
	s->child = srv->child;
	s->server = srv;
	srv->child = s;
}

static void _sck_remove(_SCK* s,HSCK h)
{
    if ( !s || s->mod != SKSERVER ) return;

    _SCK* f;
    _SCK* r;
    for(f = s; f->child; f = f->child)
    {
        if (f->child->h == h)
        {
            r = f->child;
            f->child = r->child;
            return;
        }
    }
}

static INT32 _sck_read(_SCK* s, INT32 sz)
{
	INT32 r;
	BYTE* b = s->buf.buf;
	while ( sz > 0)
	{
		r =  recv(s->h,  b, sz, 0);
			if ( r <= 0) { return 0; }
		sz -= r;
		b += r;
	}
	return 1;
}

static INT32 _sck_recv(_SCK* s)
{
	INT32 r;
	
	if ( s->buf.mode == PKPACK )
	{
		PACK pk;
		
		if ( !_sck_read(s,4) ) { if ( s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_READ); sck_close(s); return 0; }
		memcpy(&pk.sz,s->buf.buf,4);
		if ( pk.sz > s->buf.rsz )
		{
			free(s->buf.buf);
			s->buf.buf = malloc(pk.sz);
			s->buf.rsz = pk.sz;
		}
		
		if ( !_sck_read(s,4) ) { if ( s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_READ); sck_close(s); return 0; }
		memcpy(&pk.count,s->buf.buf,4);
		
		if ( pk.count > 1 )
		{
			if ( !_sck_read(s,4) ) { if ( s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_READ); sck_close(s); return 0; }
			memcpy(&pk.id,s->buf.buf,4);
		}
		
		if ( !_sck_read(s,pk.sz) ) { if ( s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_READ); sck_close(s); return 0; }
		pk.data = s->buf.buf;
		
		if ( s->fnc.incoming ) s->fnc.incoming(s,&pk,-1);
		return pk.sz;
	}
	
	r =  recv(s->h, s->buf.buf , s->buf.rsz, 0);
	if ( r <= 0) { if ( s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_READ); sck_close(s); return 0; }
	if ( s->fnc.incoming ) s->fnc.incoming(s,s->buf.buf,r);
	return r;
}

static void* _sck_cli_run(void* p)
{
    THREAD_START(p,_SCK*,s);
	
    if ( connect(s->h,(struct sockaddr *)&s->inf.adr,sizeof(struct sockaddr_in)) < 0)
    {
		if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_CONNECT);
        if (s->fnc.disconnect) s->fnc.disconnect(s,NULL,0);
        THREAD_END(0);
    }
    
    if (s->fnc.connect) s->fnc.connect(s,s->arg.arg,0);
    
    while ( s->h >= 0)
    {
		THREAD_REQUEST();
		
		if ( !_sck_recv(s) ) break;
    }
	
	THREAD_ONEXIT
	
    if (s->fnc.disconnect) s->fnc.disconnect(s,NULL,0);

    THREAD_END(0);
}

static void* _sck_srv_run(void* p)
{
    THREAD_START(p,_SCK*,s);

    while ( s->h >= 0)
    {
		THREAD_REQUEST();
        
        if (listen(s->h,1) < 0)
        {
            if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_LISTEN);
            THREAD_END(0);
        }
        
        struct sockaddr_in adr;
        size_t sinsize = sizeof(struct sockaddr_in);

        HSCK hmc = accept(s->h,(struct sockaddr *)&adr,&sinsize);
			if (hmc  < 0 ) { if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_ACCEPT);  continue; }

        
        _SCK* mc = sck_sc_new(s,hmc,htons(adr.sin_port));
        strcpy(mc->inf.ip,inet_ntoa(adr.sin_addr));
		
        if ( s->fnc.accept )
        {
			if (!s->fnc.accept(mc,NULL,0) )
            {
                sck_free(mc);
                continue;
            }
        }

        sck_run(mc);
    }//thread loop

	THREAD_ONEXIT
	
    if (s->fnc.disconnect) s->fnc.disconnect(s,NULL,0);

    THREAD_END(0);
}

static void* _sck_sc_run(void* p)
{
    THREAD_START(p,_SCK*,s);   
    
    while ( s->h >= 0)
    {
		THREAD_REQUEST();
		
		if ( !_sck_recv(s) ) break;
    }
	
	THREAD_ONEXIT
	
    if (s->fnc.disconnect) s->fnc.disconnect(s,NULL,0);

    THREAD_END(0);
}

static INT32 _sck_sere_run(SCK s)
{	
    if ( connect(s->h,(struct sockaddr *)&s->inf.adr,sizeof(struct sockaddr_in)) < 0)
	{
		return 0;
    }
    return 1;
}

SCK sck_cli_new(CHAR* ip,INT32 port)
{
	_SCK* c = malloc(sizeof(_SCK));
	
	c->mod = SKCLIENT;
	c->fnc.connect = NULL;
    c->fnc.disconnect = NULL;
    c->fnc.accept = NULL;
    c->fnc.incoming = NULL;
    c->fnc.eerr = NULL;
    c->fnc.endsend = NULL;
    c->arg.autofree = FALSE;
    c->arg.argfree = NULL;
    c->arg.arg = NULL;
    c->buf.buf = malloc(SCK_BUFFER_SIZE);
    c->buf.mode = PKDEFAULT;
    c->buf.rsz = SCK_BUFFER_SIZE;
    
	memset(&c->inf.adr,0, sizeof(struct sockaddr_in));
    c->inf.adr.sin_family = AF_INET;
    c->inf.adr.sin_port = htons(port);
    c->inf.adr.sin_addr.s_addr = inet_addr(ip);

    if ( (c->h = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0 ) {free(c);return NULL;}
    
    c->th = thr_new(_sck_cli_run,0,0,0);
    
    return c;
}

SCK sck_srv_new(INT32 port)
{
    _SCK* s = malloc(sizeof(_SCK));
	
	s->mod = SKSERVER;
	s->fnc.connect = NULL;
    s->fnc.disconnect = NULL;
    s->fnc.accept = NULL;
    s->fnc.incoming = NULL;
    s->fnc.eerr = NULL;
    s->fnc.endsend = NULL;
    s->arg.autofree = FALSE;
    s->arg.argfree = NULL;
    s->arg.arg = NULL;
    s->buf.buf = malloc(SCK_BUFFER_SIZE);
    s->buf.mode = PKDEFAULT;
    s->buf.rsz = SCK_BUFFER_SIZE;
    
    memset(&s->inf.adr,0, sizeof(struct sockaddr_in));
    s->inf.adr.sin_family = AF_INET;
    s->inf.adr.sin_port = htons(port);
    s->inf.adr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ( (s->h = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0 ) {free(s);return NULL;}
    
    INT32 flag_ok;
    setsockopt(s->h,SOL_SOCKET,SO_REUSEADDR,(char *)&flag_ok,sizeof(flag_ok));
    if ( bind(s->h,(struct sockaddr *)(&s->inf.adr),sizeof(struct sockaddr_in)) < 0 ) {close(s->h);free(s);return NULL;}

    s->th = thr_new(_sck_srv_run,0,0,0);

    return s;
}

SCK sck_sc_new(SCK srv,HSCK h, INT32 port)
{
    _SCK* s = malloc(sizeof(_SCK));
	
	s->mod = SKSRV;
	s->fnc.connect = srv->fnc.connect;
    s->fnc.disconnect = srv->fnc.disconnect;
    s->fnc.accept = srv->fnc.accept;
    s->fnc.incoming = srv->fnc.incoming;
    s->fnc.eerr = srv->fnc.eerr;
    s->fnc.endsend = srv->fnc.endsend;
    s->arg.autofree = FALSE;
    s->arg.argfree = NULL;
    s->arg.arg = NULL;
    s->buf.buf = malloc(srv->buf.rsz);
    s->buf.mode = srv->buf.mode;
    s->buf.rsz = srv->buf.rsz;
    
    memset(&s->inf.adr,0, sizeof(struct sockaddr_in));
    s->inf.adr.sin_family = AF_INET;
    s->inf.adr.sin_port = htons(port);
    s->h = h;
    _sck_add(srv,s);

    s->th = thr_new(_sck_sc_run,0,0,0);
    
    return s;
}

SCK sck_sere_new(CHAR* ip,INT32 port)
{
	_SCK* c = malloc(sizeof(_SCK));
	
	c->mod = SKSERE;
	c->fnc.connect = NULL;
    c->fnc.disconnect = NULL;
    c->fnc.accept = NULL;
    c->fnc.incoming = NULL;
    c->fnc.eerr = NULL;
    c->fnc.endsend = NULL;
    c->arg.autofree = FALSE;
    c->arg.argfree = NULL;
    c->arg.arg = NULL;
    c->buf.buf = malloc(SCK_BUFFER_SIZE);
    c->buf.mode = PKDEFAULT;
    c->buf.rsz = SCK_BUFFER_SIZE;
    
	memset(&c->inf.adr,0, sizeof(struct sockaddr_in));
    c->inf.adr.sin_family = AF_INET;
    c->inf.adr.sin_port = htons(port);
    c->inf.adr.sin_addr.s_addr = inet_addr(ip);

    if ( (c->h = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0 ) {free(c);return NULL;}
    
    c->th = NULL;
    
    return c;
}

VOID sck_event(SCK s, SCKCALL con, SCKCALL acp, SCKCALL dis, SCKCALL eerr, SCKCALL endsend, SCKCALL incoming)
{
	s->fnc.connect = con;
    s->fnc.disconnect = dis;
    s->fnc.accept = acp;
    s->fnc.incoming = incoming;
    s->fnc.eerr = eerr;
    s->fnc.endsend = endsend;
}

VOID sck_arg_set(SCK s, VOID* arg, SCKCALL fncfree, BOOL autofree)
{
	s->arg.arg = arg;
	s->arg.argfree = fncfree;
	s->arg.autofree = autofree;
}

VOID* sck_arg_get(SCK s)
{
	return s->arg.arg;
}

VOID sck_buf_set(SCK s, PKMODEL mode, BOOL newb, INT32 rsz, VOID* buf)
{
	s->buf.mode = mode;
	if ( newb )
	{
		s->buf.rsz = rsz;
		if ( s->buf.buf ) free(s->buf.buf);
		s->buf.buf = (buf) ? buf : malloc(rsz);
	}
}

VOID* sck_buf_get(SCK s)
{
	return s->buf.buf;
}

INT32 sck_reset(SCK s)
{
    if ( s->mod != SKCLIENT)
    {
        if ( (s->h = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0 )
            return 0;
    }
    else
        return 0;

    if (s->mod == SKSERVER )
    {
        int flag_ok;
        setsockopt(s->h,SOL_SOCKET,SO_REUSEADDR,(char *)&flag_ok,sizeof(flag_ok));
        if ( bind(s->h,(struct sockaddr *)(&s->inf.adr),sizeof(struct sockaddr_in)) < 0 )
        {
            close(s->h);
            return 0;
        }
    }
    return 1;
}

VOID sck_close(SCK s)
{
    if (s->h >= 0);
    shutdown(s->h,2);
    close(s->h);
    s->h = SCK_INVALID;
}

VOID sck_free(SCK s)
{
    if ( !s ) return;
    
    if ( s->mod == SKSERVER)
    {
		_SCK *c,*n;
		for (c = s->child ; c; c = n)
		{
			n = c->child;
			sck_close(c);
			thr_stop(c->th,SCK_FORCETIMEOUT,1);
			thr_free(c->th);
			if ( c->buf.buf ) free(c->buf.buf);
			free(c);
		}
	}
    else if ( s->mod == SKSRV)
    {
        _sck_remove(s->server,s->h);
    }

    sck_close(s);
    
    if (s->arg.argfree)
        s->arg.argfree(s,s->arg.arg,0);
    else if (s->arg.autofree)
        free(s->arg.arg);
    
    if ( s->th )
    {
		thr_stop(s->th,SCK_FORCETIMEOUT,1);
		thr_free(s->th);
	}
    if ( s->buf.buf ) free(s->buf.buf);
    free(s);
}

INT32 _sck_write(SCK s,VOID* data,INT32 sz)
{
    INT32 r;
    BYTE* buf = (BYTE*) data;
    while( sz > 0 )
    {
        r =  send(s->h,buf, sz, 0);
			if (r < 0 ) {return 0;}
        sz -= r;
        buf += r;
    }
    return 1;	
}

INT32 sck_send(SCK s,VOID* data,INT32 sz)
{
	if ( !_sck_write(s,data,sz) ) {if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_SEND);return 0;}
    
    if (s->fnc.endsend) s->fnc.endsend(s,data,sz);
    return 1;
}

INT32 sck_recv(SCK s)
{
	return _sck_recv(s);
}

INT32 sck_pack(SCK s, INT32 count, INT32 id, INT32 sz, VOID* data)
{
	if ( !_sck_write(s,&sz,4) ) {if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_SEND);return 0;}
	if ( !_sck_write(s,&count,4) ) {if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_SEND);return 0;}
	if ( count > 1 )
	{
		if ( !_sck_write(s,&id,4) ) {if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_SEND);return 0;}
	}
	if ( !_sck_write(s,data,sz) ) {if (s->fnc.eerr) s->fnc.eerr(s,NULL,SCK_ERR_SEND);return 0;}
	
	PACK pk;
	pk.id = id;
	pk.count = count;
	pk.sz = sz;
	pk.data = data;
	
    if (s->fnc.endsend) s->fnc.endsend(s,&pk,-1);
    return 1;
} 

INT32 sck_run(SCK s)
{
	if ( s->mod == SKSERE )
		return _sck_sere_run(s);
		
    thr_run(s->th,s);
    return 1;
}

VOID sck_waitclose(SCK s)
{
    thr_waitthr(s->th);
}

VOID sck_getip(SCK s,CHAR* ip)
{
    strcpy(ip,inet_ntoa(s->inf.adr.sin_addr));
}

VOID sck_remoteip(SCK s,CHAR* ip)
{
    if (s->mod != SKSRV) return;
    strcpy(ip,s->inf.ip);
}

VOID sck_getipbyname(CHAR* host, CHAR* ip)
{
    struct sockaddr_in adr;
    struct hostent* hh;
    memset(&adr,0, sizeof(struct sockaddr_in));

    if((hh = gethostbyname(host)) == NULL) {ip[0]= '\0'; return;}

    memcpy((char *)&adr.sin_addr, hh->h_addr, hh->h_length);
    adr.sin_family = AF_INET;
    strcpy(ip,inet_ntoa(adr.sin_addr));
}

inline INT32 sck_getport(SCK s)
{
    return htons(s->inf.adr.sin_port);
}

inline SCKMODEL sck_model(SCK s)
{
    return s->mod;
}

SCK sck_sfi(SCK s, INT32 index)
{
    if ( !s || s->mod != SKSERVER ) return NULL;

    _SCK* f;
    int i;
    for(f = s->child , i = 0; f && i < index ; f = f->child, ++i);

    return f;
}

SCK sck_enum(SCK server)
{
	static SCK s = NULL;
	
	if ( server ) s = server;
	
	if ( s )
	{
		s = s->child;
		return s;
	}
	
	return NULL;
}

SCK sck_server(SCK my)
{
	return my->server;
}
