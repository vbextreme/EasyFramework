#ifndef __EASYALLOC_H__
#define __EASYALLOC_H__

//0.000000151

#include <easytype.h>
#include <malloc.h>

#define ALC_NOT_REALLOC -1

#define alc_new(TYPE,N) (TYPE*)malloc(sizeof(TYPE)*N)
#define alc_free(VAR) PPC_MULTILINE_START \
						free(VAR);VAR=NULL; \
					  PPC_MULTILINE_END
					  
#define alc_neww(TYPE,NY,NX) (TYPE**)alc_mallocm(NY,NX,sizeof(TYPE))

VOID alc_freem(VOID **b,UINT32 y);
VOID** alc_mallocm(UINT32 y,UINT32 x,SIZET st);
VOID** alc_reallocm(VOID **b,INT32 oldy,INT32 oldx,INT32 newy,INT32 newx,SIZET st);
inline SIZET alc_rsizeof(SIZET type);

inline VOID* mal_init(register VOID* baseadr,register SIZET szmem);
VOID* mal_malloc(register VOID* baseadr,register SIZET reqmem);
VOID mal_free(register VOID* freeadr);
VOID mal_dbg_mem(VOID* baseadr);
VOID mal_dbg_adr(VOID* wadr);
#endif
