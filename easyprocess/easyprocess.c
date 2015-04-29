#include "easyprocess.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <easythread.h>
#include <easystring.h>
#include <easyfile.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
				
static CHAR tcpstatus[PI_SCK_TCP_MAX_STATES][15] = { "nostate","established","synsent","synrecv",
											         "finwait1","finwait2","timewait",
												     "close","closewait","lastack","listen",
												     "closing"};

/// ////// ///
/// SIGNAL ///
/// ////// ///

BOOL sig_set(SIG* old, INT32 sig, SIGCALL fnc, BOOL restart, BOOL restore)
{
	SIG sa;
	sa.sa_handler = fnc;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = (restart) ? SA_RESTART : 0;
	sa.sa_flags |= (restore) ? SA_RESETHAND : 0; 
	
	if ( sigaction(sig,&sa,old) < 0 ) return FALSE;
	return TRUE;
}

/// /////// ///
/// PROCESS ///
/// /////// ///

BOOL _infoparse(CHAR** name, CHAR** val, CHAR* buf, INT32 szb, FILE* f)
{
	CHAR* p;
	if ( !fgets(buf,szb,f) ) return FALSE;
	*name = buf;
	p = str_movetos(buf,"\t:");
		if ( !*p ) return FALSE;
	if ( *p == '\t' )
	{
		*p++ = '\0';
		p = str_movetoc(p,':');
		++p;
	}
	else
	{
		*p++ = '\0';
	}
	p = str_skipspace(p);
		if ( !*p ) return FALSE;
	*val = p;
	p = str_movetoc(p,'\n');
	*p = '\0';
	return TRUE;
}

CHAR* _myscan(CHAR** v, CHAR* s)
{
	if ( *s == '\0' ) return NULL;
	CHAR del = ' ';
	*v = s;
	if ( *s == '(' )
	{
		del = ')';
		++s;
		*v = s;
	}
	
	s = str_movetoc(s,del);
	if ( *s )
	{
		*s = '\0';
		++s;
		if ( del == ')' ) ++s;
	}
	return s;
} 

BOOL pro_info_io(PIIO* pi, PID pid)
{
	static CHAR *ln[] = { "rchar","wchar","syscr","syscw","read_bytes","write_bytes"};
	BOOL ck[6] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
	UINT32 *lv[6];
	
	lv[0] = &pi->allrd;	lv[1] = &pi->allwr;
	lv[2] = &pi->syscallrd; lv[3] = &pi->syscallwr;
	lv[4] = &pi->hdrd; lv[5] = &pi->hdwr;
	
	CHAR buf[1024];
	CHAR* n = NULL;
	CHAR* v = NULL;
	
	sprintf(buf,"/proc/%d/io",pid);
	
	FILE* f = fopen(buf,"r");
		if ( !f ) return FALSE;
	
	INT32 i;
	
	while ( _infoparse(&n,&v,buf,1024,f) )
	{
		for ( i = 0; i < 6; ++i )
		{
			if ( ck[i] ) continue;
			if ( !strcmp(ln[i],n) )
			{
				 *lv[i] = atoi(v);
				 ck[i] = TRUE;
				 break;
			}
		}
	}
	
	fclose(f);	
	return TRUE;
}

BOOL pro_info_stat(PISTAT* pi, PID pid)
{
	CHAR buf[1024];
	CHAR* b = buf;
	CHAR* eb;
	CHAR* v = NULL;
	
	sprintf(buf,"/proc/%d/stat",pid);
	
	FILE* f = fopen(buf,"r");
		if ( !f ) return FALSE;
	
	if ( !fgets(buf,1024,f) ) { fclose(f); return FALSE;}
	fclose(f);	
	
	pi->pid = strtol(b,&eb,10);
	b = eb + 1;
	b = _myscan(&v,b);
	strcpy(pi->name,v);
	pi->state = *b; b += 2;
	pi->parent = strtol(b,&eb,10); b = eb + 1;
	pi->pgrp = strtol(b,&eb,10); b = eb + 1;
	pi->session = strtol(b,&eb,10);	b = eb + 1;
	pi->ttynr = strtol(b,&eb,10); b = eb + 1;
	pi->tpgid = strtol(b,&eb,10); b = eb + 1;
	pi->flags = strtoul(b,&eb,10); b = eb + 1;
	pi->minfaults = strtoul(b,&eb,10); b = eb + 1;
	pi->wminfaults = strtoul(b,&eb,10); b = eb + 1;
	pi->majfaults = strtoul(b,&eb,10); b = eb + 1;
	pi->wmajfaults = strtoul(b,&eb,10); b = eb + 1;
	pi->utime = strtoul(b,&eb,10); b = eb + 1;
	pi->stime = strtoul(b,&eb,10); b = eb + 1;
	pi->wutime = strtoul(b,&eb,10);	b = eb + 1;
	pi->wstime = strtoul(b,&eb,10);	b = eb + 1;
	pi->rawprio = strtol(b,&eb,10);	b = eb + 1;
	pi->prio = strtol(b,&eb,10); b = eb + 1;
	pi->nthreads = strtol(b,&eb,10); b = eb + 1;
	pi->itrealvalue = strtol(b,&eb,10);	b = eb + 1;
	pi->sttime = strtoull(b,&eb,10); b = eb + 1;
	pi->vmemsize = strtol(b,&eb,10); b = eb + 1;
	pi->rss = strtol(b,&eb,10);	b = eb + 1;
	pi->rsslim = strtoul(b,&eb,10);	b = eb + 1;
	pi->stcode = strtoul(b,&eb,10); b = eb + 1;
	pi->encode = strtoul(b,&eb,10); b = eb + 1;
	pi->ststack = strtoul(b,&eb,10); b = eb + 1;
	pi->esp = strtoul(b,&eb,10); b = eb + 1;
	pi->eip = strtoul(b,&eb,10); b = eb + 1;
	pi->signal = strtoul(b,&eb,10); b = eb + 1;
	pi->blocked = strtoul(b,&eb,10); b = eb + 1;
	pi->sigignore = strtoul(b,&eb,10); b = eb + 1;
	pi->sigcatch = strtoul(b,&eb,10); b = eb + 1;
	pi->wchan = strtoul(b,&eb,10); b = eb + 1;
	pi->pagswap = strtoul(b,&eb,10); b = eb + 1;
	pi->cumpagswap = strtoul(b,&eb,10); b = eb + 1;
	pi->exitsignal = strtol(b,&eb,10); b = eb + 1;
	pi->processor = strtol(b,&eb,10); b = eb + 1;
	pi->rtprio = strtoul(b,&eb,10); b = eb + 1;
	pi->policy = strtoul(b,&eb,10); b = eb + 1;
	pi->delayio = strtoull(b,&eb,10); b = eb + 1;
	pi->guesttime = strtoul(b,&eb,10); b = eb + 1;
	pi->cguesttime = strtol(b,&eb,10);
	
	return TRUE;
}

BOOL pro_info_mem(PIMEM* pi, PID pid)
{
	CHAR buf[1024];
	CHAR* b = buf;
	CHAR* eb;
	
	sprintf(buf,"/proc/%d/statm",pid);
	
	FILE* f = fopen(buf,"r");
		if ( !f ) return FALSE;
	
	if ( !fgets(buf,1024,f) ) { fclose(f); return FALSE;}
	fclose(f);	
	
	pi->size = strtoul(b,&eb,10); b = eb + 1;
	pi->resident = strtoul(b,&eb,10); b = eb + 1;
	pi->share = strtoul(b,&eb,10);	b = eb + 1;
	pi->text = strtoul(b,&eb,10); b = eb + 1;
	pi->lib = strtoul(b,&eb,10); b = eb + 1;
	pi->data = strtoul(b,&eb,10); b = eb + 1;
	pi->dt = strtoul(b,&eb,10); b = eb + 1;
	
	return TRUE;
}

PID pro_pid_lst(BOOL reset)
{
	CHAR n[512];
	if ( reset )
	{
		CHAR* ph = "/proc";
		if ( dir_list(n,TRUE,FT_DIR,ph) < 0 ) return -1;
		if ( n[0] >= '0' && n[0] <= '9') return atoi(n);
	}
	
	while ( dir_list(n,TRUE,FT_DIR,NULL) != -1 )
		if ( n[0] >= '0' && n[0] <= '9') return atoi(n);
	
	return -1;
}

BOOL pro_info_cpu(PICPU* pi)
{
	static CHAR *ln[] = { "processor","model name","BogoMIPS","Features","CPU implementer",
		                  "CPU architecture","CPU variant","CPU part","CPU revision"};
		
	BOOL ck[9] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
	VOID *lv[9];
	static INT32 tlv[9] = {0,2,1,2,0,0,0,0,0};
	
	CHAR buf[1024];
	CHAR* n = NULL;
	CHAR* v = NULL;
	
	FILE* f = fopen("/proc/cpuinfo","r");
		if ( !f ) return FALSE;
	
	INT32 i;
	INT32 nc;
	for ( nc = 0; nc < 5; ++nc ) 
	{	
		lv[0] = &pi->core[nc].n; lv[1] = &pi->core[nc].model;
		lv[2] = &pi->core[nc].mips; lv[3] = &pi->core[nc].features;
		lv[4] = &pi->core[nc].implementer; lv[5] = &pi->core[nc].architecture;
		lv[6] = &pi->core[nc].variant; lv[7] = &pi->core[nc].part;
		lv[8] = &pi->core[nc].revision; 
	
		while ( _infoparse(&n,&v,buf,1024,f) )
		{	
			if ( !strcmp(n,"Hardware") ) goto ENDCORE;
			for ( i = 0; i < 9; ++i )
			{
				if ( ck[i] ) continue;
				if ( !strcmp(ln[i],n) )
				{
					switch ( tlv[i] )
					{
						case 0:
							*((INT32*)lv[i]) = atoi(v);
						break;
						case 1:
							*((FLOAT64*)lv[i]) = strtod(v,NULL);
						break;
						case 2:
							strcpy((CHAR*)lv[i],v);
						break;
					}
					ck[i] = TRUE;
				}
			}
		}
		memset(ck,FALSE,sizeof(BOOL) * 9);
	}
	if ( nc == 128 ) {fclose(f);return FALSE;}
	ENDCORE:
	pi->ncore = nc;
	strcpy(pi->hardware,v);
	if ( !_infoparse(&n,&v,buf,1024,f) ) {fclose(f);return FALSE;}
	strcpy(pi->revision,v);
	if ( !_infoparse(&n,&v,buf,1024,f) ) {fclose(f);return FALSE;}
	strcpy(pi->serial,v);
	
	fclose(f);	
	return TRUE;
}

BOOL pro_info_meminfo(PIMEMI* pi)
{
	static CHAR *ln[] = { "MemTotal", "MemFree", "Buffers", "Cached", "SwapCached",
						  "Active", "Inactive", "Active(anon)", "Inactive(anon)", "Active(file)",
						  "Inactive(file)", "Unevictable", "Mlocked","HighTotal","HighFree",
						  "LowTotal", "LowFree", "MmapCopy", "SwapTotal", "SwapFree",
						  "Dirty", "Writeback", "AnonPages", "Mapped", "Shmem",
						  "Slab", "SReclaimable", "SUnreclaim", "KernelStack", "PageTables",
						  "Quicklists", "NFS_Unstable", "Bounce", "WritebackTmp", "CommitLimit",
						  "Committed_AS", "VmallocTotal", "VmallocUsed", "VmallocChunk", "HardwareCorrupted",
						  "AnonHugePages", "HugePages_Total", "HugePages_Free", "HugePages_Rsvd", "HugePages_Surp",
						  "Hugepagesize" };
	BOOL ck[46] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,
				   FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,
				   FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,
				   FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,
				   FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
	UINT32 *lv[46];
	lv[0] = &pi->total;	lv[1] = &pi->free; lv[2] = &pi->buffers; lv[3] = &pi->cached; lv[4] = &pi->swapcached;
	lv[5] = &pi->active; lv[6] = &pi->inactive; lv[7] = &pi->activeanon; lv[8] = &pi->inactiveanon; lv[9] = &pi->activefile;
    lv[10] = &pi->inactivefile; lv[11] = &pi->unevictable; lv[12] = &pi->mlocked; lv[13] = &pi->hightotal; lv[14] = &pi->highfree;
    lv[15] = &pi->lowtotal; lv[16] = &pi->lowfree; lv[17] = &pi->mmapcopy; lv[18] = &pi->swaptotal; lv[19] = &pi->swapfree;
    lv[20] = &pi->dirty; lv[21] = &pi->writeback; lv[22] = &pi->anonpages; lv[23] = &pi->mapped; lv[24] = &pi->shmem;
    lv[25] = &pi->slab; lv[26] = &pi->sreclaimable; lv[27] = &pi->sunreclaim; lv[28] = &pi->kernelstack; lv[29] = &pi->pagetables;
    lv[30] = &pi->quicklists; lv[31] = &pi->nfsunstable; lv[32] = &pi->bounce; lv[33] = &pi->writebacktmp; lv[34] = &pi->commitlimit;
    lv[35] = &pi->committedas; lv[36] = &pi->vmalloctotal; lv[37] = &pi->vmallocused; lv[38] = &pi->vmallocchunk; lv[39] = &pi->hardwarecorrupted;
    lv[40] = &pi->anonhugepages; lv[41] = &pi->hugepagestotal; lv[42] = &pi->hugepagesfree; lv[43] = &pi->hugepagesrsvd; lv[44] = &pi->hugepagessurp;
    lv[45] = &pi->hugepagesize;
	
	CHAR buf[1024];
	CHAR* n = NULL;
	CHAR* v = NULL;
	
	FILE* f = fopen("/proc/meminfo","r");
		if ( !f ) return FALSE;
	
	INT32 i;
	while ( _infoparse(&n,&v,buf,1024,f) )
	{
		for ( i = 0; i < 46; ++i )
		{
			if ( ck[i] ) continue;
			if ( !strcmp(ln[i],n) )
			{
				 *lv[i] = strtoul(v,NULL,10);
				 ck[i] = TRUE;
				 break;
			}
		}
	}
	
	for ( i = 0; i < 46; ++i )
	{
		if ( ck[i] ) continue;
		*lv[i] = 0;
		ck[i] = TRUE;
	}
	
	fclose(f);	
	return TRUE;
}

VOID _parse_mod(PIMODULE* pi,INT32 id, CHAR* line)
{
	CHAR* p;
	
	line = str_copytoc(pi->name[id],line,' ');
	++line;
	pi->size[id] = strtoul(line,&p,10); line = p+1;
	pi->nused[id] = strtoul(line,&p,10); line = p+1;
	if ( *line == '-' )
	{
		pi->from[id][0][0] = '\0';
		return;
	}
	pi->from[id][pi->nused[id]][0] = '\0';
	INT32 i;
	for ( i = 0; i < pi->nused[id]; ++i)
	{	
		line = str_copytoc(pi->from[id][i],line,',');
		++line;
		if ( *line == ' ') break;
	}
	
}

BOOL pro_info_modules(PIMODULE* pi)
{
	CHAR buf[1024];
	
	FILE* f = fopen("/proc/modules","r");
		if ( !f ) return FALSE;
	
	pi->count = 0;
	while ( fgets(buf,1024,f) )
	{
		_parse_mod(pi,pi->count,buf);
		++pi->count;
	}
	
	fclose(f);
	return TRUE;
}

BOOL pro_info_kcpu(PIKCPU* pi)
{
	CHAR buf[2048];
	CHAR* b;
	CHAR* eb;
	
	FILE* f = fopen("/proc/stat","r");
		if ( !f ) return FALSE;
	
	pi->ncpu = 0;
	while ( fgets(buf,2048,f) )
	{
		b = buf;
		if ( strncmp(b,"cpu",3) ) break;
		b = str_movetoc(b,' ');
		b = str_skipspace(b);
		pi->user[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->nice[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->system[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->idle[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->iowait[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->irq[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->softirq[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->steal[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->guest[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		pi->guestnice[pi->ncpu] = strtoul(b,&eb,10); b = eb+1;
		++pi->ncpu;
	}
	
	while ( fgets(buf,2048,f) )
	{
		if ( !strncmp(buf,"btime",5) )
		{
			b = &buf[6];
			pi->boottime = strtoul(b,NULL,10);
		}
		else if ( !strncmp(buf,"processes",9) )
		{
			b = &buf[10];
			pi->processes = strtoul(b,NULL,10);
		}
		else if ( !strncmp(buf,"procs_running",13) )
		{
			b = &buf[14];
			pi->prunning = strtoul(b,NULL,10);
		}
		else if ( !strncmp(buf,"procs_blocked",13) )
		{
			b = &buf[14];
			pi->pblk = strtoul(b,NULL,10);
		}
	}
	
	fclose(f);	
	
	return TRUE;
}

BOOL pro_cpu_usage(FLOAT64* ret, FLOAT64 secscan)
{
	PIKCPU spi;
	PIKCPU epi;
	if ( !pro_info_kcpu(&spi) ) return FALSE;
	thr_sleep(secscan);
	if ( !pro_info_kcpu(&epi) ) return FALSE;
	
	UINT32 sttick;
	UINT32 entick;
	UINT32 idle;
	INT32 i;
	for ( i = 0; i < spi.ncpu; ++i)
	{
		sttick = spi.user[i] + spi.nice[i] + spi.system[i] + spi.idle[i] + spi.iowait[i] + spi.irq[i] + spi.softirq[i] + spi.steal[i] + spi.guest[i] + spi.guestnice[i];
		entick = epi.user[i] + epi.nice[i] + epi.system[i] + epi.idle[i] + epi.iowait[i] + epi.irq[i] + epi.softirq[i] + epi.steal[i] + epi.guest[i] + epi.guestnice[i];
		entick -= sttick;
		idle = epi.idle[i] - spi.idle[i];
		*ret++ = ((FLOAT64)(entick - idle) / (FLOAT64) entick) * 100.0;
	}
	
	return TRUE;
}

BOOL pro_info_netarp(PINETARP* pi)
{	
	CHAR buf[1024];
	CHAR* v;
	CHAR* b;
	
	FILE* f = fopen("/proc/net/arp","r");
		if ( !f ) return FALSE;
	
	if ( !fgets(buf,1024,f) ) {fclose(f);return FALSE;}
	
	pi->count = 0;
	while ( fgets(buf,1024,f) )
	{
		b = str_copytos(pi->ip[pi->count],buf," \t");
		b = str_skipspace(b);
		
		pi->hwtype[pi->count] = strtoul(b,&v,10) ; b = v+1;
		b = str_skipspace(b);
		
		pi->flags[pi->count] = strtoul(b,&v,10) ; b = v+1;
		b = str_skipspace(b);
		
		b = str_copytos(pi->hw[pi->count],b," \t");
		b = str_skipspace(b);
		
		b = str_copytos(pi->mask[pi->count],b," \t");
		b = str_skipspace(b);
		
		b = str_copytos(pi->dev[pi->count],b," \t\n");
		
		++pi->count;
	}
	
	fclose(f);	
	return TRUE;
}

BOOL pro_info_net(PINET* pi)
{	
	CHAR buf[1024];
	CHAR* eb;
	CHAR* b;
	
	FILE* f = fopen("/proc/net/dev","r");
		if ( !f ) return FALSE;
	
	if ( !fgets(buf,1024,f) ) {fclose(f);return FALSE;}
	if ( !fgets(buf,1024,f) ) {fclose(f);return FALSE;}
	
	pi->count = 0;
	while ( fgets(buf,1024,f) )
	{
		b = str_skipspace(buf);
		b = str_copytoc(pi->face[pi->count],b,':');
		++b;
		b = str_skipspace(b);
		pi->recvbyte[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->recvpck[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->recverr[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->recvdrop[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->recvfifo[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->recvframe[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->recvcompressed[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->recvmulticast[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		
		pi->sendbyte[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->sendpck[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->senderr[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->senddrop[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->sendfifo[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->sendcolls[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->sendcarrier[pi->count] = strtoul(b,&eb,10) ; b = eb + 1;
		b = str_skipspace(b);
		pi->sendcompressed[pi->count] = strtoul(b,&eb,10) ;
		++pi->count;
	}
	
	fclose(f);	
	return TRUE;
}

BOOL pro_net_speed( FLOAT64* dw, FLOAT64* up, CHAR* face, FLOAT64 secscan)
{
	PINET spi;
	PINET epi;
	if ( !pro_info_net(&spi) ) return FALSE;
	thr_sleep(secscan);
	if ( !pro_info_net(&epi) ) return FALSE;
	
	INT32 i;
	for ( i = 0; i < spi.count; ++i)
	{
		if ( strcmp(face,spi.face[i]) ) continue;
		
		*dw = epi.recvbyte[i] - spi.recvbyte[i];
		*dw = ((1.0 / secscan) * *dw)/1024.0;
		*up = epi.sendbyte[i] - spi.sendbyte[i];
		*up = ((1.0 / secscan) * *up)/1024.0;
		break;
	}
	
	return TRUE;
}

BOOL pro_info_sck(PISCK* pi, UINT32 model)
{
	static FILE* f = NULL;
	static UINT32 stmod;
	CHAR buf[1024];
	CHAR* eb;
	CHAR* b;
	
	switch ( model )
	{
		case PI_SCK_TCP:
			if ( f ) fclose(f);
			f = fopen("/proc/net/tcp","r");
				if ( !f ) return FALSE;
			if ( !fgets(buf,1024,f) ) { fclose(f); f = NULL; return FALSE; }
			stmod = PI_SCK_TCP;
		break;
		
		case PI_SCK_TCP6:
			if ( f ) fclose(f);
			f = fopen("/proc/net/tcp6","r");
				if ( !f ) return FALSE;
			if ( !fgets(buf,1024,f) ) { fclose(f); f = NULL; return FALSE; }
			stmod = PI_SCK_TCP6;
		break;
		
		case PI_SCK_UDP:
			if ( f ) fclose(f);
			f = fopen("/proc/net/udp","r");
				if ( !f ) return FALSE;
			if ( !fgets(buf,1024,f) ) { fclose(f); f = NULL; return FALSE; }
			stmod = PI_SCK_UDP;
		break;
		
		case PI_SCK_UNIX:
			if ( f ) fclose(f);
			f = fopen("/proc/net/unix","r");
				if ( !f ) return FALSE;
			if ( !fgets(buf,1024,f) ) { fclose(f); f = NULL; return FALSE; }
			stmod = PI_SCK_UNIX;
		break;
		
		default: case PI_SCK_CONTINUE:
			if ( !f ) return FALSE;
		break;
	}
	
	if ( !fgets(buf,1024,f) ) { fclose(f); f = NULL; return FALSE; }
	
	if ( stmod == PI_SCK_UNIX)
	{
		
		b = str_skipspace(buf);
		pi->nx.num = strtoul(b,&eb,10); b = eb+2;
		pi->nx.refcount = strtoul(b,&eb,10); b = eb+1;
		pi->nx.protocol = strtoul(b,&eb,10); b = eb+1;
		pi->nx.flags = strtoul(b,&eb,10); b = eb+1;
		pi->nx.type = strtoul(b,&eb,10); b = eb+1;
		pi->nx.status = strtoul(b,&eb,10); b = eb+1;
		b = str_skipspace(b);
		pi->nx.inode = strtoul(b,&eb,10); b = eb+1;
		str_copytos(pi->nx.path,b," \n\t");
		return TRUE;
	}
	
	struct in_addr iat;
	struct in6_addr i6at;
	
	b = str_skipspace(buf);
	pi->tcp.slot = strtoul(b,&eb,10); b = eb+1;
	b = str_skipspace(b);
	
	if ( stmod == PI_SCK_TCP6 )
	{
		CHAR hex[3] = {0,0,0};
		INT32 i;
		for ( i = 0; i < 16; ++i )
		{
			hex[0] = *b++;
			hex[1] = *b++;
			i6at.__in6_u.__u6_addr8[i] = strtoul(hex,NULL,16);
		}
		inet_ntop(AF_INET6,&i6at,pi->tcp.lip,40);
		++b;
	}
	else
	{
		iat.s_addr = strtoul(b,&eb,16); b = eb+1;
		inet_ntop(AF_INET,&iat,pi->tcp.lip,40);
	}
	
	pi->tcp.lport = strtoul(b,&eb,16); b = eb+1;
	
	if ( stmod == PI_SCK_TCP6 )
	{
		CHAR hex[3] = {0,0,0};
		INT32 i;
		for ( i = 0; i < 16; ++i )
		{
			hex[0] = *b++;
			hex[1] = *b++;
			i6at.__in6_u.__u6_addr8[i] = strtoul(hex,NULL,16);
		}
		inet_ntop(AF_INET6,&i6at,pi->tcp.rip,40);
		++b;
	}
	else
	{
		iat.s_addr = strtoul(b,&eb,16); b = eb+1;
		inet_ntop(AF_INET,&iat,pi->tcp.rip,40);
	}
	
	pi->tcp.rport = strtoul(b,&eb,16); b = eb+1;
	pi->tcp.status = strtoul(b,&eb,16); b = eb+1;
	pi->tcp.qtx = strtoul(b,&eb,16); b = eb+1;
	pi->tcp.qrx = strtoul(b,&eb,16);
	
	return TRUE;
}

CHAR* pro_tcp_status(UINT32 status)
{
	if ( status == 0 || status >= PI_SCK_TCP_MAX_STATES ) return NULL;
	return tcpstatus[status];
}

/// //////////////////////////////////////////////////////////////////////////////////////////

INT32 pro_pipe(PIPE* p)
{
	INT32 fp[2];
	INT32 ret;
	if ( (ret = pipe(fp)) < 0 ) return -1;
	p->inp = fp[0];
	p->out = fp[1];
	p->finp = fdopen(p->inp, "r");
	p->fout = fdopen(p->inp, "w");
	return 0;
}

VOID pro_pipe_closeread(PIPE* p)
{
	if (p->finp) fclose(p->finp);
	if (p->inp != -1 ) close(p->inp);
	p->inp = -1;
	p->finp = NULL;
}

VOID pro_pipe_closewrite(PIPE* p)
{
	if ( p->fout ) fclose(p->fout);
	if ( p->out != -1 ) close(p->out);
	p->out = -1;
	p->fout = NULL;
}

VOID pro_pipe_close(PIPE* p)
{
	pro_pipe_closeread(p);
	pro_pipe_closewrite(p);
}

VOID pro_pipe_cpr_pipetofd(PIPE* p, INT32 fd)
{
	dup2(p->inp,fd);
}

VOID pro_pipe_cpr_fdtopipe(PIPE* p, INT32 fd)
{
	fclose(p->finp);
	dup2(fd,p->inp);
	p->finp = fdopen(p->inp, "r");
}

VOID pro_pipe_cpw_pipetofd(PIPE* p, INT32 fd)
{
	dup2(p->out,fd);
}

VOID pro_pipe_cpw_fdtopipe(PIPE* p, INT32 fd)
{
	fclose(p->fout);
	dup2(fd,p->out);
	p->fout = fdopen(p->out, "w");
}


PID pro_bash(CHAR* cmd, PIPE* p, INT32 mode)
{
	INT32 ret;
	switch ( (ret = pro_fork()) )
	{
		case PRO_ERROR:	return -1;
		
		case PRO_CHILD:
			if ( p )
			{
				if ( mode & PRO_PIPE_RED_INP ) pro_pipe_cpr_pipetofd(p,STDIN_FILENO);
				if ( mode & PRO_PIPE_RED_OUT ) pro_pipe_cpw_pipetofd(p,STDOUT_FILENO);
				if ( mode & PRO_PIPE_RED_ERR ) pro_pipe_cpw_pipetofd(p,STDERR_FILENO);
				pro_pipe_close(p);
			}	
			execl("/bin/bash", "bash", "-c", cmd, (char *)0);
			_exit(-1);
		break;
		
		default: 
			if ( mode & ~PRO_PIPE_RED_INP ) pro_pipe_closewrite(p);
			if ( mode & ~PRO_PIPE_RED_OUT && mode & ~PRO_PIPE_RED_ERR ) pro_pipe_closeread(p);
		return ret;
	}
	return -1;
}

PID pro_execvp(CHAR* app, CHAR** argv, PIPE* p, INT32 mode)
{
	INT32 ret;
	switch ( (ret = pro_fork()) )
	{
		case PRO_ERROR:	return -1;
		
		case PRO_CHILD:
			if ( p )
			{
				if ( mode & PRO_PIPE_RED_INP ) pro_pipe_cpr_pipetofd(p,STDIN_FILENO);
				if ( mode & PRO_PIPE_RED_OUT ) pro_pipe_cpw_pipetofd(p,STDOUT_FILENO);
				if ( mode & PRO_PIPE_RED_ERR ) pro_pipe_cpw_pipetofd(p,STDERR_FILENO);
				pro_pipe_close(p);
			}	
			execvp(app, argv);
			_exit(-1);
		break;
		
		default: 
			if ( mode & ~PRO_PIPE_RED_INP ) pro_pipe_closewrite(p);
			if ( mode & ~PRO_PIPE_RED_OUT && mode & ~PRO_PIPE_RED_ERR ) pro_pipe_closeread(p);
		return ret;
	}
	return -1;
}
PROSTATE pro_pid_state(INT32* ex, PID p, BOOL async)
{
	
	INT32 flag =  WUNTRACED | WCONTINUED;
		if (async) flag |= WNOHANG ;
		
	INT32 st;
	if ( -1 ==  waitpid(p,&st,flag) ) return -1;
	
	if ( WIFEXITED(st) )
	{
		*ex = WEXITSTATUS(st);
		return P_EXITED;
	}
	
	if ( WIFSIGNALED(st) )
	{
		*ex = WTERMSIG (st);
		return P_ONSIGNAL;
	}
	
	if ( WIFSTOPPED(st) )
	{
		*ex = WSTOPSIG(st);
		return P_STOP;
	}
	
	if ( WIFCONTINUED(st) )
	{
		*ex = 0;
		return P_CONTINUE;
	}
		
	return P_RUN;
}
