#ifndef __EASYSERIAL_H__
#define __EASYSERIAL_H__

//chmod a+rw /dev/ttyS1

#include <easytype.h>

#define SRL_MAX_INP 256

//CANONICAL read to NL, EOF or EOL

typedef enum {SRL_CANONICAL,SRL_ACANONICAL,SRL_NONCANONICAL,SRL_ANONCANONICAL}SRLMODE;

typedef struct __HSRL* HSRL;

HSRL srl_open(CHAR* p, SRLMODE m, UINT32 baund, UINT32 bit, UINT32 parity, UINT32 bstop, UINT32 timeout, UINT32 nchar);
UINT32 srl_read(HSRL h,VOID* data,UINT32 sz);
UINT32 srl_write(HSRL h,const VOID* data,UINT32 sz);
VOID srl_close(HSRL h);


#endif
