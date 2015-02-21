#include "easylist.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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


