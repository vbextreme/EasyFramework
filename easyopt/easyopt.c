#include "easyopt.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct __MYOPT
{
	struct option* lopt;
	CHAR* sopt;
	int nopt;
}_MYOPT;

MYOPT opt_new(INT32 narg, CHAR* format)
{
	_MYOPT* opt = malloc(sizeof(_MYOPT));
	
	opt->nopt = narg;
	opt->lopt = calloc(narg + 1,sizeof(struct option));
	opt->sopt = malloc((narg * 3) + 1);
	
	CHAR sa;
	CHAR inp[80];
	CHAR* d;
	INT32 ni;
	CHAR* sop = opt->sopt;
	INT32 i = 0;
	
	while ( *format && i < narg)
	{
		ni = 0;
		d = inp;
		sa = '\0';
		while ( *format && *format != ',' && *format != ':' && 79 > ++ni )
		{
			if ( *format >= 'A' && *format <= 'Z' )
			{
				sa = *format + 'a' - 'A';
				*d++ = sa;
				++format;
			}
			else
			{
				*d++ = *format++;
			}
		}
		*d = '\0';
		
		while ( *format && *format != ',' && *format != ':') ++format;
		
		if ( sa != '\0' ) *sop++ = sa;
		
		CHAR* temp = malloc(ni+1);
		strcpy(temp,inp);
		opt->lopt[i].name = temp;
		
		opt->lopt[i].has_arg = 0;
		if ( *format == ':' )
		{
			++opt->lopt[i].has_arg;
			++format;
			*sop++ = ':';
			if ( *format == ':' )
			{
				++opt->lopt[i].has_arg;
				++format;
				*sop++ = ':';
			}
		}
		
		opt->lopt[i].val = sa;
		opt->lopt[i].flag = NULL;
		++i;
		
		if ( *format == ',' ) ++format;
	}
	
	*sop = '\0';
	
	return opt;
}

VOID opt_free(MYOPT opt)
{
	INT32 i;
	
	for ( i = 0; i < opt->nopt; ++i)
	{
		free((CHAR*)opt->lopt[i].name);
	}
	free(opt->lopt);
	free(opt->sopt);
	free(opt);
}

VOID opt_usage(CHAR* name, MYOPT opt)
{
	printf("usage %s ",name);
	
	INT32 i;
	CHAR* p;
	BOOL up;
	
	for ( i = 0; i < opt->nopt; ++i)
	{
		if ( opt->lopt[i].has_arg == 2 ) printf("[");
		printf("--");
		p =(CHAR*) opt->lopt[i].name;
		up = FALSE;
		while ( *p )
		{
			if ( !up && opt->lopt[i].val == *p )
			{
				putchar(*p - ('a' - 'A'));
				up = TRUE;
			}
			else
			{
				putchar(*p);
			}
			++p;
		}
		
		if ( opt->lopt[i].has_arg == 1 )
		{
			printf("=VALUE");
		}
		else if ( opt->lopt[i].has_arg == 2 )
		{
			printf("=OPTIONAL]");
		}
		putchar(' ');
	}
	
	printf("\n");
}

INT32 opt_parse(CHAR** carg, MYOPT opt, INT32 argc, CHAR** argv)
{
	INT32 optindex = 0;
	INT32 c;
	
	c = getopt_long (argc, argv, opt->sopt, opt->lopt, &optindex);
	if ( c == -1 ) return -1;
	
	if ( optarg )
	{
		if (optarg[0] == '=' )
			*carg = &optarg[1];
		else
			*carg = &optarg[0];
		while ( **carg && (**carg == ' ' || **carg == '\t' ) ) ++(*carg);
	}
	else
	{
		*carg = NULL;
	}
	
	return (CHAR)c;
}







