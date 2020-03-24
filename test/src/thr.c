#include "test.h"
#include <ef/threads.h>

/*@test -r --thr 'test threads'*/

mutex_s* glock;

void* thrPrint(void* data){
	puts(data);
	return NULL;
}

void* thrPrintLock(void* data){
	mutex_lock(glock);
	delay_ms(500);
	printf("thread write: %s\n", (char*)data);
	mutex_unlock(glock);
	return NULL;
}

void* thrQM(void* data){
	qmessages_s* qm = data;

	while(1){
		__message_free message_s* msg = qmessages_wait(qm);
		if( msg->type == 1 ) break;
		printf("message: '%s'\n", (char*)msg->data);
	}

	return NULL;
}

/*@fn*/
void test_thr(__unused const char* argA, __unused const char* argB){
	glock = mutex_new();

	thr_s* hello = thr_new(thrPrint, "hello", 0, 0, 0);
	thr_wait(hello, NULL);
	thr_free(hello);

	hello = thr_new(thrPrintLock, "world", 0,0,0);
	mutex_lock(glock);
	puts("main lock and write");
	mutex_unlock(glock);
	thr_wait(hello, NULL);
	thr_free(hello);

	qmessages_s* qm = qmessages_new();
	hello = thr_new(thrQM, qm, 0,0,0);
	char* lstmsg[] = {
		"a",
		"ab",
		"abc",
		"abcd",
		NULL
	};
	message_s* msg;
	for( size_t i = 0; lstmsg[i]; ++i){
		msg = message_new_raw(strlen(lstmsg[i])+1, NULL);
		msg->id = i;
		msg->type = 0;
		strcpy((char*)msg->data, lstmsg[i]);
		qmessages_send(qm, msg);
	}
	msg = message_new_raw(0, NULL);
	msg->type = 1;
	msg->id = -1;
	qmessages_send(qm, msg);
	thr_wait(hello, NULL);
	thr_free(hello);
	
	thr_s** vtp = vector_new(thr_s*, 10, 2);
	for( size_t i = 0; lstmsg[i]; ++i){
		vector_push_back(vtp, thr_new(thrPrintLock, lstmsg[i], 0,0,0));
	}
	if( thr_wait_all(vtp) ){
		dbg_error("waitall");
	}
	puts("end wait");
	vector_foreach( vtp, i){
		thr_free(vtp[i]);
	}
	vector_free(vtp);
	vtp = NULL;


	vtp = vector_new(thr_s*, 10, 2);
	for( size_t i = 0; lstmsg[i]; ++i){
		vector_push_back(vtp, thr_new(thrPrintLock, lstmsg[i], 0,0,0));
	}
	thr_anyof(vtp, NULL);
	puts("end anyoff");
	err_disable();
	vector_foreach( vtp, i){
		thr_cancel(vtp[i]);
		thr_free(vtp[i]);
	}
	err_restore();
	vector_free(vtp);
	vtp = NULL;


	//message_s* mm;
	//listdoubly_do(qm->queue, mm){
	//	printf("lst:%s\n", (char*)mm->data);
	//}listdoubly_while(qm->queue, mm);
	//mm = list_doubly_extract(qm->queue);
	
	dbg_info("release memory");
	qmessage_free(qm);
	mutex_free(glock);
}

