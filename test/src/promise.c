#include "test.h"
#include <ef/promise.h>
#include <ef/socket.h>
#include <ef/deadpoll.h>
#include <ef/file.h>

/*@test -p --promise 'test promise'*/

void* pro_time(void* t){
	long* td = t;
	promise_s* self = promise_self();
	dbg_info("promise:%d, delay:%ld", self->id, *td);
	promise_delay(*td);
	dbg_info("promise:%d, end", self->id);
	return NULL;
}

void* pro_servlet(void* arg){
	__socket_close socket_s* servlet = arg;

	if( !servlet ){
		dbg_error("servlet");
		err_print();
		return NULL;
	}
	
	int ev;
	while( (ev=promise_await_fd(servlet->fd, SOCKET_DEADPOLL_EVENT)) & EPOLLIN && !((ev & EPOLLHUP)||(ev & EPOLLRDHUP)) ){
		//dbg_fail("oh %d %d",ev,ev & EPOLLRDHUP);
		dbg_info("process servlet");
		char* line;
		size_t nr;
		while( stream_kbhit(servlet->stream) && (nr=stream_inp(servlet->stream, &line, '\n', 1)) > 0 ){
			printf("%s > %s", socket_addr_get(servlet), line);
			if( !str_equal(line, nr, "close\n", strlen("close\n")) ){
				dbg_info("send close");
				stream_out(servlet->stream, "exit\n", strlen("exit\n"));
				free(line);
				stream_flush(servlet->stream);
				return NULL;
			}
			stream_out(servlet->stream, line, strlen(line));
			free(line);
		}
		stream_flush(servlet->stream);
		//dbg_info("wait fd");
	}
	return NULL;
}

void* pro_server_accept(void* arg){
	socket_s* server = arg;
	if( !server ){
		dbg_error("no server");
		return NULL;
	}

	size_t id = 1000;
	if( promise_await_fd(server->fd, SOCKET_DEADPOLL_EVENT) & EPOLLIN ){
		return promise_start(id++, 0, pro_servlet, socket_accept(socket_new(server->type, NULL, NULL, NULL),server,4096,0,NULL));	
	}
	return NULL;
}

void* pro_server(__unused void* t){
	__socket_close socket_s* server = socket_listen(
			socket_open(
				socket_new(SOCKET_TYPE_NET4, NULL, NULL, NULL), 
				0, 0, 0
			),
			(esport_u){.port=1234}
	);
	if( !server ){
		err_print();
		return NULL;
	}
	
	promise_s** allp = vector_new(promise_s*, 8, 2);
	while( 1 ){
		dbg_info("process all promise");
		vector_push_back(allp, promise_start(999, 0, pro_server_accept, server));
		promise_s* acc = promise_anyof(allp);
		dbg_info("anyoff id(%d)", acc->id);
		if( acc->id == 999 ){
			if( acc->ret ) vector_push_back(allp, acc->ret);
		}
		vector_foreach(allp, i){
			if( allp[i]->id == acc->id ){
				promise_finalize(allp[i]);
				vector_remove(allp, i);
			}
		}
	}

	return NULL;
}

/*@fn*/
void test_promise(const char* argA, __unused const char* argB){
	dbg_info("begin promise");
	promise_begin();

	if( argA != NULL ){
		promise_await(promise_start(777, 0, pro_server, NULL));
	}
	else{
		long td = 1000;
		promise_s* p0 = promise_start(2, 0, pro_time, &td);
		if(	!p0 ){
			err_print();
			dbg_fail("promise start");
		}

		dbg_info("await promise");
		size_t ts = time_ms();
		promise_await(p0);
		printf("wait time: %lu\n", time_ms() - ts);
		dbg_info("finalize promise");
		promise_finalize(p0);
	
		dbg_info("test anyof");
		long t0 = 1300;
		long t1 = 1100;
		promise_s** vp = vector_new(promise_s*, 6, 2);
		ts = time_ms();
		vector_push_back(vp, promise_start(2, 0, pro_time, &t0));
		vector_push_back(vp, promise_start(3, 0, pro_time, &t1));
	
		promise_s* pterm = promise_anyof(vp);
		printf("promise:%d terminated after:%lu\n", pterm->id, time_ms() - ts);
		vector_foreach(vp, i){
			if( vp[i]->id == pterm->id ){
				vector_remove(vp, i);
				break;
			}
		}
		promise_finalize(pterm);

		vector_foreach(vp, i){
			promise_await(vp[i]);
			printf("promise:%d terminated after:%lu\n", vp[i]->id, time_ms() - ts);
			promise_finalize(vp[i]);
		}
	}
	dbg_info("end promise");
	promise_end();
}

