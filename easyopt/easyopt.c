#include "easyopt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <easyconsole.h>
#include <easystring.h>

typedef struct __MYOPT
{
	struct option* lopt;
	CHAR* sopt;
	CHAR** descript;
	INT32* chf;
	INT32 nopt;
}_MYOPT;

//~A is A
//A is a
//A(descript)
MYOPT opt_new(CHAR* format)
{
	INT32 narg = 1;
	INT32 i;
	CHAR* pf = format;
	for (; *pf; ++pf) if (*pf == ',') ++narg;
	
	_MYOPT* opt = malloc(sizeof(_MYOPT));
	
	opt->nopt = narg;
	opt->lopt = calloc(narg + 1,sizeof(struct option));
	opt->sopt = malloc((narg * 3) + 1);
	opt->chf = malloc(sizeof(INT32) * narg);
	opt->descript = malloc(sizeof(CHAR*) * narg);
	
	CHAR inp[80];
	CHAR* d;
	CHAR* sop = opt->sopt;
	INT32 ni;
	i = 0;
	
	do
	{
		if ( *format == '~' )
		{
			++format;
			opt->chf[i] = -1;
		}
		else
			opt->chf[i] = 0;
		
		ni = 0;
		d = inp;
		while ( *format && *format != ',' && *format != ':' && *format != '(' && 79 > ++ni )
		{
			if ( *format >= 'A' && *format <= 'Z' )
			{
				*d++ = *format + ('a' - 'A');
				//printf("CHF[%d]\n",);
				opt->chf[i] = ( opt->chf[i] ) ? *format : *format + ('a' - 'A');
				++format;
			}
			else
				*d++ = *format++;
		}
		*d = '\0';
		
		if ( opt->chf[i] > 0 ) *sop++ = opt->chf[i];
		
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
		
		if ( *format == '(' )
		{
			pf = str_movetos(++format,"),");
			INT32 szd = pf - format;
			opt->descript[i] = malloc( szd + 1 );
			strncpy(opt->descript[i],format,szd);
			opt->descript[i][szd] = '\0';
			format = pf + 1;
		}
		else
			opt->descript[i] = NULL;
		
		opt->lopt[i].val = (opt->chf[i] < 0) ? 0 : opt->chf[i];
		opt->lopt[i].flag = NULL;
		++i;
		
		format = str_movetoc(format,',');
		if ( *format == ',' ) ++format;
	}while( *format );
	
	*sop = '\0';
	
	return opt;
}

VOID opt_free(MYOPT opt)
{
	INT32 i;
	
	for ( i = 0; i < opt->nopt; ++i)
	{
		free((CHAR*)opt->lopt[i].name);
		if ( opt->descript[i] ) free(opt->descript[i]);
	}
	free(opt->descript);
	free(opt->chf);
	free(opt->lopt);
	free(opt->sopt);
	free(opt);
}

VOID opt_usage(CHAR* name, MYOPT opt)
{
	printf("usage %s ",name);
	
	INT32 i;
	CHAR* p;
	BOOL fin;
	
	for ( i = 0; i < opt->nopt; ++i)
	{
		printf("--");
		p = (CHAR*) opt->lopt[i].name;
		
		fin = FALSE;
		con_setcolor(0,CON_COLOR_WHYTE);
		while ( *p )
		{
			if ( tolower(opt->chf[i]) == *p && !fin)
			{
				fin = TRUE;
				con_setcolor(0,CON_COLOR_GREEN);
				putchar(opt->chf[i]);
				con_setcolor(0,CON_COLOR_WHYTE);
			}
			else
			{
				putchar(*p);
			}
			++p;
		}
		con_setcolor(0,0);
		
		if ( opt->lopt[i].has_arg == 1 )
		{
			printf("=VALUE");
		}
		else if ( opt->lopt[i].has_arg == 2 )
		{
			printf("=OPTIONAL");
		}
		
		if ( opt->descript[i] )
			printf("(%s)",opt->descript[i]);
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
	if ( (CHAR)c == '?' ) return -1;
	
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
	
	if ( c == 0 )
		return OPTLONG + optindex;
	
	return (CHAR)c;
}

const CHAR* opt_fromc(MYOPT opt, INT32 c)
{
	c -= OPTLONG;
	return ( c < 0 || c >= opt->nopt) ? NULL : opt->lopt[c].name;
}




