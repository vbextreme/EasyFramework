#include <ef/promise.h>
#include <ef/phq.h>
#include <ef/deadpoll.h>
#include <ef/list.h>
#include <ef/vector.h>
#include <ef/delay.h>
#include <ef/mth.h>
#include <ef/memory.h>
#include <ef/err.h>

#include <utime.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/sysinfo.h>

//promise.id 0 = epoll
//promise.id 1 = main
//

__private __thread promise_s* running = NULL;
__private __thread long rcount = 0;
__private __thread volatile promise_s* prot;
__private __thread deadpoll_s* promisedp = NULL;
__private __thread phq_s* timequeue = NULL;

__private void promise_push(promise_s* co){
	++rcount;
	//dbg_info("push:%lu", rcount);

	// prevent double insert
	promise_s* l;
	listdoubly_do(running, l){
		if( l == co ){
			dbg_warning("already present");
			return;
		}
	}listdoubly_while(running, l);

	list_doubly_add_before(running, co);
	co->status = PROMISE_STATUS_IDLE;	
}

__private promise_s* promise_pop(void){
	--rcount;
	//dbg_info("pop:%lu", rcount);
	promise_s* co = running;
	running = LIST_DOUBLY(running)->next;
	co = list_doubly_extract(co);
	LIST_DOUBLY(co)->next = co;
	LIST_DOUBLY(co)->prev = co;
	co->status = PROMISE_STATUS_STOPPED;
	return co;
}

__private void promise_swapcontext(promise_s* curr, promise_s* next){
	//dbg_info("yield %d -> %d", curr->id, next->id);

	if( curr == next ) return;
	if( next->stack && next->protect ){
		//dbg_info("next promise set rw");
		if( mprotect(next->stack, next->size, PROT_READ | PROT_WRITE) ){
			dbg_error("mprotect next stack");
			dbg_errno();
		}
	}
	prot = curr;

	swapcontext(&curr->ctx, &next->ctx);

	//?? why unlock running? next is not already unlocked before context switch?
	if( running->stack && running->protect ){
		//dbg_info("running promise set rw");
		if( mprotect(running->stack, running->size, PROT_READ | PROT_WRITE) ){
			dbg_error("mprotect next stack");
			dbg_errno();
		}
	}

	if( prot->stack && prot->protect ){
		//dbg_info("prev promise protect");
		if( mprotect(prot->stack, prot->size, PROT_NONE) ){
			dbg_error("mprotect next stack");
			dbg_errno();
		}
	}
}

void promise_yield(void){
	promise_s* idle = running;
	running = LIST_DOUBLY(running)->next;
	promise_swapcontext(idle, running);
}

__private void promise_yield_finalize(int status){
	promise_s* me = promise_pop();
	//swapcontext(&me->ctx, &running->ctx);
	me->status = status;
	promise_swapcontext(me, running);
}

__private void promise_true_start(promise_s* co){
	//dbg_info("starting:%d", co->id);
	co->ret = co->fn(co->arg);
	vector_foreach(co->await, i){
		if( co->await[i]->status == PROMISE_STATUS_STOPPED ){
			co->await[i]->caller = running;
			promise_push(co->await[i]);
		}
	}
	vector_clear(co->await);
	promise_yield_finalize(PROMISE_STATUS_TERMINATED);
}

promise_s* promise_start(int id, size_t stacksize, promise_f fn, void* arg){
	promise_s* co = list_doubly_new(promise_s, NULL, NULL);
	co->size = stacksize ? stacksize : PROMISE_STACK_DEFAULT;
	co->fn = fn;
	co->arg = arg;
	co->id = id;
	//co->acount = 0;
	co->status = 0;
	co->protect = 0;
	co->await = vector_new(promise_s*, 6, 2);
	if( !(co->stack = mem_heap_alloc(&co->size)) ){
		err_push("heap alloc");
		vector_free(co->await);
		list_doubly_free(co);
		return NULL;
	}
	
	//dbg_info("create stack of:%lub %lukib %lumib", co->size, co->size / 1024, co->size / (1024*1024));

	getcontext(&co->ctx);

	co->ctx.uc_stack.ss_sp = co->stack;
    co->ctx.uc_stack.ss_size = co->size;
    co->ctx.uc_stack.ss_flags = 0;
	co->ctx.uc_link = NULL;
	
	makecontext(&co->ctx, (void(*)(void))promise_true_start, 1, co);
	promise_push(co);
	return co;
}

__private void* dp_task(__unused void* arg){
	//deadpoll_s* dp = arg;
	long twait = 0;

	while(1){
		phqElement_s* el = phq_peek(timequeue);
		if( el ){
			//dbg_info("check timeout:%ld", el->priority - time_ms());
			if( el->priority <= time_ms() ){
				//dbg_info("timeout delayed");
				el = phq_pop(timequeue);
				promise_push(el->data);
				phq_element_free(el);
				continue;
			}
			else{
				twait = el->priority - time_ms();
			}
		}
		else{
			twait = -1;
		}
		
		// can't enter in sleep mode because other promise running
		if( rcount > 1 ) twait = 0;
		
		//dbg_info("deadpoll(%ld,%ld) tout:%ld", rcount,phq_count(timequeue),twait);
		const long sowait = twait;
		if( deadpoll_event(promisedp, &twait) == DEADPOLL_TIMEOUT && sowait > 0 ){
			//dbg_info("timeout");
			el = phq_pop(timequeue);
			iassert(el != NULL);
			promise_push(el->data);
			phq_element_free(el);
		}
		promise_yield();
	}

	return NULL;
}

__private int dp_cbk(deadpoll_s* dp, int ev, void* arg){
	//dbg_warning("event:%d",ev);
	promise_s* tsk = arg;
	//dbg_warning("unregister fd:%d", tsk->pollev);
	deadpoll_unregister(dp, tsk->pollev);
	tsk->pollev = ev;
	promise_push(tsk);	
	return 0;
}

__private void dp_cleanup(void* ep){
	if( !rcount ){
		pollEvent_s* pe = ep;
		promise_s* pro = pe->arg;
		promise_finalize(pro);
	}
}

int promise_await_fd(int fd, int events){
	running->pollev = fd;
	deadpoll_register(promisedp, fd, dp_cbk, running, events, dp_cleanup);
	promise_yield_finalize(PROMISE_STATUS_STOPPED);
	return running->pollev;
}

void promise_delay(long ms){
	phqElement_s* el = phq_element_new(time_ms() + ms, (void*)running, NULL);
	iassert( el != NULL );
	phq_insert(timequeue, el);
	promise_yield_finalize(PROMISE_STATUS_STOPPED);
}

__private void promise_await_push(promise_s* to, promise_s* from){
	//dbg_info("toawait.count:%lu", vector_count(to->await));
	vector_push_back(to->await, from);
}

__private void promise_await_remove(promise_s* from, promise_s* me){
	vector_foreach(from->await, i){
		if( from->await[i] == me ){
			vector_remove(from->await, i);
			break;
		}
	}
}

promise_s* promise_await(promise_s* co){
	//dbg_info("enter in await");
	if( co->status == PROMISE_STATUS_TERMINATED ) return co;
	dbg_info("pop");
	promise_s* me = promise_pop();
	me->caller = NULL;
	//dbg_info("await push");
	promise_await_push(co, me);
	promise_swapcontext(me, running);
	//dbg_info("exit await");
	return running->caller;
}

promise_s* promise_anyof(promise_s** co){
	size_t add = 0;
	promise_s* me = promise_pop();
	vector_foreach(co, i){
		if( co[i]->status != PROMISE_STATUS_TERMINATED ){
			promise_await_push(co[i], me);
			++add;
		}
	}
	if( add ){
		promise_swapcontext(me, running);
	}
	vector_foreach(co, i){
		promise_await_remove(co[i], me);
	}
	return running->caller;
}

void promise_begin(void){
	running = list_doubly_new(promise_s, NULL, NULL);
	if( !running ){
		dbg_fail("eom");
	}
	running->id = 1;
	running->status = PROMISE_STATUS_IDLE;
	running->arg = NULL;
	running->fn = NULL;
	running->stack = NULL;
	rcount = 1;

	timequeue = phq_new(8, 8, phq_cmp_asc);
	iassert(timequeue);

	promisedp = deadpoll_new();
	iassert(promisedp);

	promise_start(0, 0, dp_task, NULL);
}

void promise_end(void){
	promise_s* rm;
	while( rcount ){
		rm = promise_pop();
		promise_finalize(rm);
	}
	deadpoll_free(promisedp);
	phqElement_s* el;
	while( (el=phq_pop(timequeue)) ){
		promise_finalize(el->data);
		phq_element_free(el);
	}
	phq_free(timequeue);
}

void promise_finalize(promise_s* tsk){
	if( tsk->stack ){
		mem_heap_close(tsk->stack, tsk->size);
	}
	vector_free(tsk->await);
	list_doubly_free(tsk);
}

promise_s* promise_self(void){
	return running;
}

