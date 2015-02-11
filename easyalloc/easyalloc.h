#ifndef __EASYALLOC_H__
#define __EASYALLOC_H__

#include <malloc.h>
#include <easytype.h>

#define ALC_NOT_REALLOC -1

#define alc_new(var,n) (var*)malloc(sizeof(var)*n)
#define alc_free(var) do{free(var);var=NULL;}while(1)
#define alc_neww(var,y,x) (var**)alc_mallocm(y,x,sizeof(var))

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

VOID alc_freem(void **b,UINT32 y);
void** alc_mallocm(UINT32 y,UINT32 x,size_t st);
void** alc_reallocm(void **b,int oldy,int oldx,int newy,int newx,size_t st);

#endif
