#define _GNU_SOURCE

#include "easythread.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <sys/sysctl.h>
#include <sched.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>

typedef struct __MUTEX
{
    pthread_mutex_t mutex;
}_MUTEX;

typedef struct __MUTEN
{
	INT32 idsh;
	CHAR* base;
	pthread_mutex_t* mx;
	pthread_mutexattr_t* att;
}_MUTEN;

typedef struct __BARRIER
{
    pthread_barrier_t barrier;
}_BARRIER;

typedef struct __EVENT
{
    MUTEX mutex;
    pthread_cond_t condition;
    INT32 autoenter;
    INT32 autoexit;
    INT32 value;
    INT32 resumeall;
}_EVENT;

typedef struct __MESSAGE
{
    THRMESSAGE type;
    VOID* msg;
    INT32 autofree;
    struct __MESSAGE* next;
}_MESSAGE;

typedef struct __MSGQUEUE
{
    _MESSAGE* first;
    _MESSAGE* last;
    _MESSAGE* current;

    MUTEX safeins;
    EVENT havemsg;
    INT32 szcoda;
}_MSGQUEUE;

typedef struct __WORKER
{
    INT32 autofree;
    VOID* param;

    INT32 priority;
    INT32 priostat;

    FLOAT64 timer;
    FLOAT64 elapse;

    WORKCALL dowork;
    WORKCALL onprogress;
    WORKCALL oncomplete;
    WORKCALL tofree;

    struct __WORKER* next;
    struct __WORKER* prev;
}_WORKER;

typedef struct __WORK
{
    _WORKER* first;
    _WORKER* last;
    _WORKER* current;

    MUTEX safeins;
    EVENT havework;
    INT32 timemode;
}_WORK;

typedef struct __THR
{
    pthread_t id;
    pthread_attr_t att;
    THRCALL fnc;
    INT32 runsuspend;
    EVENT suspend;
    EVENT finish;
    VOID* param;
    THRMODE stato;
}_THR;

typedef struct __JOB
{
    THR* j;
    INT32 n;
}_JOB;

/// /////// ///
/// SUPPORT ///
/// /////// ///

static FLOAT64 _bch_get()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

static inline FLOAT64 _bch_clc(double st,double en)
{
    return en-st;
}

static cpu_set_t* _setcpu(UINT32 mcpu)
{
	static cpu_set_t cpu;
	CPU_ZERO(&cpu);
	
	if (mcpu == 0 ) return &cpu;
	
	UINT32 s;
	while ( (s = mcpu % 10) )
	{
		CPU_SET(s - 1,&cpu);
		mcpu /= 10;
	}
	
	return &cpu;
}

/// ///// ///
/// MUTEX ///
/// ///// ///

MUTEX thr_mutex_new()
{
    _MUTEX* m= malloc (sizeof(_MUTEX));
    pthread_mutex_init(&m->mutex,NULL);
    return m;
}

inline VOID thr_mutex_lock(MUTEX m)
{
    pthread_mutex_lock(&m->mutex);
}

inline VOID thr_mutex_unlock(MUTEX m)
{
    pthread_mutex_unlock(&m->mutex);
}

VOID thr_mutex_free(MUTEX m)
{
    pthread_mutex_destroy(&m->mutex);
    free(m);
}

/// ///// ///
/// MUTEN ///
/// ///// ///

MUTEN thr_muten_new(CHAR *phname, UINT32 offset, UINT32 n)
{
	_MUTEN* mx = malloc(sizeof(_MUTEN));
	
	key_t k = ftok(phname,'M');
	if ( k < 0 ) {free(mx); return NULL;}
	
	UINT32 shsz = (sizeof(pthread_mutex_t) + sizeof(pthread_mutexattr_t)) * n;
	
	if ( (mx->idsh = shmget(k,shsz,IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) < 0 )
	{
		if ( (mx->idsh = shmget(k,shsz,S_IRUSR | S_IWUSR)) < 0 ) {free(mx); return NULL;}
		mx->base = shmat(mx->idsh, (void *)0, 0);
		if (mx->base == (char *)(-1)) {free(mx); return NULL;}
		mx->mx =(pthread_mutex_t*)(mx->base + offset);
		mx->att =(pthread_mutexattr_t*)( mx->base + sizeof(pthread_mutex_t) + offset);
		return mx; 
	}
	
	mx->base = shmat(mx->idsh, (void *)0, 0);
	if (mx->base == (char *)(-1)) {free(mx);return NULL;}
	mx->mx =(pthread_mutex_t*)(mx->base + offset);
	mx->att =(pthread_mutexattr_t*)( mx->base + sizeof(pthread_mutex_t) + offset);
	
	pthread_mutexattr_init(mx->att);
	pthread_mutexattr_setpshared(mx->att, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(mx->mx,mx->att);
	return mx; 
}

inline VOID thr_muten_lock(MUTEN m)
{
    pthread_mutex_lock(m->mx);
}

inline VOID thr_muten_unlock(MUTEN m)
{
    pthread_mutex_unlock(m->mx);
}

VOID thr_muten_destroy(MUTEN mx)
{
	pthread_mutex_destroy(mx->mx);
	pthread_mutexattr_destroy(mx->att); 
}

VOID thr_muten_release(MUTEN mx)
{
	shmdt(mx->base);
	shmctl(mx->idsh,IPC_RMID,0);
	free(mx);
}

VOID thr_muten_free(MUTEN mx)
{
	shmdt(mx->base);
	free(mx);
}

inline UINT32 thr_muten_sz()
{
	return sizeof(_MUTEN);
}
/// /////// ///
/// BARRIER ///
/// /////// ///

BARRIER thr_barrier_new(int nthread)
{
    _BARRIER* b = malloc (sizeof(_BARRIER));
    pthread_barrier_init(&b->barrier,NULL,nthread);
    return b;
}

inline VOID thr_barrier_enter(BARRIER b)
{
    pthread_barrier_wait(&b->barrier);
}

VOID thr_barrier_free(BARRIER b)
{
    pthread_barrier_destroy(&b->barrier);
    free(b);
}

/// ////// ///
/// EVENTO ///
/// ////// ///

EVENT thr_event_new(INT32 autoenter,INT32 autoexit,INT32 resumeall,INT32 value)
{
    _EVENT* e= malloc (sizeof(_EVENT));
    e->autoenter = autoenter;
    e->autoexit = autoexit;
    e->resumeall = resumeall;
    e->value = 0;
    e->mutex = thr_mutex_new();
    pthread_cond_init(&e->condition, NULL);
    return e;
}

VOID thr_event_free(EVENT e)
{
    thr_mutex_free(e->mutex);
    pthread_cond_destroy(&e->condition);
    free(e);
}

VOID thr_event_enter(EVENT e)
{
    if (e->autoenter) return;
    thr_mutex_lock(e->mutex);
}

VOID thr_event_exit(EVENT e)
{
    if (e->autoexit) return;
    thr_mutex_unlock(e->mutex);
}

INT32 thr_event_wait(EVENT e,INT32 timeoutms)
{
    if (e->autoenter)
        thr_mutex_lock(e->mutex);

    if (!e->value)
    {
        if (!timeoutms)
        {
            pthread_cond_wait(&e->condition, &e->mutex->mutex);
        }
        else
        {
            struct timespec   ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += timeoutms * 1000;


            INT32 ret = pthread_cond_timedwait(&e->condition, &e->mutex->mutex, &ts);
            if (ret)
            {
                ret = 0;
                thr_mutex_unlock(e->mutex);
                return 0;
            }
        }

    }

    if (e->autoexit)
    {
        thr_mutex_unlock(e->mutex);
    }

    return 1;
}

VOID thr_event_raise(EVENT e)
{
    thr_mutex_lock(e->mutex);
        e->value = 1;


    if (e->resumeall)
        pthread_cond_broadcast(&e->condition);
    else
    {
        pthread_cond_signal(&e->condition);
    }

    thr_mutex_unlock(e->mutex);
}

VOID thr_event_reset(EVENT e)
{
    thr_mutex_lock(e->mutex);
    e->value = 0;
    thr_mutex_unlock(e->mutex);
}

/// /////// ///
/// MESSAGE ///
/// /////// ///

MESSAGE thr_message_new(THRMESSAGE type,VOID* data,INT32 autofree)
{
    _MESSAGE* m = (_MESSAGE*)malloc(sizeof(_MESSAGE));
    m->type = type;
    m->msg = data;
    m->autofree = autofree;
    m->next = NULL;
    return m;
}

VOID thr_message_free(MESSAGE m)
{
    if ( m == NULL ) return;
    if (m->autofree && m->msg != NULL) free(m->msg);
    free(m);
}

THRMESSAGE thr_message_gettype(MESSAGE m) { return m->type; }
VOID* thr_message_getmsg(MESSAGE m) { return m->msg; }
INT32 thr_message_getautofree(MESSAGE m) { return m->autofree; }

/// //////// ///
/// MSGQUEUE ///
/// //////// ///

MSGQUEUE thr_queue_new()
{
    _MSGQUEUE* q = (_MSGQUEUE*)malloc(sizeof(_MSGQUEUE));

    q->szcoda = 0;
    q->first = NULL;
    q->last = NULL;
    q->current = NULL;


    q->safeins = thr_mutex_new();
    q->havemsg = thr_event_new(1,1,0,0);

    return q;
}

INT32 thr_queue_free(MSGQUEUE q)
{
    if (q == NULL) return 0;

    for ( ; q->first != NULL ; q->first = q->current)
    {
        q->current = q->first->next;
        thr_message_free(q->first);
    }

    thr_mutex_free(q->safeins);
    thr_event_free(q->havemsg);
    free(q);

    return 0;
}

inline INT32 thr_queue_getsize(MSGQUEUE q) {return q->szcoda;}

INT32 thr_queue_add(MSGQUEUE q,MESSAGE m)
{
    thr_mutex_lock(q->safeins);

    m->next = NULL;

    if (q->first == NULL)
    {
        q->first = m;
        q->last = m;
    }
    else
    {
        q->last->next = m;
        q->last = m;
    }


    ++q->szcoda;

    thr_mutex_unlock(q->safeins);
    thr_event_raise(q->havemsg);
    return 0;
}

MESSAGE thr_queue_getmessage(MSGQUEUE q, UINT32 waitms)
{
    if (q == NULL) return NULL;

    if (waitms && q->szcoda <= 0)
    {
        thr_event_wait(q->havemsg,waitms);
        thr_event_reset(q->havemsg);
    }

    thr_mutex_lock(q->safeins);

    _MESSAGE* r;

    if (q->first == NULL)
    {
        r = NULL;
    }
    else
    {
        r = q->first;
        if (q->first == q->last)
        {
            q->first = NULL;
            q->last = NULL;
        }
        else
        {
            q->first = q->first->next;
        }
        --q->szcoda;
    }
    thr_mutex_unlock(q->safeins);

    return r;
}

/// ////// ///
/// WORKER ///
/// ////// ///

WORKER thr_worker_new(WORKCALL dowork,WORKCALL progress,WORKCALL complete,WORKCALL tofree,INT32 autofree,VOID* param,INT32 priority)
{
    _WORKER* w = (_WORKER*)malloc(sizeof(_WORKER));

    w->dowork = dowork;
    w->onprogress = progress;
    w->oncomplete = complete;
    w->tofree = tofree;

    w->autofree = autofree;
    w->param = param;

    w->priority = priority;
    w->priostat = 0;

    w->timer = 0.0;
    w->elapse = 0.0;

    w->next = NULL;
    w->prev = NULL;

    return w;
}

/// //// ///
/// WORK ///
/// //// ///

WORK thr_work_new()
{
    _WORK* w = (_WORK*)malloc(sizeof(_WORK));

    w->first = NULL;
    w->last = NULL;
    w->current = NULL;
    w->timemode = 0;

    w->safeins = thr_mutex_new();
    w->havework = thr_event_new(1,1,0,0);

    return w;
}

INT32 thr_work_free(WORK w)
{
    if (w == NULL) return 0;
    thr_mutex_lock(w->safeins);

    for ( ; w->first != NULL ; w->first = w->current)
    {
        w->current = w->first->next;
        if (w->first->autofree)
            {if (w->first->param) free(w->first->param);}
        else if (w->first->tofree)
            w->first->tofree(NULL,w->first->param);

        free(w->first);
    }

    thr_mutex_unlock(w->safeins);
    thr_mutex_free(w->safeins);
    thr_event_free(w->havework);
    free(w);

    return 0;

}

INT32 thr_work_add(WORK w,WORKER wr)
{
    thr_mutex_lock(w->safeins);

    wr->next = NULL;
    wr->prev = NULL;
    if (wr->timer == 0.0) ++w->timemode;

    if (w->first == NULL)
    {
        w->first = wr;
        w->last = wr;
        wr->priostat = wr->priority;
    }
    else
    {
        --wr->priostat;
        if (wr->priostat > 0)
        {
            _WORKER* i;

            for (i = w->first; i  != NULL && wr->priority <= i->priority; i = i->next);

            if (i == NULL)
            {
                wr->prev = w->last;
                w->last->next = wr;
                w->last = wr;
            }
            else
            {
                if (i->prev != NULL)
                    i->prev->next = wr;
                else
                    w->first = wr;

                wr->prev = i->prev;
                i->prev = wr;
                wr->next = i;
            }
        }
        else
        {
            wr->prev = w->last;
            w->last->next = wr;
            w->last = wr;
            wr->priostat = wr->priority;
        }
    }

    thr_event_raise(w->havework);
    thr_mutex_unlock(w->safeins);


    return 0;
}

INT32 thr_work_run(WORK w)
{
    if (w == NULL) return -1;

    w->current = NULL;
    INT32 retw;
    INT32 mss = 0;

    while (1)
    {
        thr_event_wait(w->havework,mss);

        thr_mutex_lock(w->safeins);
            if (w->first == NULL)
            {
                mss = 0;
                thr_event_reset(w->havework);
                thr_mutex_unlock(w->safeins);
                continue;
            }

            if (!w->timemode)
            {
                FLOAT64 minsleep=99999.0;
                FLOAT64 ca;
                _WORKER* i;

                for (i = w->first ; i != NULL ; i = i->next)
                {
                    ca = i->timer - _bch_clc(i->elapse,_bch_get());
                    if ( ca < minsleep)
                    {
                        minsleep = ca;
                    }
                }

                if (ca >= 0.0)
                {
                    mss = ca * 1001.0;
                    if (mss < 1) mss = 1;
                    thr_event_reset(w->havework);
                }
            }
            else
            {
                mss = 0;
            }

            w->current = w->first;
            w->first = w->first->next;
            if (w->first != NULL)
                w->first->prev = NULL;
            else
            {
                mss = 0;
                thr_event_reset(w->havework);
            }
            if (w->current->timer == 0.0) --w->timemode;


        thr_mutex_unlock(w->safeins);

        if (w->current->priority == THR_WORK_PRIORITY_END) break;

        if ( w->current->timer > 0.0 )
        {
            if ( _bch_clc(w->current->elapse,_bch_get()) >= w->current->timer)
            {
                w->current->timer = 0.0;
                if (w->current->dowork != NULL)
                    retw = w->current->dowork(w,w->current->param);
                else
                    retw = THR_WORK_COMPLETE;
            }
            else
            {
                retw = THR_WORK_SKIP;
            }
        }
        else
        {
            if (w->current->dowork != NULL)
                retw = w->current->dowork(w,w->current->param);
            else
                retw = THR_WORK_COMPLETE;
        }

        if (retw > 0)
        {
            w->current->elapse = _bch_get();
            w->current->timer = (double)(retw) / 1000.0;
            if (w->current->onprogress != NULL)
                w->current->onprogress(w,w->current->param);

            thr_work_add(w,w->current);
        }
        else if (retw == THR_WORK_COMPLETE)
        {
            if (w->current->oncomplete != NULL)
                retw = w->current->oncomplete(w,w->current->param);

            if (w->current->autofree)
                {if (w->current->param) free(w->current->param);}
            else if (w->current->tofree != NULL)
                w->current->tofree(NULL,w->current->param);
            free(w->current);
            w->current = NULL;
            if (retw == THR_WORK_COMPLETE_EXIT)
                break;
        }
        else if (retw == THR_WORK_CONTINUE)
        {
            if (w->current->onprogress != NULL)
                w->current->onprogress(w,w->current->param);

            thr_work_add(w,w->current);
        }
        else if (retw == THR_WORK_SKIP)
        {
            thr_work_add(w,w->current);
        }
        else
        {
            if (w->current->autofree)
                {if (w->current->param) free(w->current->param);}
            else if (w->current->tofree != NULL)
            {
                w->current->tofree(NULL,w->current->param);
            }
            free(w->current);
            break;
        }

    }

    return 0;
}


/// ////// ///
/// THREAD ///
/// ////// ///

THR thr_new(THRCALL thrcall, UINT32 stksz, INT32 runsuspend, UINT32 oncpu)
{
    _THR* thr = (_THR*) malloc(sizeof(_THR));

    thr->stato = T_CREATE;
    thr->fnc = thrcall;

    pthread_attr_init(&thr->att);
    if ( stksz > 0 ) pthread_attr_setstacksize (&thr->att, stksz);
	
	if ( oncpu > 0 )
	{
		cpu_set_t* ncpu = _setcpu(oncpu);
		pthread_attr_setaffinity_np(&thr->att,CPU_SETSIZE,ncpu);
	}
	
    thr->runsuspend = runsuspend;
    thr->suspend = thr_event_new(1,1,0,0);
    thr->finish = thr_event_new(1,1,0,0);

    return thr;
}

INT32 thr_run(THR t,VOID* param)
{

    if (t->stato > T_CREATE && t->stato < T_END) return 0;

    t->stato = T_RUN;
    t->param = param;
    thr_event_reset(t->suspend);
    thr_event_reset(t->finish);
    pthread_create(&t->id,&t->att,t->fnc,(void*)t);
    return 1;
}

INT32 thr_free(THR t)
{
    if (t->stato != T_END) return 0;

    thr_event_free(t->suspend);
    thr_event_free(t->finish);
    pthread_attr_destroy(&t->att);
    free(t);
    return 1;
}

VOID thr_changecpu(THR t, UINT32 oncpu)
{
	if ( oncpu > 0 )
	{
		cpu_set_t* ncpu = _setcpu(oncpu);
		pthread_attr_setaffinity_np(&t->att,CPU_SETSIZE,ncpu);
	}
}

inline VOID* thr_getparam(THR t)
{
    return t->param;
}

VOID* thr_waitthr(THR t)
{
    void* ret;
    pthread_join(t->id,&ret);
    return ret;
}

VOID thr_requestwait(THR t)
{
    if (t->stato != T_RUN) return;
    t->stato = T_PAUSE;
}

VOID thr_resume(THR t)
{
    if (t->stato != T_PAUSE) return;
    thr_event_raise(t->suspend);
}

VOID thr_startsuspend(THR t)
{
    if (!t->runsuspend) return;
    thr_suspendme(t);
}

VOID thr_suspendme(THR t)
{
    t->stato = T_PAUSE;

    thr_event_wait(t->suspend,0);
    thr_event_reset(t->suspend);
    t->stato = T_RUN;

}

VOID thr_chkpause(THR t)
{
    if (t->stato != T_PAUSE) return;
    thr_suspendme(t);
}

INT32 thr_chkrequestend(THR t)
{
    if (t->stato != T_REQUESTEXIT) return 0;
    return 1;
}

INT32 thr_sleep(FLOAT64 sleep_time)
{
	struct timespec tv;

	tv.tv_sec = (time_t) sleep_time;

	tv.tv_nsec = (long) ((sleep_time - tv.tv_sec) * 1e+9);

	while (1)
	{
		int rval = nanosleep (&tv, &tv);
		if (rval == 0)
			return 0;
		else if (errno == EINTR)
			continue;
		else
			return rval;
	}
	return 0;
}

INT32 thr_msleep(UINT32 ms)
{
	FLOAT64 rs = (FLOAT64)(ms) / 1000.0;
    return thr_sleep(rs);
}

INT32 thr_nsleep(UINT32 ns)
{
	FLOAT64 rs = (FLOAT64)(ns) / 1000000.0;
    return thr_sleep(rs);
}


INT32 thr_stop(THR t, UINT32 ms, INT32 forceclose)
{
    if (t->stato == T_END) return 1;
    if (t->stato != T_RUN) return 0;
    THRMODE old = t->stato;
    t->stato = T_REQUESTEXIT;
    int tr = thr_event_wait(t->finish,ms);
    if (!tr)
    {
        if (forceclose)
        {
            pthread_cancel(t->id);
            t->stato = T_END;
        }
        else
        {
            t->stato = old;
            return 0;
        }
    }
    thr_event_reset(t->finish);
    return 1;
}

VOID thr_exit(THR t,VOID* ret)
{
    thr_event_raise(t->finish);
    t->stato=T_END;
    pthread_exit(ret);
}

UINT32 thr_ncore()
{
	INT32 ncore = sysconf( _SC_NPROCESSORS_ONLN );
	return (ncore <= 0 ) ? 1 : ncore;
}

/// /// ///
/// JOB ///
/// /// ///

JOB thr_job_new(INT32 nthread, THRCALL thrcall, UINT32 stksz)
{
    _JOB* j= (_JOB*)malloc(sizeof(JOB));

    j->n = nthread;

    j->j = (THR*)malloc(sizeof(THR)*j->n);
    INT32 i;
    for (i=0 ; i < j->n ; i++)
    {
        j->j[i] = thr_new(thrcall,stksz,1,0);
        j->j[i]->param = NULL;
    }

    return j;
}

VOID thr_job_run(JOB j)
{
    INT32 i;
    for (i = 0; i < j->n; i++)
    {
        thr_run(j->j[i],j->j[i]->param);
    }
}

VOID thr_job_free(JOB j)
{
    INT32 i;
    for (i = 0 ; i < j->n ; i++)
    {
        thr_free(j->j[i]);
    }
    free(j->j);
    free(j);
}

VOID thr_job_wait(JOB j)
{
    INT32 i;
    for (i = 0; i < j->n; i++)
    {
        thr_waitthr(j->j[i]);
    }
}

INT32 thr_job_stop(JOB j, UINT32 ms, INT32 forceclose)
{
    INT32 i;
    for (i = 0; i < j->n ; i++)
    {
        if (!thr_stop(j->j[i],ms,forceclose))
            return 0;
    }

    return 1;
}

VOID thr_job_setparam(JOB j, UINT32 index, VOID* p)
{
    if (index >= j->n) return;
    j->j[index]->param = p;
}

/// /////// ///
/// PROCESS ///
/// /////// ///
 
static INT32 _procstat_getbufferfields (FILE *fp, CHAR* buffer, UINT32 bf_sz, INT32 idcpu)
{
	fseek (fp, 0, SEEK_SET);
    fflush (fp);
    *buffer = '\0';
	INT32 i;
	for ( i = -1; i < idcpu; ++i)
		if ( !fgets(buffer, bf_sz, fp) ) return 0;
	return 1;
}

static VOID _procstat_fields (CHAR* buffer, UINT32 *user, UINT32* nice, UINT32* system, UINT32* idle, UINT32* iowait,UINT32* irq, UINT32* softirq, UINT32* steal, UINT32* guest, UINT32* guest_nice)
{
	CHAR* tk = strtok(buffer," ");
	tk = strtok(NULL, " ");
	*user = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*nice = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*system = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*idle = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*iowait = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*irq = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*softirq = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*steal = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*guest = (UINT32)atoll(tk);
	tk = strtok(NULL," ");
	*guest_nice =(UINT32)atoll(tk);
}

FLOAT64 pro_cpu_usage(INT32 idcpu, FLOAT64 tscan)
{
	FILE *fp;
	
	UINT32 us,ni,sy,idle,io,ir,si,st,gu,gn;
	UINT32 st_tick,en_tick,st_idle,en_idle;
	
	fp = fopen ("/proc/stat", "r");
		if (fp == NULL) return -1.0;
	
	CHAR buf[1024];
	
	if ( !_procstat_getbufferfields(fp,buf,1024,idcpu) ) {fclose(fp); return -2.0;}
	
	_procstat_fields(buf,&us,&ni,&sy,&idle,&io,&ir,&si,&st,&gu,&gn);
	st_idle = idle;
	st_tick = us + ni + sy + idle + io + ir + si + st + gu + gn;
	
	thr_sleep(tscan);
	
	if ( !_procstat_getbufferfields(fp,buf,1024,idcpu) ) {fclose(fp); return -2.0;}
	
	_procstat_fields(buf,&us,&ni,&sy,&idle,&io,&ir,&si,&st,&gu,&gn);
	en_idle = idle;
	en_tick = us + ni + sy + idle + io + ir + si + st + gu + gn;
	
	en_tick -= st_tick;
	en_idle -= st_idle;
	
	fclose(fp);
	
	return ((en_tick - en_idle) / (FLOAT64) en_tick) * 100.0;
}
