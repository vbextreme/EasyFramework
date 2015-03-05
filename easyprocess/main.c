#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include "easyprocess.h"
#include <easyconsole.h>

VOID sh(INT32 s)
{
	printf("sig:%d\n",s);
}

VOID printinfo(PISTAT* pi)
{
	if ( !pi )
	{
		printf("----------------------------------------------------------\n");
		printf("-  PID  -         NAME        - STATE -  MEM mb  - NTHR -\n");
		printf("----------------------------------------------------------\n");
		return;
	}
	
	printf("-%7d-%21.21s-   %c   -%10.3f-%6d-\n",pi->pid,pi->name,pi->state,((double)pi->vmemsize/1024.0)/1024.0,pi->nthreads);
}

int main(int argc, char** argv)
{
	PISCK pi;
	
	if ( !pro_info_sck(&pi,PI_SCK_UNIX) )
	{
		puts("no sck");
		return 0;
	}
	
	//printf("sl(%2d) ip(%s)(%s) port(%u)(%u) state(%s)\n",pi.tcp.slot,pi.tcp.lip,pi.tcp.rip,pi.tcp.lport,pi.tcp.rport, pro_tcp_status(pi.tcp.status));
	printf("sl(%2d) ref(%u) state(%s) path(%s)\n",pi.nx.num,pi.nx.refcount,pro_tcp_status(pi.nx.status),pi.nx.path);
	while ( pro_info_sck(&pi,PI_SCK_CONTINUE) )
	{
		//printf("sl(%2d) ip(%s)(%s) port(%u)(%u) state(%s)\n",pi.tcp.slot,pi.tcp.lip,pi.tcp.rip,pi.tcp.lport,pi.tcp.rport, pro_tcp_status(pi.tcp.status));
		printf("sl(%2d) ref(%u) state(%s) path(%s)\n",pi.nx.num,pi.nx.refcount,pro_tcp_status(pi.nx.status),pi.nx.path);
	}
	
	return 0;
	/*
	FLOAT64 d,u;
	puts("Internet speed Download, Upload");
	UINT32 y,x;
	con_getrc(&y,&x);
	
	while(1)
	{
		if ( pro_net_speed(&d,&u,"wlan12",0.2) )
		{
			con_gotorc(y,x);
			printf("%8.2fkb %8.2fkb",d,u);
			con_flush();
		}
	}
	*/
	
	/*
	PINET pi;
	if ( !pro_info_net(&pi) ) { puts("error 0"); return 0;}
	if ( pi.count == 0 ) {puts("no network"); return 0;}
	
	INT32 i;
	for ( i = 0; i < pi.count; ++i)
	{
		printf("%s %u %u\n",pi.face[i],pi.recvbyte[i],pi.sendbyte[i]);
	}
	
	return 0;
	*/
	/*
	FLOAT64 us[5];
	if ( !pro_cpu_usage(us,1.0) ) { puts("error 0"); return 0;}
	
	printf("coreA:%.1f\n",us[0]);
	printf("core1:%.1f\n",us[1]);
	printf("core2:%.1f\n",us[2]);
	printf("core3:%.1f\n",us[3]);
	printf("core4:%.1f\n",us[4]);
	return 0;
	*/
	/*
	PIKCPU pi;
	if ( !pro_info_kcpu(&pi) ) { puts("error 0"); return 0;}
	
	INT32 i;
	for ( i = 0; i < pi.ncpu; ++i)
	{
		printf("cpu%d %u %u %u\n",i,pi.user[i],pi.softirq[i],pi.guestnice[i]);
	}
	printf("boot:%u\n",pi.boottime);
	printf("blk:%u\n",pi.pblk);
	return 0;
	*/
	
	/*
	PIMODULE pi;
	if ( !pro_info_modules(&pi) ) { puts("error 0"); return 0;}
	
	INT32 i;
	for ( i = 0; i < pi.count; ++i)
	{
		printf("%s sz:%u us:%u ",pi.name[i],pi.size[i],pi.nused[i]);
		INT32 k;
		for ( k = 0; k < pi.nused[i]; ++k )
		{
			printf("%s ",pi.from[i][k]);
		}
		printf("\n");
	}
	return 0;
	*/
	/*
	PIMEMI pi;
	if ( !pro_info_meminfo(&pi) ) { puts("error 0"); return 0;}
	printf("total:%u free:%u vmt:%u vmu:%u\n",pi.total,pi.free,pi.vmalloctotal,pi.vmallocused);
	return 0;
	*/
	/*
	PICPU cpu;
	
	if ( !pro_info_cpu(&cpu) ) { puts("error 0"); return 0;}
	
	INT32 i;
	for ( i = 0; i < cpu.ncore; ++i)
	{
		printf("[%d]%s\n",cpu.core[i].n,cpu.core[i].model);
		printf("%f %s\n",cpu.core[i].mips,cpu.core[i].features);
		printf("%d\n\n",cpu.core[i].revision);
	}
	printf("hw:%s rv:%s se:%s\n",cpu.hardware,cpu.revision,cpu.serial);
	return 0;
	*/
	/*
	PISTAT pi;
	PIMEM mm;
	UINT32 npid = 0;
	UINT32 nthr = 0;
	UINT32 nmem = 0;
	PID pid;
	
	if ( (pid = pro_pid_lst(TRUE)) != -1 )
	{
		printinfo(NULL);
		if ( pro_info_stat(&pi,pid) )
		{
			if ( !pro_info_mem(&mm,pid) ) exit(0);
			pi.vmemsize = mm.size;
			printinfo(&pi);
			++npid;
			nthr += pi.nthreads;
			nmem += ((double)mm.size / 1024.0) / 1024.0;
		}
		
		while ( (pid = pro_pid_lst(FALSE)) != -1 )
		{
			if ( pro_info_stat(&pi,pid) )
			{
				if ( !pro_info_mem(&mm,pid) ) exit(0);
				pi.vmemsize = mm.size;
				printinfo(&pi);
				++npid;
				nthr += pi.nthreads;
				nmem += ((double)mm.size / 1024.0) / 1024.0;;
			}
		}
		printf("---------------------------------------------------------\n");	
		printf("-%7u-                     -       -%10.3f-%6u-\n",npid,((double)nmem/1024.0),nthr);
		printf("---------------------------------------------------------\n");	
	}
	
	return 0;
	*/
	/*
	sig_set(NULL,SIGINT,sh,FALSE,TRUE);
	while(1);
	
	puts("exit code");
	return 0;
	
	
	PIPE pi;
	if ( pro_pipe(&pi) ) {puts("error pipe"); return 0;}
	
	INT32 pc = pro_sh("ls",&pi,NULL);
		if (pc == -1 ) {puts("error fork"); return 0;}
	
	CHAR bu[1024];
	
	while ( fgets(bu,1024,pi.f))
		printf("%s",bu);
	
	INT32 v;
	PROSTATE ps = pro_pidstate(&v,pc,FALSE);
	
	switch(ps)
	{
		case P_RUN:
			puts("is running?");
		break;
		
		case P_EXITED:
			printf("exit:%d\n",v);
		break;
		
		case P_ONSIGNAL:
			printf("recsig:%d\n",v);
		break;
		
		default: puts("error state"); break;
	}
	
	
	
	return 0;
	*/
}

#endif
