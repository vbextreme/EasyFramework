#ifndef EASYSOCKET_H_INCLUDED
#define EASYSOCKET_H_INCLUDED

#include <easytype.h>

#define SCK_ERR_CREATE  -99
#define SCK_ERR_BINDING -98
#define SCK_ERR_LISTEN  -97
#define SCK_ERR_ACCEPT  -96
#define SCK_ERR_CONNECT -95
#define SCK_ERR_SOCKET  -94
#define SCK_ERR_READ    -93
#define SCK_ERR_SEND    -92
#define SCK_INVALID     -1
#define SCK_ACCEPTRQ     1
#define SCK_DISCAREGRQ   0

#define SCK_FORCETIMEOUT 10

#define SCK_IP_SIZE      12

#define SCK_BUFFER_SIZE 1024

typedef int HSCK;

typedef struct __SCK* SCK;

typedef struct _PACK
{
	INT32 sz;
	INT32 id;
	INT32 count;
	VOID* data;
}PACK;

typedef enum {SKNULL,SKSERVER,SKCLIENT,SKSRV,SKSERE}  SCKMODEL;
typedef enum {PKDEFAULT,PKPACK}  PKMODEL;

typedef INT32 (*SCKCALL)(SCK,void*,INT32);

SCK sck_cli_new(CHAR* ip,INT32 port);
SCK sck_srv_new(INT32 port);
SCK sck_sc_new(SCK srv,HSCK h,INT32 port);
SCK sck_sere_new(CHAR* ip,INT32 port);
VOID sck_event(SCK s, SCKCALL con, SCKCALL acp, SCKCALL dis, SCKCALL eerr, SCKCALL endsend, SCKCALL incoming);
VOID sck_arg_set(SCK s, VOID* arg, SCKCALL fncfree, BOOL autofree);
VOID* sck_arg_get(SCK s);
VOID sck_buf_set(SCK s, PKMODEL mode, BOOL newb, INT32 rsz, VOID* buf);
VOID* sck_buf_get(SCK s);
INT32 sck_reset(SCK s);
VOID sck_close(SCK s);
VOID sck_free(SCK s);
INT32 sck_send(SCK s,VOID* data,INT32 sz);
INT32 sck_recv(SCK s);
INT32 sck_pack(SCK s, INT32 count, INT32 id, INT32 sz, VOID* data);
INT32 sck_run(SCK s);
VOID sck_waitclose(SCK s);
VOID sck_getip(SCK s,CHAR* ip);
VOID sck_remoteip(SCK s,CHAR* ip);
VOID sck_getipbyname(CHAR* host, CHAR* ip);
inline INT32 sck_getport(SCK s);
inline SCKMODEL sck_model(SCK s);
SCK sck_sfi(SCK s, INT32 index);
SCK sck_enum(SCK server);
SCK sck_server(SCK my);

#endif // EASYSOCKET_H_INCLUDED
