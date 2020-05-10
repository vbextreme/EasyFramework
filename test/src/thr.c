#include "test.h"
#include <ef/threads.h>

/*@test -r --threads 'test threads'*/

#define NWAIT 100

struct fort{
	long ms[NWAIT];
	mutex_s* mtx;
	semaphore_s* sem;
	event_s* ev;
};

#define N 30
volatile long counter;
volatile int sequence[N];

void* t_print_mutex(void* arg){
	struct fort* f = arg;
	
	while(1){
		mutex_lock(f->mtx);
		if( counter == 0 ) break;
		--counter;
		printf("no %lu) %ld\n", thr_self_id(), counter);
		mutex_unlock(f->mtx);
		delay_ms(f->ms[0]);
	}
	printf("%lu) end\n", thr_self_id());
	mutex_unlock(f->mtx);
	return NULL;
}

void* t_print_sem_consumer(void* arg){
	struct fort* f = arg;
	size_t i = 0;

	while(1){
		semaphore_wait(f->sem);
		if( i >= N ) break;
		delay_ms(f->ms[0]);
		printf("cons:%d\n", sequence[i++]);
	}
	printf("end consumer\n");

	return NULL;
}

void* t_print_sem_producer(void* arg){
	struct fort* f = arg;
	size_t i = 0;

	while(1){
		sequence[i] = i;
		++i;
		semaphore_post(f->sem);
		delay_ms(f->ms[1]);
		if( i >= N ){
			semaphore_post(f->sem);
			break;
		}
	}
	printf("end producer\n");

	return NULL;
}

void* t_print_ev_consumer(void* arg){
	struct fort* f = arg;
	size_t i = 0;

	while(1){
		event_wait(f->ev);
		if( i >= N || counter == 777 ) break;
		delay_ms(f->ms[0]);
		printf("ev:%d\n", sequence[i++]);
	}
	printf("end consumer\n");

	return NULL;
}

void* t_print_ev_producer(void* arg){
	struct fort* f = arg;
	size_t i = 0;

	while(1){
		sequence[i] = i;
		++i;
		event_raise(f->ev);
		delay_ms(f->ms[1]);
		if( i >= N ){
			counter = 777; // not realy safe
			event_raise(f->ev);
			break;
		}
	}
	printf("end producer\n");
	

	return NULL;
}

/*@fn*/
void test_thr(__unused const char* argA, __unused const char* argB){
	struct fort ff = {
		.ms = { 250, 150},
		.mtx = mutex_new(),
		.sem = semaphore_new(0),
		.ev = event_new()
	};
	counter = 30;

	thr_s** vt = vector_new(thr_s*, 16, (vfree_f)thr_free );

	/* test mutex*/
	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	thr_wait_all(vt);
	vector_clear(vt);

	/* test semaphore*/
	vector_push_back(vt, thr_start(t_print_sem_consumer, &ff));
	vector_push_back(vt, thr_start(t_print_sem_producer, &ff));
	thr_wait_all(vt);
	vector_clear(vt);

	/* test event*/
	vector_push_back(vt, thr_start(t_print_ev_consumer, &ff));
	vector_push_back(vt, thr_start(t_print_ev_producer, &ff));
	thr_wait_all(vt);
	vector_clear(vt);


	
	/* end test */
	vector_free(vt);

}

