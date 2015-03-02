#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include "easyprocess.h"

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
