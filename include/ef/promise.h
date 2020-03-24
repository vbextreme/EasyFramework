#ifndef __EF_PROMISE_H__
#define __EF_PROMISE_H__

#include <ef/type.h>
#include <ucontext.h>

#ifndef PROMISE_STACK_DEFAULT
	#define PROMISE_STACK_DEFAULT (8UL * MiB)
#endif

#define PROMISE_STATUS_IDLE       0
#define PROMISE_STATUS_STOPPED    1
#define PROMISE_STATUS_TERMINATED 2

typedef void*(*promise_f)(void*);

typedef struct promise{
	struct promise** await;
	struct promise* caller;
	ucontext_t ctx;
	void* stack;
	size_t size;
	int protect;
	promise_f fn;
	void* arg;
	void* ret;  /**< return value*/
	int pollev; /**< event epoll*/
	int id;     /**< promise id, main promise have id 1, and id 0 is for epoll promise*/
	int status;
}promise_s;

/** change context to next promise */
void promise_yield(void);

/** start new promise
 * @param id id for promise, > 1
 * @param stacksize set stack size, 0 automatic set to default value
 * @param fn functon where start promise
 * @param arg argument to pass a function
 * @return
 */
promise_s* promise_start(int id, size_t stacksize, promise_f fn, void* arg);

/** in promise wait event on fd
 * @return event raised
 */
int promise_await_fd(int fd, int events);

/** sleep in promise */
void promise_delay(long ms);

/** wait a promise
 * @return a ended promise
 */
promise_s* promise_await(promise_s* co);

/** wait any of promise
 * @param co is vector of promise
 * @return a promise finished
 */
promise_s* promise_anyof(promise_s** co);

/** before used promise */
void promise_begin(void);

/** after end use promise*/
void promise_end(void);

/** when finish to use promise call finalize*/
void promise_finalize(promise_s* tsk);

/** return promise inside a promise */
promise_s* promise_self(void);

#endif
