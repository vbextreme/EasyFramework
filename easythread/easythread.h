#ifndef EASYTHREAD_H_INCLUDED
#define EASYTHREAD_H_INCLUDED

#include <easytype.h>

#define THREAD_START(VOIDPARAMNAME,TYPE,NAMEVAR)    THR _this = (THR) VOIDPARAMNAME;\
                                                    thr_startsuspend(_this);\
                                                    TYPE NAMEVAR = (TYPE) thr_getparam(_this)


#define THREAD_END(VOIDRETVAL)     thr_exit(_this,VOIDRETVAL);\
                                   return NULL

#define THREAD_REQUEST(void)    if (thr_chkrequestend(_this)) goto _LRQEXIT;\
                                thr_chkpause(_this)

#define THREAD_TOEXIT   goto _LRQEXIT

#define THREAD_ONEXIT   _LRQEXIT:

#define THR_AUTOCPU 0

#define THR_MUTEN_NAME_MAX 64
#define THR_MUTEN_MAX 43

///il ritorno > 0 è l'attesa in millisecondi
#define THR_WORK_COMPLETE 0
#define THR_WORK_CONTINUE -1
#define THR_WORK_COMPLETE_EXIT -2
#define THR_WORK_SKIP -7
#define THR_WORK_PRIORITY_END -3

#define THR_TALK_MSG_SZ 1024

typedef struct __MUTEX* MUTEX;
typedef struct __MUTEN* MUTEN;
typedef struct __BARRIER* BARRIER;
typedef struct __EVENT* EVENT;
typedef struct __MESSAGE* MESSAGE;
typedef struct __MSGQUEUE* MSGQUEUE;
typedef struct __TALKQUEUE* TALKQUEUE;
typedef struct __WORKER* WORKER;
typedef struct __WORK* WORK;
typedef struct __THR* THR;
typedef struct __JOB* JOB;

typedef VOID*(*THRCALL)(VOID*);
typedef INT32(*WORKCALL)(WORK, VOID*);

typedef enum {T_CREATE = 0,T_RUN,T_PAUSE,T_END,T_REQUESTEXIT} THRMODE;
typedef enum {M_CHAR = 0,M_UCHAR,M_PCHAR,M_INT16,M_UINT16,M_PINT16,M_INT32,M_UINT32,M_PINT32,M_DOUBLE,M_PDOUBLE,M_USER} THRMESSAGE;

/// MUTEX ///
MUTEX thr_mutex_new();
inline VOID thr_mutex_lock(MUTEX m);
inline VOID thr_mutex_unlock(MUTEX m);
VOID thr_mutex_free(MUTEX m);
/// MUTEN ///
MUTEN thr_muten_new(CHAR* filenamespace, CHAR* name);
inline VOID thr_muten_lock(MUTEN m);
inline VOID thr_muten_unlock(MUTEN m);
VOID thr_muten_destroy(CHAR* filenamespace, MUTEN m);
VOID thr_muten_free(MUTEN m);
/// BARRIER ///
BARRIER thr_barrier_new(int nthread);
inline VOID thr_barrier_enter(BARRIER b);
VOID thr_barrier_free(BARRIER b);
/// EVENT ///
EVENT thr_event_new(INT32 autoenter, INT32 autoexit, INT32 resumeall, INT32 value);
VOID thr_event_free(EVENT e);
VOID thr_event_enter(EVENT e);
VOID thr_event_exit(EVENT e);
INT32 thr_event_wait(EVENT e,INT32 timeoutms);
VOID thr_event_raise(EVENT e);
VOID thr_event_reset(EVENT e);
/// MESSAGE ///
MESSAGE thr_message_new(THRMESSAGE type, VOID* data, INT32 autofree);
VOID thr_message_free(MESSAGE m);
THRMESSAGE thr_message_gettype(MESSAGE m);
VOID* thr_message_getmsg(MESSAGE m);
int thr_message_getautofree(MESSAGE m);
/// MSGQUEUE ///
MSGQUEUE thr_queue_new();
INT32 thr_queue_free(MSGQUEUE q);
inline INT32 thr_queue_getsize(MSGQUEUE q);
INT32 thr_queue_add(MSGQUEUE q, MESSAGE m);
MESSAGE thr_queue_getmessage(MSGQUEUE q, UINT32 waitms);
/// TALKQUEUE ///
TALKQUEUE thr_talk_new(CHAR* tpath, INT32 maxask, INT32 maxreply);
VOID thr_talk_free(TALKQUEUE t, CHAR* tpath);
TALKQUEUE thr_talk_hook(CHAR* tpath);
VOID thr_talk_unhook(TALKQUEUE t);
INT32 thr_talk_ask(TALKQUEUE t, VOID* question, INT32 sz, BOOL wantanswer, BOOL forcequestion);
INT32 thr_talk_reply(TALKQUEUE t, INT32 idr, VOID* answer, INT32 sz);
INT32 thr_talk_waitask(TALKQUEUE t, VOID* question, INT32* sz);
INT32 thr_talk_waitanswer(TALKQUEUE t, INT32 idr, VOID* answer, INT32* sz);
VOID thr_talk_arforsize(INT32* nask, INT32* nreply, UINT32 sz);

/// WORKER ///
WORKER thr_worker_new(WORKCALL dowork, WORKCALL progress, WORKCALL complete, WORKCALL tofree, INT32 autofree, VOID* param, INT32 priority);
/// WORK ///
WORK thr_work_new();
INT32 thr_work_free(WORK w);
INT32 thr_work_add(WORK w, WORKER wr);
INT32 thr_work_run(WORK w);
/// THREAD ///
THR thr_new(THRCALL thrcall, UINT32 stksz, INT32 runsuspend, UINT32 oncpu);
INT32 thr_run(THR t,VOID* param);
INT32 thr_free(THR t);
VOID thr_changecpu(THR t, UINT32 oncpu);
inline VOID* thr_getparam(THR t);
VOID* thr_waitthr(THR t);
VOID thr_requestwait(THR t);
VOID thr_resume(THR t);
VOID thr_startsuspend(THR t);
VOID thr_suspendme(THR t);
VOID thr_chkpause(THR t);
INT32 thr_chkrequestend(THR t);
INT32 thr_sleep(FLOAT64 sleep_time);
INT32 thr_msleep(UINT32 ms);
INT32 thr_nsleep(UINT32 ns);
INT32 thr_stop(THR t, UINT32 ms, INT32 forceclose);
VOID thr_exit(THR t, VOID* ret);
UINT32 thr_ncore();
/// JOB ///
JOB thr_job_new(int nthread, THRCALL thrcall, UINT32 stksz);
VOID thr_job_run(JOB j);
VOID thr_job_free(JOB j);
VOID thr_job_wait(JOB j);
INT32 thr_job_stop(JOB j, UINT32 ms, INT32 forceclose);
VOID thr_job_setparam(JOB j, UINT32 index, VOID* p);
/// PROCESS ///
FLOAT64 pro_cpu_usage(INT32 idcpu, FLOAT64 tscan);

#endif // EASYTHREAD_H_INCLUDED
