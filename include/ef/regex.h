#ifndef __EF_REGEX_H__
#define __EF_REGEX_H__

#include <ef/type.h>
#include <ef/vector.h>
#include <ctype.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

typedef struct regex{
	pcre2_code* rx;
	pcre2_match_data* match;
	PCRE2_SIZE* offv;
	size_t offc;
	const char* regex;
	const char* text;
	size_t lent;
	unsigned int newline;
	int utf8;
	int crlf;
}regex_s;

/** for init new regex on stack*/
#define REGEX_NEW() {\
	.rx = NULL,\
	.match = NULL,\
	.offv = NULL,\
	.offc = 0,\
	.regex = NULL,\
	.text = NULL,\
	.lent = 0,\
	.newline = 0,\
	.utf8 = -1,\
	.crlf = -1\
}

/** init regex
 * @param rx
 */
void regex_init(regex_s* rx);

/** set regular expression*/
void regex_set(regex_s* rx, const char* match);

/** set text to match*/
void regex_text(regex_s* rx, const char* txt, size_t len);

/** delete match result*/
void regex_match_delete(regex_s* rx);

/** delete regex, autocall regex_match_delete*/
void regex_free(regex_s* rx);

/** build a regex setted by regex_set, if error store in ef/err.h*/
int regex_build(regex_s* rx);

/** match regex, return 0 on match, -1 if no match or error*/
int regex_match(regex_s* rx);

/** continue match, return 0 on match -1 if no match or error*/
int regex_match_continue(regex_s* rx);

/** number of capture*/
size_t regex_match_count(regex_s* rx);

/** get capture
 * @param lenout out value, pointer to size of capture, NULL if you not want a len
 * @param rx regex obj
 * @param index index of capture < regex_match_count
 * @return capture
 */
const char* regex_match_get(size_t* lenout, regex_s* rx, size_t index);

/** execute regex on str and return capture in vector
 * vector contain copy of capture, need to free each element before delete vector
 * @param str string
 * @param regex regex
 * @param global continue after match
 * @return vector for captured string, free each element before free vector, NULL error or not match
 */
char** str_regex(const char* str, const char* regex, int global);


#endif
