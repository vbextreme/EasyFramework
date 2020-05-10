#include "test.h"
#include <ef/threads.h>

/*@test -r --threads 'test threads'*/

#define NWAIT 100

struct fort{
	long ms[NWAIT];
	mutex_s* mtx;
	semaphore_s* sem;
	event_s* ev;
	int fdev;
	qmessages_s* qm;
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
	counter  = 0;
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

void* t_print_evfd_consumer(void* arg){
	struct fort* f = arg;
	size_t i = 0;
	counter = 0;
	while(1){
		printf("ev #\n");
		fd_timeout(f->fdev,-1);
		long val = 0;
		if( counter == 777 ) break;
		if( event_fd_read(&val, f->fdev) ){
			dbg_error("event");
			continue;
		}
		if( counter == 777 ) break;
		delay_ms(f->ms[0]);
		printf("ev(%ld):%d\n", val, sequence[i++]);
		
	}
	printf("end consumer\n");

	return NULL;
}

void* t_print_evfd_producer(void* arg){
	struct fort* f = arg;
	size_t i = 0;

	while(1){
		sequence[i] = i;
		printf("-->%lu\n", i);
		++i;
		event_fd_write(f->fdev, 1);
		delay_ms(f->ms[1]);
		if( i >= N ){
			counter = 777;
			event_fd_write(f->fdev, 1);
			break;
		}
	}
	printf("end producer\n");
	
	return NULL;
}

void* t_print_messages_consumer(void* arg){
	struct fort* f = arg;
	while(1){
		printf("qm #\n");
		fd_timeout(f->qm->evfd,-1);
		message_s* msg;
		while( (msg = qmessages_get(f->qm)) ){
			int* i = (int*)&msg->data;
			printf("qm <- %d\n", *i);
			delay_ms(f->ms[0]);
			if( *i == 777 ){
				message_free(msg);
				goto END;
			}
			message_free(msg);
		}
		delay_ms(f->ms[2]);
	}
END:
	printf("end consumer\n");

	return NULL;
}

void* t_print_messages_producer(void* arg){
	struct fort* f = arg;
	size_t i = 0;

	for( i = 0; i < N; ++i){
		message_s* msg = message_new(int, NULL);
		msg->id = 0;
		msg->type = 0;
		int* k = (int*)&msg->data;
		*k = i;
		qmessages_send(f->qm, msg);
		delay_ms(f->ms[1]);
		printf("-->%lu\n", i);
	}
	message_s* msg = message_new(int, NULL);
	msg->id = 0;
	msg->type = 0;
	int* k = (int*)&msg->data;
	*k = 777;
	qmessages_send(f->qm, msg);
	event_fd_write(f->fdev, 1);

	printf("end producer\n");
	
	return NULL;
}

void* t_print_messagesblock_consumer(void* arg){
	struct fort* f = arg;
	
	printf("qm #\n");
	fd_timeout(f->qm->evfd,-1);
	message_s* msg;
	while( (msg = qmessages_get(f->qm)) ){
		int* i = (int*)&msg->data;
		printf("qm <- %d\n", *i);
		message_free(msg);
	}
	printf("qm --\n");
	delay_ms(f->ms[0]);
	
	printf("qm #\n");
	fd_timeout(f->qm->evfd,-1);
	while( (msg = qmessages_get(f->qm)) ){
		int* i = (int*)&msg->data;
		printf("qm <- %d\n", *i);
		message_free(msg);
	}
	printf("end consumer\n");

	return NULL;
}

void* t_print_messagesblock_producer(void* arg){
	struct fort* f = arg;
	size_t i = 0;

	message_s* msg = message_new(int, NULL);
	msg->id = 0;
	msg->type = 0;
	int* k = (int*)&msg->data;
	*k = i;
	qmessages_send(f->qm, msg);	
	printf("-->%lu\n", i);
	
	delay_ms(f->ms[1]);

	msg = message_new(int, NULL);
	msg->id = 0;
	msg->type = 0;
	k = (int*)&msg->data;
	*k = 777;
	qmessages_send(f->qm, msg);

	printf("end producer\n");
	
	return NULL;
}

/*@fn*/
void test_thr(__unused const char* argA, __unused const char* argB){
	struct fort ff = {
		.ms = { 100, 100, 100},
		.mtx = mutex_new(),
		.sem = semaphore_new(0),
		.ev = event_new(),
		.fdev = event_fd_new(0, 1),
		.qm = qmessages_new(1)
	};
	counter = 30;

	thr_s** vt = vector_new(thr_s*, 16, (vfree_f)thr_free );

	/* test mutex*/
	puts("mutex:");
	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	thr_wait_all(vt);
	vector_clear(vt);

	/* test semaphore*/
	puts("semaphore:");
	vector_push_back(vt, thr_start(t_print_sem_consumer, &ff));
	vector_push_back(vt, thr_start(t_print_sem_producer, &ff));
	thr_wait_all(vt);
	vector_clear(vt);

	/* test event*/
	puts("event");
	vector_push_back(vt, thr_start(t_print_ev_consumer, &ff));
	vector_push_back(vt, thr_start(t_print_ev_producer, &ff));
	thr_wait_all(vt);
	vector_clear(vt);

	/* test event fd*/
	puts("eventfd:");
	vector_push_back(vt, thr_start(t_print_evfd_consumer, &ff));
	vector_push_back(vt, thr_start(t_print_evfd_producer, &ff));
	thr_wait_all(vt);
	vector_clear(vt);

	/* test messages */
	puts("messages:");
	ff.ms[0] = 250;
	ff.ms[1] = 100;
	ff.ms[2] = 300;
	vector_push_back(vt, thr_start(t_print_messages_consumer, &ff));
	vector_push_back(vt, thr_start(t_print_messages_producer, &ff));
	thr_wait_all(vt);
	vector_clear(vt);

	/* test messages block */
	puts("messages block:");
	ff.ms[0] = 450;
	ff.ms[1] = 100;
	vector_push_back(vt, thr_start(t_print_messagesblock_consumer, &ff));
	vector_push_back(vt, thr_start(t_print_messagesblock_producer, &ff));
	thr_wait_all(vt);
	vector_clear(vt);


	mutex_free(ff.mtx);
	semaphore_free(ff.sem);
	event_free(ff.ev);
	fd_close(ff.fdev);

	/* end test */
	vector_free(vt);

}

