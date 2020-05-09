#include "test.h"
#include <ef/threads.h>
#include <ef/vector.h>

/*@test -d --threads 'test threads'*/

struct fort{
	long ms;
	mutex_s* mtx;
};

volatile long counter;

void* t_print_mutex(void* arg){
	struct fort* f = arg;
	
	while(1){
		mutex_lock(f->mtx);
		if( counter == 0 ) break;
		--counter;
		printf("no %lu) %ld\n", thr_self_id(), counter);
		mutex_unlock(f->mtx);
		delay_ms(f->ms);
	}
	printf("%lu) end\n", thr_self_id());
	mutex_unlock(f->mtx);
	return NULL;
}

void* t_print_mutexfd(void* arg){
	struct fort* f = arg;
	
	while(1){
		int fd;
	   	while(1){
			fd = mutex_fd_lock(f->mtx);
			if( fd == -1 ) break;
			fd_timeout(fd, -1);
			fd = mutex_fd_event(f->mtx, fd);
			if( fd == -1 ) break;
			fd_timeout(fd, -1);
		}

		if( counter == 0 ) break;
		--counter;
		printf("fd %lu) %ld\n", thr_self_id(), counter);
		mutex_unlock(f->mtx);
		delay_ms(f->ms);
	}
	printf("fd %lu) end\n", thr_self_id());
	mutex_unlock(f->mtx);
	return NULL;
}

/*@fn*/
void test_threads(__unused const char* argA, __unused const char* argB){
	struct fort ff = {
		.ms = 500,
		.mtx = mutex_new()
	};
	counter = 300;

	thr_s** vt = vector_new(thr_s*, 16, 4);

	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	vector_push_back(vt, thr_start(t_print_mutex, &ff));
	vector_push_back(vt, thr_start(t_print_mutexfd, &ff));
	vector_push_back(vt, thr_start(t_print_mutexfd, &ff));

	thr_wait_all(vt);	







}

