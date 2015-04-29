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

typedef struct __EDIT* EDIT;

#define str_cmp(A,OP,B) (strcmp(A,B) OP 0)
#define ifstr(A,OP,B) if ( strcmp(A,B) OP 0 )

#define str_cpy(D,S) strcpy(D,S);
#define str_ncpy(D,S,N) strncpy(D,S,N);
#define str_len(S) strlen(S);

CHAR* str_skipspace(register CHAR* s);
CHAR* str_skipline(register CHAR* s);
CHAR* str_skipc(register CHAR* s, register CHAR skip);
CHAR* str_skips(register CHAR* s, register CHAR* skip);
CHAR* str_movetoc(register CHAR* s, register CHAR delimiter);
CHAR* str_movetos(register CHAR* s, register CHAR* delimiter);
CHAR* str_copytoc(register CHAR* d, register CHAR* s, register CHAR delimiter);
CHAR* str_copytos(register CHAR* d, register CHAR* s, register CHAR* delimiter);
CHAR* str_firstvalidchar(register CHAR* s);
CHAR* str_toend(register CHAR* s);
CHAR* str_insc(register CHAR* d, register CHAR c);
CHAR* str_inss(register CHAR* d, register CHAR* s);
CHAR* str_del(register CHAR* d, register UINT32 n);
BOOL str_empty(register CHAR* s);
INT32 str_isvoc(register INT32 c);
CHAR* str_ltrim(register CHAR* s);
CHAR* str_rtrim(register CHAR* s);
CHAR* str_trim(register CHAR* s);

#define rex_mk(REX,PAT) regcomp(REX,PAT,REG_EXTENDED)
#define rex_free(REX) regfree(REX)
INT32 rex_exec(CHAR** stout, CHAR** enout, CHAR** s, REGEX* r);
VOID rex_perror(INT32 err, REGEX* r);

#endif // EASYSTRING_H_INCLUDED
