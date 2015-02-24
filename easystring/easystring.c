#include "easystring.h"

#include <stdlib.h>
#include <stdio.h>

CHAR* str_skipspace(CHAR* s)
{
	while ( *s && (*s == ' ' || *s == '\t') ) ++s;
	return s;
}

CHAR* str_skipline(CHAR* s)
{
	while ( *s && *s != '\n' ) ++s;
	if ( *s ) ++s;
	return s;
}


CHAR* str_skipc(CHAR* s, CHAR skip)
{
	while ( *s && *s == skip ) ++s;
	return s;
}

CHAR* str_skips(CHAR* s, CHAR* skip)
{
	CHAR* dd;
	while( *s )
	{
		for ( dd = skip; *dd; ++dd)
			if ( *s == *dd ) {++s;continue;}
		break; 
	}
	return s;
}

CHAR* str_movetoc(CHAR* s, CHAR delimiter)
{
	while ( *s && *s != delimiter ) ++s;
	return s;
}

CHAR* str_movetos(CHAR* s, CHAR* delimiter)
{
	CHAR* dd;
	for (; *s; ++s )
	{
		for ( dd = delimiter; *dd; ++dd)
			if ( *s == *dd ) {return s;} 
	}
	return s; 
}

CHAR* str_copytoc(CHAR* d, CHAR* s, CHAR delimiter)
{
	while ( *s && *s != delimiter ) *d++ = *s++;
	*d = '\0';
	return s;
}

CHAR* str_copytos(CHAR* d, CHAR* s, CHAR* delimiter)
{
	CHAR* dd;
	while ( *s )
	{
		for ( dd = delimiter; *dd; ++dd)
			if ( *s == *dd ) { *d = '\0'; return s;} 
		*d++ = *s++;
	}
	*d = '\0';
	return s; 
}

CHAR* str_firstvalidchar(CHAR* s)
{
	while ( *s && ( *s == ' ' || *s == '\t' || *s == '\n' ) ) ++s;
	return s; 
}

CHAR* str_toend(CHAR* s)
{
	while ( *s ) ++s;
	return s;
}

CHAR* str_insc(CHAR* d, CHAR c)
{
	UINT32 l = strlen(d) + 1;
	memmove(d+1,d,l);
	*d = c;
	return d + 1;
}

CHAR* str_inss(CHAR* d, CHAR* s)
{
	UINT32 ls = strlen(s);
	UINT32 ld = strlen(d) + 1;
	memmove(d+ls,d,ld);
	memcpy(d,s,ls);
	return d + ls;
}

BOOL str_empty(CHAR* s)
{
	return ( !s || !*s ) ? TRUE : FALSE;
}

INT32 rex_exec(CHAR** stout, CHAR** enout, CHAR** s, REGEX* r)
{
	regmatch_t rm;
	INT32 ret = regexec(r,*s,1,&rm,0);
	if ( 0 == ret )
	{
		*stout = rm.rm_so + *s;
		*enout = rm.rm_eo + *s;
		*s = *s + rm.rm_eo + 1;
		return 0;
	}
	return ret;
}
	
VOID rex_perror(INT32 err, REGEX* r)
{
	CHAR msg[0x1000];
	regerror(err,r,msg,0x1000);
	printf("rex %s\n",msg);
}



















