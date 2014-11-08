///Testare le nuove funzionalità di work ok

#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include "easythread.h"
#include <sys/time.h>
#include <time.h>

#define _dbgd(VAR) printf( #VAR ":=%d\n",VAR)

VOID* test(VOID* val)
{
	THREAD_START(val,INT32*,id);
	
	while(1)
	{
		THREAD_REQUEST();
		
		//printf("...test:%d...\n",*id);
		//fflush(stdout);
		//thr_sleep(0.3);
	}
	
	THREAD_ONEXIT
	return NULL;
}


INT32 main()
{
	
	printf("ncore:%u\n",thr_ncore());
	
	INT32 a = 1,b = 2;
	
	THR t0 = thr_new(test,0,0,12);
	THR t1 = thr_new(test,0,0,34);
	
	thr_run(t0,&a);
	thr_run(t1,&b);
	
	thr_sleep(5.0);
	
	thr_stop(t0,1000,1);
	thr_stop(t1,1000,1);
	
	thr_free(t0);
	thr_free(t1);
	
	return 0;
}


#endif
/*
double start;

typedef struct _TEST
{
    int max;
    int sleep;
    int ex;
    double offset;
}TEST;

static double _bch_get()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

static double _bch_clc(double st,double en)
{
    return en-st;
}

int completato(WORK w,void* p)
{
    TEST* t = p;
    int ret = (t->ex==2) ? THR_WORK_COMPLETE_EXIT : THR_WORK_COMPLETE;
    free(t);
    return ret;
}

int esempio(WORK w,void* p)
{
    TEST* t = p;

    double at = _bch_get();
    if (t->max == 0) t->offset = _bch_clc(start,at);
    ++t->max;

    if (t->max >1)
    {
        printf("[%d]%d)%lf\n",t->ex,t->max,(_bch_clc(start,at)-t->offset) / (t->max-1));
        fflush(stdout);
    }

    if ( t->ex != 1 && t->max == 10)
        return THR_WORK_COMPLETE;

    return t->sleep;
}

void* mt(void* p)
{
    THREAD_START(p,WORK,w);

    thr_work_run(w);

    THREAD_END(NULL);
}

int main()
{
    THR t = thr_new(mt,0,0,0);

    WORK w = thr_work_new(1);

    start = _bch_get();

    thr_run(t,w);

    TEST *v = malloc(sizeof (TEST));
    v->max = 0;
    v->sleep = 1;
    v->ex = 0;
    thr_work_add(w,thr_worker_new(esempio,NULL,completato,NULL,0,v,1));

    v = malloc(sizeof (TEST));
    v->max = 0;
    v->sleep = THR_WORK_CONTINUE;
    v->ex = 1;
    thr_work_add(w,thr_worker_new(esempio,NULL,completato,NULL,0,v,1));

    v = malloc(sizeof (TEST));
    v->max = 0;
    v->sleep = 2;
    v->ex = 2;
    thr_work_add(w,thr_worker_new(esempio,NULL,completato,NULL,0,v,1));

    thr_waitthr(t);

    thr_work_free(w);

    _dbgd(thr_free(t));

    return 0;
}




#endif // _DEBUG
*/
/*

void* mt(void* p)
{
    THREAD_START(p,MSGQUEUE,msq);

    MESSAGE msg;
    int* v;
    while ( (msg = thr_queue_getmessage(msq,1)) )
    {

        v = (int*) thr_message_getmsg(msg);
        printf("Rimangono:%d\n",thr_queue_getsize(msq));
        thr_sleep(1);
        switch (*v)
        {
            case 0:
                printf("Good bye\n");
                thr_message_free(msg);
                goto ENDQUEUE;
            break;
            case 1:
                printf("Hello world\n");
                thr_message_free(msg);
            break;
            default:
                printf("Inaspettato\n");
            break;
        }
    }

    ENDQUEUE:

    THREAD_END(NULL);
}

int main()
{
    THR t = thr_new(mt,0,0);

    MSGQUEUE msq = thr_queue_new();

    thr_run(t,msq);

    int i;
    int *v;
    for (i = 0; i < 1000 ; i++)
    {
        thr_sleep(1);
        v = malloc(sizeof (int));
        *v = 1;
        thr_queue_add(msq,thr_message_new(M_INT32,v,1));
    }

    v = malloc(sizeof (int));
    *v = 0;
    thr_queue_add(msq,thr_message_new(M_INT32,v,1));


    thr_waitthr(t);

    _dbgd(thr_free(t));

    return 0;
}

*/
