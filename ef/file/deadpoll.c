#include <ef/deadpoll.h>
#include <ef/delay.h>
#include <ef/memory.h>
#include <ef/list.h>
#include <ef/err.h>

#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/sysinfo.h>
#include <fcntl.h>

#ifndef DEADPOLL_POLLING_EVENTS
	#define DEADPOLL_POLLING_EVENTS 32
#endif

__private int dp_unreg_cmp(void* epl, void* pfd){
	int* fd = pfd;
	pollEvent_s* ev = epl;
	return ev->fd - *fd;
}

err_t deadpoll_unregister(deadpoll_s* dp, int fd){
	__listsimple_free pollEvent_s* ev = list_simple_find_extract(&dp->events, &fd, dp_unreg_cmp);
	
	if( ev == NULL ){
		dbg_error("fd(%d) not exists", fd);
		err_push("fd(%d) not exists", fd);
		return -1;
	}

	--dp->nevents;

	if( epoll_ctl(dp->pollfd, EPOLL_CTL_DEL, ev->fd, NULL) ){
		dbg_error("unregister fd:%d", fd);
		dbg_errno();
		err_pushno("epoll_ctl on fd:%d", fd);
		return -1;
	}

	return 0;
}

err_t deadpoll_register(deadpoll_s* dp, int fd, pollCbk_f cbk, void* arg, int onevents, listFree_f cleanup){
	iassert(cbk);

	dbg_warning("register fd %d",fd);
	if( onevents == 0 ) onevents = EPOLLIN | EPOLLET | EPOLLPRI;

	pollEvent_s* dpe = list_simple_new(pollEvent_s, NULL, cleanup);

	if( dpe == NULL ){
		err_pushno("malloc");
		return -1;
	}

	struct epoll_event ev = {
		.data.ptr = dpe,
		.events = onevents,
	};
	if( epoll_ctl(dp->pollfd, EPOLL_CTL_ADD, fd, &ev) ){
		err_pushno("epoll_ctl on fd:%d", fd);
		list_simple_free(dpe);
		return -1;
	}

	dpe->fd = fd;
	dpe->arg = arg;
	dpe->callback = cbk;
	dpe->event = onevents;

	list_simple_add_head(&dp->events, dpe);
	++dp->nevents;
	
	return 0;
}

void deadpoll_register_timeout(deadpoll_s* dp, pollCbk_f cbk, void* arg){
	dp->timeout = cbk;
	dp->timeoutarg = arg;
}

deadpoll_s* deadpoll_new(void){
	deadpoll_s* dp = list_simple_new(deadpoll_s, NULL, NULL);
	if( !dp ){
		err_pushno("malloc");
		return NULL;
	}

	dp->pollfd = epoll_create1(0);
	if( dp->pollfd < 0 ){
		err_pushno("epoll_create1");
		free(dp);
		return NULL;
	}

	dp->events = NULL;
	dp->timeout = NULL;
	dp->timeoutarg = NULL;
	dp->nevents = 0;
	
	return dp;
}

void deadpoll_free(deadpoll_s* dp){
	pollEvent_s* next;
	while( dp->events ){
		next = LIST_SIMPLE(dp->events)->next;
		epoll_ctl(dp->pollfd, EPOLL_CTL_DEL, dp->events->fd, NULL);
		list_simple_free(dp->events);
		dp->events = next;
	}
	close( dp->pollfd );
	list_simple_free(dp);
}

void deadpoll_free_auto(deadpoll_s** dp){
	if( *dp ){
		deadpoll_free(*dp);
	}
}

int deadpoll_event(deadpoll_s* dp, long* timems){
	int eventCount;
	struct epoll_event epollEvent[DEADPOLL_POLLING_EVENTS];
	memset(epollEvent, 0, sizeof(struct epoll_event) * DEADPOLL_POLLING_EVENTS);
	long timer = time_ms();

	switch( (eventCount=epoll_wait(dp->pollfd, epollEvent, DEADPOLL_POLLING_EVENTS, *timems)) ){
		case -1:
			dbg_error("deadpoll");
			dbg_errno();
			err_pushno("epoll_wait");
		return DEADPOLL_ERROR;

		case 0:
			if( timems ) *timems = 0;
			if( dp->timeout && dp->timeout(dp, 0, dp->timeoutarg) ){
				return DEADPOLL_ERROR;
			}
		return DEADPOLL_TIMEOUT;

		default:
			for( size_t i = 0; i < (size_t)eventCount; ++i ){
				pollEvent_s* ev = epollEvent[i].data.ptr;
				if( ev->fd == -1 ) {
					dbg_warning("no fd on %ld",i); 
					continue;
				}
				
				if( ev->callback(dp, epollEvent[i].events, ev->arg) ){
					return DEADPOLL_ERROR;
				}
			}
			if( timems ) *timems -= time_ms() - timer;
		return DEADPOLL_EVENT;
	}

	return DEADPOLL_ERROR;
}


int deadpoll_loop(deadpoll_s* dp, long timems){
	long timestart = timems;

	while(1){
		switch( deadpoll_event(dp, &timems) ){
			case DEADPOLL_ERROR: return DEADPOLL_ERROR;
			case DEADPOLL_TIMEOUT: break;
			case DEADPOLL_EVENT:
				if( timems && dp->timeout){
					timems -= timestart;
					while( timems < 0 ){ timems += timestart; }
				}
				else{
					timems = timestart;
				}
			break;
		}
	}
	return DEADPOLL_ERROR;	
}


