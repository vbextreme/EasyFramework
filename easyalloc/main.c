#include "easyalloc.h"
#include <pthread.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>

#ifdef _APP
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char** argv)
{
    printf("Test Shared mem!\n");
    
    CHAR* k = "/home/odroid/Croject/lcd";
    
    
    SHM s = shm_new(k,1024);
		if (!s) {puts("Error 1"); return 0;}
	if ( argc == 2 && *argv[1] == 'k' )
	{
		shm_destroy(s);
		puts("destroy");
		return 0;
	}
	
	CHAR* str0 = shm_malloc(s,15);
		if ( str0 )
			strcpy(str0,"ciao mondo");
		else
			puts("Error str0");
	
	shm_print(s);
	fflush(stdout);
			
	CHAR* str1 = shm_malloc(s,15);
		if ( str1 )
			strcpy(str1,"globale");
		else
			puts("Error str1");
	
	shm_print(s);
	printf("1)%s\n2)%s\n", str0,str1);
	fflush(stdout);
	
	if (str0) shm_free(s,str0);
	shm_print(s);
	fflush(stdout);
	
	//if (str1) shm_free(s,str1);
    shm_print(s);
    fflush(stdout);
    
    //shm_destroy(s);
    
    return 0;
}
#endif

/*

typedef struct __SHM
{
	INT32 id;
	CHAR* base;
	CHAR* st;
	UINT32 sz;
	pthread_mutex_t* mx;
	pthread_mutexattr_t* mxatt;
}_SHM;

SHM shm_new(BOOL* isnw, CHAR* pathkey, UINT32 sz)
{
	key_t k = ftok(pathkey,'K');
	if ( k < 0 ) {return NULL;}
	
	_SHM* h = alc_new(_SHM,1);
	
	h->sz = sz;
	
	UINT32 exsz = sizeof(pthread_mutex_t) + sizeof(pthread_mutexattr_t) + 5;
	
	*isnw = FALSE;
	if ( (h->id = shmget(k,sz + exsz,IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) < 0 )
	{
		if ( (h->id = shmget(k,sz + exsz,S_IRUSR | S_IWUSR)) < 0 ) {free(h); return NULL;} 
	}
	else
		*isnw = TRUE;
	
	h->base = shmat(h->id, (void *)0, 0);
		if (h->base == (char *)(-1)) {free(h); return NULL;}
	h->mx = (pthread_mutex_t*) h->base;
	h->mxatt = (pthread_mutexattr_t*)(h->base + sizeof(pthread_mutex_t));
	h->st = h->base + sizeof(pthread_mutex_t) + sizeof(pthread_mutexattr_t);
	
	if ( !*isnw ) return h;
	
	*h->st = 0;
	*(UINT32*)(h->st + 1) = sz;
	
	pthread_mutexattr_init(h->mxatt);
	pthread_mutexattr_setpshared(h->mxatt, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(h->mx,h->mxatt);
	
	return h;
}

VOID shm_release(SHM h)
{
	shmdt(h->base);
	free(h);
}

VOID shm_destroy(SHM h)
{
	pthread_mutex_destroy(h->mx);
	pthread_mutexattr_destroy(h->mxatt); 
	shmdt(h->base);
	shmctl(h->id,IPC_RMID,0);
	free(h);
}

void* shm_malloc(SHM h, UINT32 sz)
{	
	pthread_mutex_lock(h->mx);
	
	CHAR* fs = h->st;
	
	while( 1 )
	{
		while( *fs )
		{			
			fs += *(UINT32*)(fs + 1) + 4;
				if ( fs >= ( h->st + h->sz - 5 ) ){pthread_mutex_unlock(h->mx); return NULL;}
		}
		if ( *(UINT32*)(fs + 1) >= sz ) break;
	}
	
	CHAR* ret = fs + 5;
	
	*fs = 1;
	UINT32 blksz = *(UINT32*)(fs + 1);
	
	if ( blksz <= sz + 6 ) {pthread_mutex_unlock(h->mx);return ret;}
	
	*(UINT32*)(fs + 1) = sz;
	fs += sz + 4;
	*fs = 0;
	*(UINT32*)(fs + 1) = blksz - (sz + 5);
	
	pthread_mutex_unlock(h->mx);
	
	return ret;
}

VOID _shm_align(SHM h)
{
	CHAR* fs = (CHAR*) h->st;
	CHAR* bk = NULL;
	
	while( fs < h->st + h->sz )
	{
		if ( *fs )
			bk = NULL;
		else if ( bk )
			*(UINT32*)(bk + 1) += *(UINT32*)(fs + 1) + 5;
		else
			bk = fs;
		
		fs += *(UINT32*)(fs + 1) + 4;
	}
}

VOID shm_free(SHM h, void* e)
{
	pthread_mutex_lock(h->mx);
	CHAR* fs = (CHAR*) e;
	fs -= 5;
	*fs = 0;
	_shm_align(h);
	pthread_mutex_unlock(h->mx);
}

VOID shm_lock(SHM h)
{
	pthread_mutex_lock(h->mx);
}

VOID shm_unlock(SHM h)
{
	pthread_mutex_unlock(h->mx);
}

UINT32 shm_realsize(SHM h)
{
	struct shmid_ds shmbuffer;
	shmctl (h->id, IPC_STAT, &shmbuffer);
	return shmbuffer.shm_segsz;
}

VOID shm_print(SHM h)
{
	CHAR* fs = (CHAR*) h->st;
	
	printf("SHM:%u\n",h->sz);
	while( fs < h->st + h->sz )
	{
		printf("[%s](%d)\n", (*fs) ? "X" : " ", *(UINT32*)(fs + 1));
		fs += *(UINT32*)(fs + 1) + 4;
	}
}	
*/

void alc_freem(void **b,UINT32 y)
{
    int i;
    for (i=0; i<y ; i++)
        free(b[i]);

    free(b);
}

void** alc_mallocm(UINT32 y, UINT32 x,size_t st)
{
    void **b;

    b=(void**)malloc(y*sizeof(void*));
    if (b==NULL)return NULL;

    int i,ii;

    for (i=0; i<y ;i++)
    {
        b[i]=(void*)malloc(x * st);
        if (b[i] == NULL)
        {
            for (ii=0; ii < i ;ii++)
                free(b[ii]);
            free(b);
            return NULL;
        }
    }

    return b;
}

void** alc_reallocm(void **b,int oldy,int oldx,int newy,int newx,size_t st)
{
    int i;
    void **tmp,*stmp;

    if (newy != ALC_NOT_REALLOC)
    {
        tmp=(void**)realloc(b,sizeof(void*)*newy);
            if (tmp == NULL) return NULL;

        if (newy > oldy)
        {
            for (i=oldy; i < newy ;i++)
                b[i]=(void*)malloc(oldx*st);
            oldy=newy;
        }
    }

    if(newx != ALC_NOT_REALLOC)
    {
        for (i=0; i < oldy ;i++)
        {
            stmp=realloc(b[i],st*newx);
                if (stmp == NULL) return NULL;
            b[i]=stmp;
        }
    }

    return b;
}
