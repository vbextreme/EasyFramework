#include <ef/proc.h>
#include <ef/file.h>
#include <ef/str.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/delay.h>

__private size_t* avgold;
__private size_t* avgnew;
__private size_t avgtus;

int cpu_core_count(void){
	int ret;
	if( (ret=sysconf(_SC_NPROCESSORS_ONLN)) < 0 ){
		err_pushno("cpu count");
	}
	return ret;
}

size_t* cpu_tick_new(void){
	size_t ncores = cpu_core_count()+1;
	return mem_many(size_t, ncores * CPU_TIME_COUNT);
}

err_t cpu_tick_get(size_t* tick, int ncores){
	if( ncores < 0 ) ncores = cpu_core_count();
	++ncores;

	__fd_close int fps = fd_open(PATH_PROC_STAT, "r", 0);
	if( fps < 0 ){
		return -1;
	}
	__mem_free char* str = fd_slurp(NULL, fps, 4096, 1);
	const char* parse = str;

	for( size_t i = 0; (int)i < ncores; ++i ){
		parse = strpbrk(parse, " \t");
		if( parse == NULL ){
			err_push("on skip cpu");
			return -1;
		}
		parse = str_skip_h(parse);
		if( parse == NULL ){
			err_push("on skip space");
			return -1;
		}
		
		size_t it = i * CPU_TIME_COUNT;
		for(size_t k = 0; k < CPU_TIME_COUNT; ++k){
			char* en;
			tick[it++] = strtoul(parse, &en, 10);
			if( en == NULL ) return -1;
			parse = str_skip_hn(en);
		}
	}
	return 0;
}

size_t cpu_time_tick(size_t* tick){
	size_t full = 0;
	for(size_t i = 0; i < CPU_TIME_COUNT; ++i ){
		full += tick[i];
	}
	return full;
}

void cpu_auto_load_average_begin(void){
	avgold = cpu_tick_new();
	avgnew = cpu_tick_new();
	cpu_tick_get(avgold,0);
	cpu_tick_get(avgnew,0);
	avgtus = time_us();
}

void cpu_auto_load_average_end(void){
	free(avgold);
	free(avgnew);
}

double cpu_auto_load_average(unsigned core){
	if( time_us() - avgtus < 500*1000 ){
		delay_us( 500*1000 - (time_us() - avgtus) );
	}
	cpu_tick_get(avgnew,0);
	avgtus = time_us();
	double ret = cpu_load_average(avgold, avgnew, core, 0);
	SWAP(avgold, avgnew);
	return ret;
}


double cpu_load_average(size_t* tickS, size_t* tickE, unsigned core, int ncores){
	if( ncores < 0 ) ncores = cpu_core_count();
	++ncores;

	if( core > (unsigned)ncores ){
		dbg_warning("no core %u", core);
		return 0.0;
	}

	core *= CPU_TIME_COUNT; 
	size_t tick = cpu_time_tick(&tickE[core]) - cpu_time_tick(&tickS[core]);
	size_t idle = tickE[core+CPU_IDLE] - tickS[core+CPU_IDLE];
	double average = ((double)(tick - idle) / (double)tick) * 100.00; 
	//dbg_info("average %u %lf", core, average);
	return average;
}


/* sysinfo return incorrect value and not return cached, used /proc/meminfo */

__private size_t meminfo_parse(char* line){
	while( *line && (*line < '0' || *line > '9') ) ++line;
	iassert( *line );
	return strtoul(line, NULL, 10);
}

err_t meminfo_read(memInfo_s* mem){
	__private char* col[] = {
		"MemTotal:",
		"MemFree:",
		"MemAvailable:",
		"Buffers:",
		"Cached:",
		"SwapTotal:",
		"SwapFree:",
		"Shmem:",
		"SReclaimable:",
		"SUnreclaim:",
		NULL
	};

	size_t* memptr[] = {
		&mem->total,
		&mem->free,
		&mem->available,
		&mem->buffers,
		&mem->cached,
		&mem->totalswap,
		&mem->freeswap,
		&mem->shared,
		&mem->SReclaimable,
		&mem->SUnreclaim
	};

	__file_close file_t * fm = fopen(PATH_PROC_MEM, "r");
	if( fm == NULL ) {
		err_pushno("%s not available", PATH_PROC_MEM);
		return -1;
	}
	
	char inp[1024];
	while( fgets(inp, 1024, fm) ){
		for( size_t i = 0; col[i]; ++i ){
			if( !strncmp(inp, col[i], strlen(col[i])) ){
				*memptr[i] = meminfo_parse(inp);
			}
		}
	}

	mem->used = mem->total - (mem->free + mem->buffers + mem->cached + mem->SReclaimable);
	return 0;
}

err_t net_device(netDev_s* net, const char* device){
	const size_t len = strlen(device);

	__stream_close stream_s* fn = stream_open(PATH_PROC_NET_DEV, "r", 0 , 4096);
	if( !fn ){
		return -1;
	}
	
	if( stream_skip_line(fn) ){
		err_push("on skip first line");
		return -1;
	}
	if( stream_skip_line(fn) ){
		err_push("on skip second line");
		return -1;
	}
	
	while( stream_kbhit(fn) ){
		if( stream_skip_h(fn) ){
			err_push("on skip initial space");
			return -1;
		}
		char* name = NULL;
		if( stream_inp(fn, &name, ':', 0) ){
			err_push("on read dev name");
			return -1;
		}
		if( str_equal(name, strlen(name), device, len) ){
			stream_skip_line(fn);
			continue;
		}
	
		if( stream_skip_h(fn) ){
			err_push("on skip initial space");
			return -1;
		}
		size_t i;
		for( i = 0; i < ND_COUNT; ++i){
			if( stream_inp(fn, &net->receive[i], 10) ){
				err_push("read receive net bytes");
				return -1;
			}
			if( stream_skip_h(fn) ){
				err_push("on skip space");
				return -1;
			}
		}
		for( i = 0; i < ND_COUNT; ++i){
			if( stream_inp(fn, &net->transmit[i], 10) ){
				err_push("read transmit net bytes");
				return -1;
			}
			if( stream_skip_hnl(fn) ){
				err_push("on skip space/nl");
				return -1;
			}
		}
		break;
	}
	return 0;
}

int* proc_pid_fd(pid_t pid){
	char proc[PATH_MAX];
	sprintf(proc, PATHF_PROC_PID_FD, pid);
	
	__dir_close dir_s* d = dir_open(proc);
	if( d == NULL ){
		err_pushno("open dir %s", proc);
		return NULL;
	}
	
	int* ret = vector_new(int, 8, 8);
	if( !ret ) return NULL;

	dir_foreach(d, f){
		if( dirent_currentback(f) ) continue;
		int fd = strtol(dirent_name(f), NULL, 10); 
		vector_push_back(ret, fd);
	}

	return ret;
}

err_t proc_pid_stat(pidStat_s* ps, pid_t pid){
	char proc[PATH_MAX];
	sprintf(proc, "/proc/%d/stat", pid);
	__fd_close int fd = fd_open(proc, "r", 0);
	if( fd == -1 ){
		err_pushno("open pid %d", pid);
		return -1;
	}
	__mem_free char* data = fd_slurp(NULL, fd, 4096, 1);
	if( !data ){
		err_push("read pid %d", pid);
		return -1;
	}

	char* p = data;
	char* en;
	ps->pid = strtol(p, &en, 10);
	p = en + 2;

	char* d = ps->comm;
	while( *p && *p != ')' && *(p+1) !=' '){
		*d++ = *p++;
	}
	*d = 0;
	p += 2;

	ps->state = *p;
	p+=2;
	ps->ppid = strtol(p, &en, 10);
	p = en + 1;
	ps->pgrp = strtol(p, &en, 10);
	p = en + 1;
	ps->session = strtol(p, &en, 10);
	p = en + 1;
	ps->ttynr = strtol(p, &en, 10);
	p = en + 1;
	ps->tpgid = strtol(p, &en, 10);
	p = en + 1;
	ps->flags = strtoul(p, &en, 10);
	p = en + 1;
	ps->minflt = strtoul(p, &en, 10);
	p = en + 1;
	ps->cminflt = strtoul(p, &en, 10);
	p = en + 1;
	ps->majflt = strtoul(p, &en, 10);
	p = en + 1;
	ps->cmajflt = strtoul(p, &en, 10);
	p = en + 1;
	ps->utime = strtoul(p, &en, 10);
	p = en + 1;
	ps->stime = strtoul(p, &en, 10);
	p = en + 1;
	ps->cutime = strtol(p, &en, 10);
	p = en + 1;
	ps->cstime = strtol(p, &en, 10);
	p = en + 1;
	ps->priority = strtol(p, &en, 10);
	p = en + 1;
	ps->nice = strtol(p, &en, 10);
	p = en + 1;
	ps->numthreads = strtol(p, &en, 10);
	p = en + 1;
	ps->itrealvalue = strtol(p, &en, 10);
	p = en + 1;
	ps->statrtime = strtoul(p, &en, 10);
	p = en + 1;
	ps->vsize = strtoul(p, &en, 10);
	p = en + 1;
	ps->rss = strtol(p, &en, 10);
	p = en + 1;
	ps->rsslim = strtoul(p, &en, 10);
	p = en + 1;
	ps->startcode = strtoul(p, &en, 10);
	p = en + 1;
	ps->endcode = strtoul(p, &en, 10);
	p = en + 1;
	ps->startstack = strtoul(p, &en, 10);
	p = en + 1;
	ps->kstkesp = strtoul(p, &en, 10);
	p = en + 1;
	ps->kstkeip = strtoul(p, &en, 10);
	p = en + 1;
	ps->signal = strtoul(p, &en, 10);
	p = en + 1;
	ps->blocked = strtoul(p, &en, 10);
	p = en + 1;
	ps->sigignore = strtoul(p, &en, 10);
	p = en + 1;
	ps->sigcatch = strtoul(p, &en, 10);
	p = en + 1;
	ps->wchan = strtoul(p, &en, 10);
	p = en + 1;
	ps->nswap = strtoul(p, &en, 10);
	p = en + 1;
    ps->cnswap = strtoul(p, &en, 10);
	p = en + 1;
	ps->exitsignal = strtol(p, &en, 10);
	p = en + 1;
	ps->processor = strtol(p, &en, 10);
	p = en + 1;
	ps->rtpriority = strtoul(p, &en, 10);
	p = en + 1;
	ps->policy = strtoul(p, &en, 10);
	p = en + 1;
	ps->delayacctblkioticks = strtoul(p, &en, 10);
	p = en + 1;
	ps->guesttime = strtoul(p, &en, 10);
	p = en + 1;
	ps->cguesttime = strtol(p, &en, 10);
	p = en + 1;
	ps->startdata = strtoul(p, &en, 10);
	p = en + 1;
	ps->enddata = strtoul(p, &en, 10);
	p = en + 1;
	ps->startbrk = strtoul(p, &en, 10);
	p = en + 1;
	ps->argstart = strtoul(p, &en, 10);
	p = en + 1;
	ps->argend = strtoul(p, &en, 10);
	p = en + 1;
	ps->envstart = strtoul(p, &en, 10);
	p = en + 1;
	ps->envend = strtoul(p, &en, 10);
	p = en + 1;
	ps->exit_code = strtol(p, &en, 10);

	return 0;
}

