#ifndef __EF_THREADS_H__
#define __EF_THREADS_H__

#include <ef/type.h>
#include <ef/list.h>
#include <pthread.h>

/*************/
/*** futex ***/
/*************/

/** waiting until a certain condition becomes true
 * @param uaddr argument points to the futex word
 * @param futex_op operation perform on the futex
 * @param val is a value whose meaning and purpose depends on futex_op
 * @param timeout specifies a timeout for the operation
 * @param uaddr2 Where it is required, is a pointer to a second futex word that is employed by the operation.
 * @param val3
 * @return -1 for error, 0 on FUTEX_WAIT, N waiters on FUTEX_WAKE, fd for FUTEX_FD, ...
 */
int futex_to(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3);

/** waiting until a certain condition becomes true
 * @param uaddr argument points to the futex word
 * @param futex_op operation perform on the futex
 * @param val is a value whose meaning and purpose depends on futex_op
 * @param val2 depend on operation
 * @param uaddr2 Where it is required, is a pointer to a second futex word that is employed by the operation.
 * @param val3
 * @return -1 for error, 0 on FUTEX_WAIT, N waiters on FUTEX_WAKE, fd for FUTEX_FD, ...
 */
int futex_v2(int *uaddr, int futex_op, int val, unsigned val2, int *uaddr2, int val3);

/** overloading for futex*/
#define futex(ADDR, OP, VAL, V2TO, ADDR2, VAL3) _Generic((V2TO),\
	const struct timespec*: futex_to,\
	void*: futex_to,\
	unsigned: futex_v2\
)(ADDR, OP, VAL, V2TO, ADDR2, VAL3)	

/*************/
/*** mutex ***/
/*************/

typedef struct mutex{
	int futex;
	int private;
	int fd;
}mutex_s;

/** init a mutex if you have create without mutex_new*/
void mutex_init(mutex_s* mtx);

/** create new mutex */
mutex_s* mutex_new();

/** free mutex, only mutex create with mutex new*/
void mutex_free(mutex_s* mtx);

/** unlock mutex */
void mutex_unlock(mutex_s* mtx);

/** lock mutex */
void mutex_lock(mutex_s* mtx);

/** try to lock mutex 
 * @param mtx mutex
 * @return 0 if lock mutex, -1 if other thread have locked the mutex
 */
int mutex_trylock(mutex_s* mtx);

/** lock and use fd for wait with epoll, when raised event call mutex_fd_event for complete operation
 * @param mtx mutex
 * @return fd or -1 acquired
 */
int mutex_fd_lock(mutex_s* mtx);

/** when exit fronm epoll call this, -1 acquire a lock, remember to unlock before reenter in epoll
 * @param mtx mutex
 * @return  if locked is acquired return -1 otherwise return fd
 */
int mutex_fd_event(mutex_s* mtx);

/*****************/
/*** semaphore ***/
/*****************/

typedef struct semaphore{
	int futex;
	int private;
	int fd;
}semaphore_s;

/** init a semaphore if you have create without semaphore_new*/
void semaphore_init(semaphore_s* sem, int val);

/** create new semaphore */
semaphore_s* semaphore_new(int val);

/** free semaphore, only semaphore create with semaphore_new*/
void semaphore_free(semaphore_s* sem);

/** increment by 1 the sempahore, if sem is 0 wake 1 thread */
void semaphore_post(semaphore_s* sem);

/** decrement by 1 the semaphore, if sem is 0 call function started wait thread*/
void semaphore_wait(semaphore_s* sem);

/** try to decrement semaphore, if semaphore is 0 return -1 and not wait
 * @param sem semaphore
 * @return 0 if sem is decremented -1 otherwise
 */
int semaphore_trywait(semaphore_s* sem);

/** try to decrement semaphore but wait in epoll if needed, call semaphore_fd_event() when event raised on epoll
 * @param sem semaphore
 * @return fd or -1 acquired
 */
int semaphore_fd_wait(semaphore_s* sem);

/** if wait over semaphore, when event is raised on epoll call this function for check you can procede with semaphore or need to reenter to epoll
 * @param sem semaphore
 * @return fd for reenter to epoll, -1 you can continue
 */
int semaphore_fd_event(semaphore_s* sem);

/*************/
/*** event ***/
/*************/

typedef struct event{
	int futex;
	int private;
	int fd;
}event_s;

/** init a event if you have create without event_new*/
void event_init(event_s* ev);

/** create new event*/
event_s* event_new();

/** free semaphore, only event create with semaphore_new*/
void event_free(event_s* ev);

/** wake all thread that waited on this event */
void event_raise(event_s* ev);

/** wait a event raised */
void event_wait(event_s* ev);

/** try to wait event in epoll if needed, call event_fd_event() when event raised on epoll
 * @param ev event
 * @return fd or -1 acquired
 */
int event_fd_wait(event_s* ev);

/** if wait over event, when event is raised on epoll call this function for check you can procede with event or need to reenter to epoll
 * @param ev event
 * @return fd for reenter to epoll, -1 event is raised
 */
int event_fd_event(event_s* ev);

/***************/
/*** message ***/
/***************/

typedef struct message message_s;

typedef struct message{
	int id;
	int type;
	void* data[0];
}message_s;

typedef struct qmessages{
	message_s* queue;
	semaphore_s* sem;
	mutex_s* mtx;
}qmessages_s;

/** init a queue if you have create without qmessages_new*/
void qmessages_init(qmessages_s* q, semaphore_s* sem, mutex_s* mtx);

/** create new queue messages*/
qmessages_s* qmessages_new();

/** free queue messages */
void qmessage_free(qmessages_s* q);

/** wait for new messages
 * @param q queue
 * @return messages, NULL for error
 */
message_s* qmessages_wait(qmessages_s* q);

/** send a message */
void qmessages_send(qmessages_s* q, message_s* msg);

/** create a new message */
message_s* message_new_raw(size_t size, listFree_f cleanup);

/** create new message */
#define message_new(TYPE, CLEANUP) message_new_raw(sizeof(TYPE), CLEANUP)

/** free message, this function is called from consumer*/
void message_free(message_s* msg);

/** cleanup */
void message_free_auto(message_s** msg);

/** cleanup */
#define __message_free __cleanup(message_free_auto)

/**************/
/*** thread ***/
/**************/

/** thread function */
typedef void*(*thr_f)(void*);

//typedef void(*thrCleanup_f)(void*);

typedef struct thr{
	pthread_t id;
    pthread_attr_t attr;
}thr_s;

/** create and run new thread
 * @param fn function where start new thread
 * @param arg argument passed to fn
 * @param stackSize the stacksize, 0 use default value
 * @param oncpu assign thread to cpu, 0 auto
 * @param detach 1 set not joinable thread, 0 for joinable
 * @return thread or NULL for error
 */
thr_s* thr_new(thr_f fn, void* arg, unsigned stackSize, unsigned oncpu, int detach);

/** change thread cpu
 * @param thr thread
 * @param cpu cpu 1 to N
 * @return 0 successfull -1 error
 */
err_t thr_cpu_set(thr_s* thr, unsigned cpu);

/** wait, aka join, a thread and return value in out
 * @param thr a thread
 * @param out return value of thread
 * @return 0 successfull -1 error
 */
err_t thr_wait(thr_s* thr, void** out);

/** same wait but not wait, aka try join
 * @param thr a thread
 * @param out return value of thread
 * @return 0 thread end, 1 thread run, -1 error
 */
int thr_check(thr_s* thr, void** out);

/** wait, aka join, a vector of threads
 * @param vthr a vector of threads
 * @return 0 successfull -1 error
 */
err_t thr_wait_all(thr_s** vthr);

/** wait any of vector thr 
 * @param vthr a vector of thr
 * @param out return value of thread
 * @return thread exited
 */
thr_s* thr_anyof(thr_s** vthr, void** out);

/** cancel a thread
 * @param thr thread to cancel
 * @return 0 successfull, -1 error
 */
err_t thr_cancel(thr_s* thr);

/** free thread, cancel if running
 * @param thr thread
 */
void thr_free(thr_s* thr);

/** cleanup function autocall before exit from thread*/
#define thr_cleanup_push(FNC, ARG) pthread_cleanup_push(FN, ARG)

/** cleanup function autocall before exit from thread*/
#define thr_cleanup_pop(ALWAYSCALLFNC) pthread_cleanup_pop(ALWAYSCALLFNC)

/** switch to other threads */
#define thr_yield() pthread_yield()

/** exit from thread */
#define thr_exit(RET) pthread_exit(RET)



#endif
