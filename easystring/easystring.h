#ifndef EASYSTRING_H_INCLUDED
#define EASYSTRING_H_INCLUDED

#include <easytype.h>
#include <string.h>
#include <regex.h>

typedef regex_t REGEX;
#define REX_NOMATCH REG_NOMATCH

/// espressioni regolari
/// "" trova stringa uguale
/// . qualsiasi carattere != \n
/// [] uno di quelli, - nell'intervallo tra, '[' ']'
/// [^] negazione
/// ^ inizio riga
/// $ fine riga
/// () sottoespressione riusabile
/// \n dove n sta per l'indice della sottoespressione
/// * zero o piu volte
/// + una o piu volte
/// ? zero o una volta
/// {min,max} ripetizioni
/// | or


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
CHAR* str_toend(CHAR* s);
CHAR* str_insc(CHAR* d, CHAR c);
CHAR* str_inss(CHAR* d, CHAR* s);
BOOL str_empty(CHAR* s);

#define rex_mk(REX,PAT) regcomp(REX,PAT,REG_EXTENDED)
#define rex_free(REX) regfree(REX)
INT32 rex_exec(CHAR** stout, CHAR** enout, CHAR** s, REGEX* r);
VOID rex_perror(INT32 err, REGEX* r);

#endif // EASYSTRING_H_INCLUDED
