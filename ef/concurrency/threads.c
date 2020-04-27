#include <ef/threads.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/vector.h>

#include <sched.h>
#include <pthread.h>
#include <linux/futex.h>
#include <sys/syscall.h>

/*************/
/*** futex ***/
/*************/

int futex_to(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3){
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

int futex_v2(int *uaddr, int futex_op, int val, unsigned val2, int *uaddr2, int val3){
	return syscall(SYS_futex, uaddr, futex_op, val, val2, uaddr2, val3);
}

/*************/
/*** mutex ***/
/*************/

void mutex_init(mutex_s* mtx){
	mtx->futex = 0;
	mtx->private = 0;
	mtx->fd = -1;
}

mutex_s* mutex_new(){
	mutex_s* ret = mem_new(mutex_s);
	if( !ret ){
		err_pushno("malloc");
		return NULL;
	}
	mutex_init(ret);
	ret->private = FUTEX_PRIVATE_FLAG;
	return ret;
}

void mutex_free(mutex_s* mtx){
	if( mtx->fd != -1 ){
		dbg_error("some threads wait a mutex");
		close(mtx->fd);
	}
	free(mtx);
}

void mutex_unlock(mutex_s* mtx){
	unsigned const op = FUTEX_WAKE | mtx->private;
	if( __sync_fetch_and_sub(&mtx->futex, 1) != 1)  {
		mtx->futex = 0;
	    futex(&mtx->futex, op, 1, NULL, NULL, 0);
	}
}

void mutex_lock(mutex_s* mtx){
	unsigned const op = FUTEX_WAIT | mtx->private;
	unsigned m;
	if( (m = __sync_val_compare_and_swap(&mtx->futex, 0, 1)) ){
		do{
			if( m == 2 || __sync_val_compare_and_swap(&mtx->futex, 1, 2) != 0){
				futex(&mtx->futex, op, 2, NULL, NULL, 0);
			}
		}while( (m = __sync_val_compare_and_swap(&mtx->futex, 0, 2)) );
	}
}

int mutex_trylock(mutex_s* mtx){
	if( __sync_val_compare_and_swap(&mtx->futex, 0, 1) ){
		return -1;
	}
	return 0;
}

int mutex_fd_lock(mutex_s* mtx){
	unsigned const op = FUTEX_FD | mtx->private;
	unsigned m;
	if( (m = __sync_val_compare_and_swap(&mtx->futex, 0, 1)) ){
		if( m == 2 || __sync_val_compare_and_swap(&mtx->futex, 1, 2) != 0){
			mtx->fd = futex(&mtx->futex, op, 2, NULL, NULL, 0);
		}
	}
	return mtx->fd;
}

int mutex_fd_event(mutex_s* mtx){
	unsigned const op = FUTEX_FD | mtx->private;
	if( mtx->fd != -1 ) close(mtx->fd);
	mtx->fd = -1;
	if( __sync_val_compare_and_swap(&mtx->futex, 0, 2) ){
		mtx->fd = futex(&mtx->futex, op, 2, NULL, NULL, 0);
	}
	return mtx->fd;
}

/*****************/
/*** semaphore ***/
/*****************/

void semaphore_init(semaphore_s* sem, int val){
	sem->futex = val;
	sem->private = 0;
	sem->fd = -1;
}

semaphore_s* semaphore_new(int val){
	semaphore_s* ret = mem_new(semaphore_s);
	if( !ret ){
		err_pushno("malloc");
		return NULL;
	}
	semaphore_init(ret, val);
	ret->private = FUTEX_PRIVATE_FLAG;
	return ret;
}

void semaphore_free(semaphore_s* sem){
	if( sem->fd != -1 ){
		dbg_error("thread wait a semaphore");
		close(sem->fd);
	}
	free(sem);
}

void semaphore_post(semaphore_s* sem){
	unsigned const op = FUTEX_WAKE | sem->private;
	if( __sync_fetch_and_add(&sem->futex, 1) == 0 ){
		futex(&sem->futex, op, 1, NULL, NULL, 0);
	}
}

void semaphore_wait(semaphore_s* sem){
	unsigned const op = FUTEX_WAIT | sem->private;
	while( __sync_bool_compare_and_swap(&sem->futex, 0, 0) ){
		futex(&sem->futex, op, 0, NULL, NULL, 0);
	}
	__sync_fetch_and_sub(&sem->futex, 1);
}

int semaphore_trywait(semaphore_s* sem){
	if( __sync_bool_compare_and_swap(&sem->futex, 0, 0) ){
		return -1;
	}
	__sync_fetch_and_sub(&sem->futex, 1);
	return 0;
}

int semaphore_fd_wait(semaphore_s* sem){
	unsigned const op = FUTEX_FD | sem->private;
	if( __sync_bool_compare_and_swap(&sem->futex, 0, 0) ){
		sem->fd = futex(&sem->futex, op, 0, NULL, NULL, 0);
	}
	return sem->fd;
}

int semaphore_fd_event(semaphore_s* sem){
	if( sem->fd != -1 ) close(sem->fd);
	sem->fd = semaphore_fd_wait(sem);
	if( sem->fd == -1 ){
		__sync_fetch_and_sub(&sem->futex, 1);
	}
	return sem->fd;
}

/*************/
/*** event ***/
/*************/

void event_init(event_s* ev){
	ev->futex = 0;
	ev->private = 0;
	ev->fd = -1;
}

event_s* event_new(){
	event_s* ret = mem_new(event_s);
	if( !ret ){
		err_pushno("malloc");
		return NULL;
	}
	event_init(ret);
	ret->private = FUTEX_PRIVATE_FLAG;
	return ret;
}

void event_free(event_s* ev){
	if( ev->fd != -1 ){
		dbg_error("some threads wait an event");
		close(ev->fd);
	}
	free(ev);
}

void event_raise(event_s* ev){
	unsigned const op = FUTEX_WAKE | ev->private;
	if( __sync_bool_compare_and_swap(&ev->futex, 0, 1) )  {
	    futex(&ev->futex, op, INT_MAX, NULL, NULL, 0);
	}
}

void event_wait(event_s* ev){
	unsigned const op = FUTEX_WAIT | ev->private;
	if( __sync_bool_compare_and_swap(&ev->futex, 0, 0) ){
		do{
			futex(&ev->futex, op, 0, NULL, NULL, 0);
		}while( __sync_bool_compare_and_swap(&ev->futex, 0, 0) );
	}
	__sync_bool_compare_and_swap(&ev->futex,1,0);
}

int event_fd_wait(event_s* ev){
	unsigned const op = FUTEX_FD | ev->private;
	if( ev->fd != -1 ) close(ev->fd);
	if( __sync_bool_compare_and_swap(&ev->futex, 0, 0) ){
		ev->fd=futex(&ev->futex, op, 0, NULL, NULL, 0);
	}
	if( __sync_bool_compare_and_swap(&ev->futex,1,0) ){
		if( ev->fd ) close(ev->fd);
		ev->fd = -1;
	}
	return ev->fd;
}

int event_fd_event(event_s* ev){	
	return event_fd_wait(ev);
}

/*****************/
/*** qmessages ***/
/*****************/

void qmessages_init(qmessages_s* q, semaphore_s* sem, mutex_s* mtx){
	q->queue = NULL;
	q->sem = sem;
	q->mtx = mtx;
}

qmessages_s* qmessages_new(){
	qmessages_s* ret = mem_new(qmessages_s);
	if( !ret ){
		err_pushno("malloc");
		return NULL;
	}
	qmessages_init(ret, semaphore_new(0), mutex_new());
	if( !ret->sem || !ret->mtx ){
		if( ret->sem ) semaphore_free(ret->sem);
		if( ret->mtx ) mutex_free(ret->mtx);
		free(ret);
		return NULL;
	}
	return ret;
}

void qmessage_free(qmessages_s* q){
	semaphore_free(q->sem);
	mutex_free(q->mtx);
	if( q->queue ){
		list_doubly_all_free(q->queue);
	}
	free(q);
}

message_s* qmessages_wait(qmessages_s* q){
	message_s* ret;
	
	//dbg_info("sem wait");
	semaphore_wait(q->sem);
	mutex_lock(q->mtx);
		//dbg_info("mutex lock");
		if( list_doubly_only_root(q->queue) ){
			//dbg_info("last message");
			ret = q->queue;
			q->queue = NULL;
		}
		else{
			//dbg_info("get message");
			ret = q->queue;
			q->queue = LIST_DOUBLY(q->queue)->next;
			ret = list_doubly_extract(ret);
		}
		//dbg_info("mutex unlock");
	mutex_unlock(q->mtx);

	return ret;
}

void qmessages_send(qmessages_s* q, message_s* msg){
	mutex_lock(q->mtx);
		//dbg_info("mutex lock");
		if( q->queue ){
			//dbg_info("add message");
			list_doubly_add_before(q->queue, msg);
		}
		else{
			//dbg_info("first message");
			q->queue = msg;
		}
		//dbg_info("mutex unlock");
	mutex_unlock(q->mtx);
	//dbg_info("sem post");
	semaphore_post(q->sem);
}

message_s* message_new_raw(size_t size, listFree_f cleanup){
	message_s* msg = list_doubly_new_raw(size + sizeof(message_s), NULL, cleanup);
	if( !msg ) return NULL;
	return msg;
}

void message_free(message_s* msg){
	list_doubly_free(msg);
}

void message_free_auto(message_s** msg){
	if( *msg ) message_free(*msg);
}

/**************/
/*** thread ***/
/**************/

__private cpu_set_t* thr_setcpu(unsigned mcpu){
	static cpu_set_t cpu;
	CPU_ZERO(&cpu);
	if (mcpu == 0 ) return &cpu;
	unsigned s;
	while ( (s = mcpu % 10) ){
		CPU_SET(s - 1,&cpu);
		mcpu /= 10;
	}
	return &cpu;
}

thr_s* thr_new(thr_f fn, void* arg, unsigned stackSize, unsigned oncpu, int detach){
	thr_s* thr = mem_new(thr_s);
	if( thr == NULL ){
		err_pushno("malloc");
		return NULL;
	}
	
	pthread_attr_init(&thr->attr);
    if ( stackSize > 0 && pthread_attr_setstacksize(&thr->attr, stackSize) ){
		err_pushno("pthread stack size");
		goto ONERR;
	}

	if( oncpu > 0 ){
		cpu_set_t* ncpu = thr_setcpu(oncpu);
		if( pthread_attr_setaffinity_np(&thr->attr, CPU_SETSIZE, ncpu) ){
			err_pushno("pthread set affinity");
			goto ONERR;
		}
	}

	if( pthread_attr_setdetachstate(&thr->attr, detach ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE) ){
		err_pushno("pthread detach state");
		goto ONERR;
	}

	if( pthread_create(&thr->id, &thr->attr, fn, arg) ){
		err_pushno("pthread create");
		goto ONERR;
	}
	return thr;

ONERR:
	pthread_attr_destroy(&thr->attr);
	free(thr);
	return NULL;
}

err_t thr_cpu_set(thr_s* thr, unsigned cpu){
	if( cpu > 0 ){
		cpu_set_t* ncpu = thr_setcpu(cpu);
		if( pthread_attr_setaffinity_np(&thr->attr, CPU_SETSIZE, ncpu) ){
			err_pushno("pthread set affinity");
			return -1;
		}
	}
	return 0;
}

err_t thr_wait(thr_s* thr, void** out){
    if( pthread_join(thr->id, out) ){
		err_pushno("join");
		return -1;
	}
    return 0;
}

int thr_check(thr_s* thr, void** out){
	if( pthread_tryjoin_np(thr->id, out) ){
		return errno == EBUSY ? 1 : -1;
	}
	return 0;
}

err_t thr_wait_all(thr_s** vthr){
	vector_foreach(vthr, i){
		if( thr_wait(vthr[i], NULL) ){
			return -1;
		}
	}
	return 0;
}

thr_s* thr_anyof(thr_s** vthr, void** out){
	thr_s* ret = NULL;
	
	while(1){
		vector_foreach(vthr, i){
			if( !thr_check(vthr[i], out) ){
				ret = vthr[i];
				goto ONEND;
			}
		}
		thr_yield();
	}

ONEND:
	return ret;	
}

err_t thr_cancel(thr_s* thr){
	if( pthread_cancel(thr->id) ){
		err_pushno("pthread cancel");
		return -1;
	}
	return 0;
}

void thr_free(thr_s* thr){
	//err_disable();
	//thr_cancel(thr);
	//err_restore();
	pthread_attr_destroy(&thr->attr);
	free(thr);
}


