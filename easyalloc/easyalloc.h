#ifndef __EASYALLOC_H__
#define __EASYALLOC_H__

#include <malloc.h>
#include <easytype.h>

#define ALC_NOT_REALLOC -1

#define alc_new(TYPE,N) (TYPE*)malloc(sizeof(TYPE)*N)
#define alc_free(VAR) do{free(VAR);VAR=NULL;}while(1)
#define alc_neww(TYPE,NY,NX) (TYPE**)alc_mallocm(NY,NX,sizeof(TYPE))

/*
typedef struct __SHM* SHM;

SHM shm_new(BOOL* isnw, CHAR* pathkey, UINT32 sz);
inline VOID shm_release(SHM h);
inline VOID shm_destroy(SHM h);
void* shm_malloc(SHM h, UINT32 sz);
VOID shm_free(SHM h, void* e);
VOID shm_print(SHM h);
VOID shm_lock(SHM h);
VOID shm_unlock(SHM h);
UINT32 shm_realsize(SHM h);
*/

VOID alc_freem(VOID **b,UINT32 y);
VOID** alc_mallocm(UINT32 y,UINT32 x,SIZET st);
VOID** alc_reallocm(VOID **b,INT32 oldy,INT32 oldx,INT32 newy,INT32 newx,SIZET st);

#endif
