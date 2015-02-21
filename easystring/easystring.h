#ifndef EASYSTRING_H_INCLUDED
#define EASYSTRING_H_INCLUDED

#include <easytype.h>
#include <string.h>

#define str_cmp(A,OP,B) (strcmp(A,B) OP 0)
#define ifstr(A,OP,B) if ( strcmp(A,B) OP 0 )

#define str_cpy(D,S) strcpy(D,S);
#define str_ncpy(D,S,N) strncpy(D,S,N);
#define str_len(S) strlen(S);

CHAR* str_skipspace(CHAR* s);
CHAR* str_skipline(CHAR* s);
CHAR* str_skipc(CHAR* s, CHAR skip);
CHAR* str_skips(CHAR* s, CHAR* skip);
CHAR* str_movetoc(CHAR* s, CHAR delimiter);
CHAR* str_movetos(CHAR* s, CHAR* delimiter);
CHAR* str_copytoc(CHAR* d, CHAR* s, CHAR delimiter);
CHAR* str_copytos(CHAR* d, CHAR* s, CHAR* delimiter);
CHAR* str_firstvalidchar(CHAR* s);


#endif // EASYSTRING_H_INCLUDED
