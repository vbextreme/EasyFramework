#ifdef _APP

#include <stdio.h>
#include <stdlib.h>

#include "easystring.h"
#include <easyconsole.h>
#include <easyopt.h>
#include <easybenchmark.h>

#ifdef _DEBUG
	BCH_PERF_GLOBAL;
#endif

#define STRLONG 4096

int main(int argc, char** argv)
{
	CHAR alfa[29] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ \n";
	CHAR salfa[9][4] = {"ABC","DEF","GHI","JKL","MNO","PQR","STU","VWX","YZ "};
		
	CHAR strlong[STRLONG];
	INT32 i, j, k;
	for ( i = 0, j = 0; i < STRLONG -1; ++i)
	{
		strlong[i] = alfa[j++];
		if (j == 29 ) j = 0;
	}
	strlong[i] = '\0';
	
	#ifdef _DEBUG
		BCH_PERF_INIT;
		BCH_PERF_START;
	#endif
	
	CHAR* s = strlong;
	CHAR dest[STRLONG];
	
	for ( i = 0, j = 0, k = 0; i < STRLONG; ++i, ++j, ++k)
	{
		if ( j > 27 ) j = 0;
		if ( k > 8 ) k = 0;
		
		s = strlong;
		s = str_skipspace(s);
		s = str_skipline(s);
		s = str_skipc(s,alfa[j]);
		s = str_skips(s,salfa[k]);
		s = str_movetoc(s,alfa[j]);
		s = str_movetos(s,salfa[k]);
		s = str_copytoc(dest,s,alfa[j]);
		s = str_copytos(dest,s,salfa[k]);
		s = str_firstvalidchar(s);
		str_insc(dest,'a');
		str_inss(dest,alfa);
		str_del(dest,10);
		s = str_toend(s);
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
		BCH_PERF_SAVE("/home/odroid/Croject/Easy/easystring.performance.1");
	#endif
	
	return 0;
	
	/*
	INT32 c;
	CHAR* carg;
	MYOPT opt = opt_new(4,"eXpression:,Startmatch:,Endmatch:,Listmode,Help");
	
	CHAR exp[2048];
	INT32 stm = 0;
	INT32 enm = -1;
	BOOL listmode = FALSE;
	
	while ( -1 != (c = opt_parse(&carg,opt,argc, argv)) )
    {
		switch (c)
		{
			default: case 'h': opt_usage("rex",opt); return 0;
			
			case 'x':
				if ( str_empty(carg) ) { opt_usage("rex",opt);break;}
				strcpy(exp,carg);
			break;
			
			case 's': stm = atoi(carg); if ( stm < 0 ) stm = 0; break;
			case 'e': enm = atoi(carg);	break;
			case 'l': listmode = TRUE; break;
				
		}
	}
	
	REGEX rex;
	INT32 ret = rex_mk(&rex,exp);
		if ( ret ) { rex_perror(ret,&rex); return 0; }
	
	CHAR inp[2048];
	CHAR* sto;
	CHAR* eno;
	CHAR* f;
	INT32 i=0;
	
	while( fgets(inp,2048,stdin) )
	{
		f = inp;
		while ( 0 == (ret = rex_exec(&sto,&eno,&f,&rex)) )
		{
			++i;
			if ( i < stm ) continue;
			if (listmode)
				printf("%.*s\n",eno-sto,sto);
			else
				printf("%.*s",eno-sto,sto);
			if ( enm > 0 && i >= enm) return 0;	
		}
		
		if ( REX_NOMATCH != ret )
		{
			rex_perror(ret,&rex);
			return 0;
		}
	}
	
	return 0;
	*/
}

#endif
