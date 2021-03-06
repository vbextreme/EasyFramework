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
}

mutex_s* mutex_new(){
	mutex_s* ret = mem_new(mutex_s);
	if( !ret ) err_fail("malloc");
	mutex_init(ret);
	ret->private = FUTEX_PRIVATE_FLAG;
	return ret;
}

void mutex_free(mutex_s* mtx){
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

/*****************/
/*** semaphore ***/
/*****************/

void semaphore_init(semaphore_s* sem, int val){
	sem->futex = val;
	sem->private = 0;
}

semaphore_s* semaphore_new(int val){
	semaphore_s* ret = mem_new(semaphore_s);
	if( !ret ) err_fail("malloc");
	semaphore_init(ret, val);
	ret->private = FUTEX_PRIVATE_FLAG;
	return ret;
}

void semaphore_free(semaphore_s* sem){
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

/*************/
/*** event ***/
/*************/

void event_init(event_s* ev){
	ev->futex = 0;
	ev->private = 0;
}

event_s* event_new(){
	event_s* ret = mem_new(event_s);
	if( !ret ) err_fail("malloc");
	event_init(ret);
	ret->private = FUTEX_PRIVATE_FLAG;
	return ret;
}

void event_free(event_s* ev){
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

/***************/
/*** eventfd ***/
/***************/

int event_fd_new(long val, int nonblock){
	int fd = eventfd(val, 0);
	if( fd == -1 ) return -1;
	if( nonblock ){
		int flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}
	return fd;
}

err_t event_fd_read(long* val, int fd){
	if( read(fd, val, sizeof(uint64_t)) != sizeof(uint64_t)) return -1;
    return 0;
}

err_t event_fd_write(int fd, long val){
	if( write(fd, &val, sizeof(uint64_t)) != sizeof(uint64_t)) return -1;
    return 0;
}

/*****************/
/*** qmessages ***/
/*****************/

void qmessages_init(qmessages_s* q, int evfd, mutex_s* mtx){
	q->current = NULL;
	q->tail = NULL;
	q->mtx = mtx;
	q->evfd = evfd;
}

qmessages_s* qmessages_new(int nonblock){
	qmessages_s* ret = mem_new(qmessages_s);
	if( !ret ) err_fail("malloc");

	qmessages_init(ret, event_fd_new(0,nonblock), mutex_new());
	if( ret->evfd == -1 ) err_fail("eventfd");
	
	return ret;
}

void qmessages_free(qmessages_s* q){
	if( q->mtx ) mutex_free(q->mtx);
	if( q->evfd ) close(q->evfd);
	message_s* next;
	while( q->current ){
		next = q->current->next;
		if( q->current->clean ) q->current->clean(q->current->data);
		free(q->current);
		q->current = next;
	}
	free(q);
}

void* qmessages_get(qmessages_s* q){
	message_s* ret;

	mutex_lock(q->mtx);
	ret = q->current;
	if( q->current ){
		q->current = q->current->next;
	}
	mutex_unlock(q->mtx);

	if( ret ){
		ret->next = NULL;
		return ret->data;
	}

	long v;
	event_fd_read(&v, q->evfd);
	return NULL;
}

void qmessages_send(qmessages_s* q, void* m){
	message_s* msg = MESSAGE(m);
	mutex_lock(q->mtx);
	if( q->current ){
		*(q->tail) = msg;
	}
	else{
		q->current = msg;
	}	
	q->tail = &msg->next;
	mutex_unlock(q->mtx);
	event_fd_write(q->evfd, 1);
}

void* message_new_raw(size_t size, qmfree_f cleanup){
	message_s* msg = mem_flexible_structure_new(message_s, size, 1);
	if( !msg ) err_fail("malloc");
	msg->next = 0;
	msg->clean = cleanup;	
	return msg->data;
}

void message_free(void* m){
	message_s* msg = MESSAGE(m);
	if( msg->clean ) msg->clean(&msg->data);
	free(msg);
}

void message_free_auto(void** msg){
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


