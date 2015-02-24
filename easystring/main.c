#ifdef _APP

#include <stdio.h>
#include <stdlib.h>

#include "easystring.h"
#include <easyconsole.h>
#include <easyopt.h>

int main(int argc, char** argv)
{
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
}

#endif
