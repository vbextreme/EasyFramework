#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include "easyopt.h"

int main(int argc, char** argv)
{
	
	MYOPT o = opt_new(3,"Verbose,Expression:,selMatch::");
	opt_usage("test",o);
	
	INT32 c;
	CHAR* arg;
	
	while ( -1 != (c = opt_parse(&arg,o,argc,argv)) )
		printf("arg:%c%s%s\n",c,(arg) ? " = " : " ",(arg) ? arg : " ");
	
		
	opt_free(o);

/*
	INT32 optindex = 0;
	
	CHAR p[512];
	INT32 c;
	CHAR* carg;
	
	while ( -1 != (c = getopt_long (argc, argv, "hve:s:", long_options, &optindex)) )
    {
		if ( optarg )
		{
			if (optarg[0] == '=' )
				carg = &optarg[1];
			else
				carg = &optarg[0];
			carg = str_skipspace(carg);
		}
		else
		{
			carg = NULL;
		}
		
		switch (c)
		{
			default: case '?': case ':':case 'h': usage(); break;
			
			case 'v': verbose = TRUE; break;
			case 'p': port = atoi(optarg);	break;
			case 's':
				if ( carg )
				{
					if ( server_new(&irc,carg,port) )
						{if ( verbose ) puts("ok");}
					else
						{if ( verbose ) puts("error on create new server");}
					break;
				}
				view_dir(irc.path,TRUE);
			break;
			
			case 'n':
				if ( carg )
				{
					nick_set(&irc,carg,irc.user);
					if ( !verbose ) break;
				}
				printf("<%s>\n",irc.nick);
			break;
			
			case 'u':
				if ( carg )
				{
					nick_set(&irc,irc.nick,carg);
					if ( !verbose ) break;
				}
				printf("<!%s>\n",irc.user);
			break;
			
			case 'd':
				con_cls();
				debug(&irc);
				con_gets(p,512);
			break;
		}
	}
	
	CHAR inp[2048];
	
	while( fgets(inp,2048,stdin) )
	{
		
		
	}
	
	
	puts("Regular expression:");
	
	CHAR* s = "if ( \'ciao mondo\' )";
	
	printf("expression:\"%s\"\n",s);
	printf("regex:");
	con_flush();
	CHAR inp[1024];
	con_gets(inp,1024);
	if ( !inp[0] ) return 0;
	
	REGEX rex;
	
	INT32 ret = rex_mk(&rex,inp);
		if ( ret ) { rex_perror(ret,&rex); return 0; }
	
	CHAR* sto;
	CHAR* eno;
	CHAR* f = s;
	
	puts("match:");
	
	while ( 0 == (ret = rex_exec(&sto,&eno,&f,&rex)) )
	{
		printf("(%p)\'%.*s\'\n",sto,eno-sto,sto);
	}
	
	if ( REX_NOMATCH == ret )
	{
		puts(":end");
	}
	else
	{
		printf(":(%d)",ret);
		rex_perror(ret,&rex);
	}
*/
	return 0;
}

#endif
