#ifndef __EF_TERM_INFO_H__
#define __EF_TERM_INFO_H__

#include <ef/type.h>
#include <ef/rbhash.h>

#define TERM_MAGIC 282

/** directory to find terminfo */
#ifndef TERM_DATABASE_DIRECTORY 
	#define TERM_DATABASE_DIRECTORY "/usr/lib/terminfo"
#endif

/** enviroment contains database name */
#ifndef ENV_TERM
	#define ENV_TERM   "TERM"
#endif

/** extra enviroment contains unofficial cap for terminal */
#ifndef ENV_TERMEX
	#define ENV_TERMEX "TERMEX"
#endif

/** extra enviroment contains unofficial cap for common terminal */
#ifndef ENV_TERMEF
	#define ENV_TERMEF "TERMEF"
#endif

#define TERM_EF_EXTEND "ef-term-extend"

typedef enum { TI_TYPE_UNSET, TI_TYPE_BOOL, TI_TYPE_NUM, TI_TYPE_STR } titype_e;

typedef struct tiData {
	titype_e type;
	union {
		char* str;
		int num;
		bool_t bol;
	};
} tiData_s;

typedef struct termInfo {
	rbhash_s* cap;
	char* dbname;
}termInfo_s;

typedef struct tidbheader{
	int16_t magic;
	int16_t nameSize;
	int16_t booleanSize;
	int16_t numberSize;
	int16_t offsetStringCount;
	int16_t stringTableSize;
}tidbheader_s;

typedef struct tidbextend{
	int16_t booleanSize;
	int16_t numberSize;
	int16_t stringCount;
	int16_t stringTableSize;
	int16_t lastoffset;
}tidbextend_s;

typedef struct tidatabase{
	tidbheader_s header;
	tidbextend_s extend;
	size_t size;
}tidatabase_s;

typedef struct tvariable{
	union{
		long l;
		char* s;
	};
	int type;
}tvariable_s;

/** return terminal name */
const char* term_name(void);

/** return terminal extend name */
const char* term_name_extend(void);

/** return terminal common extend name if not eviroment is setted return TERM_EF_EXTEND*/
const char* term_name_ef(void);

/** convert unescaped char in printable, escaped, string
 * @param ch non printable char
 * @return escaped string rappresent ch
 */
char* term_escape_character(int ch);

/** convert unescaped string to printable escaped string
 * @param out buffer where stored string
 * @param ch string to escape
 * @return out
 */
char* term_escape_str(char* out, const char* ch);

/** init term, call this before use other function*/
void term_begin(void);

/** release resources of term */
void term_end(void);

/** load terminal cap, add to previus database if loaded
 * @param path null used TERM_DATABASE_DIRECTORY
 * @param dbname name database to load, \<path\>/\<dbname[0]\>/\<dbname\>, null use path without change for loading database
 * @return -1 error, 0 successfull
 */
err_t term_load(char* path, const char* dbname);

/** print */
#define term_print(STR) fputs(STR,stdout)

/** flush */
#define term_flush() fflush(stdout)

/** get a format cap, variable and build it on string
 * @param out builded escape
 * @param format cap
 * @param param variable for cap
 * @return out
 */
char* term_escape_make(char* out, const char* format, tvariable_s* param);

/** get a format cap, variable, build and print on stdout
 * @param format cap
 * @param param variable for cap
 * @return out
 */
void term_escape_make_print(const char* format, tvariable_s* param);

/** get a name of cap, variable and build it on string
 * @param out builded escape
 * @param name name cap
 * @param var variable for cap
 * @return 0 successfull -1 for error
 */
err_t term_escape_string(char* out, char* name, tvariable_s* var);

/** get a name of cap, variable, build it and print
 * @param name name cap
 * @param var variable for cap
  * @return 0 successfull -1 for error 
 */
err_t term_escape_print(char* name, tvariable_s* var);

/** return a boolean cap value -1 for error */
int term_escape_bool(char* name);

/** return a int cap value, -1 for error*/
int term_escape_number(char* name);

/** return a raw tiData for name*/
tiData_s* term_info(const char* name);

/** return cap bool */
int term_info_bool(const char* name);

/** return cap number */
int term_info_number(const char* name);

/** return cap string */
const char* term_info_string(const char* name);

__always_inline tvariable_s __tvariable_set_numeric(long num) { return (tvariable_s){.type = 0, .l = num }; }
__always_inline tvariable_s __tvariable_set_string(char* str) { return (tvariable_s){.type = 1, .s = str }; }

#define TVARIABLE_AUTOTYPE(V) _Generic(V,\
	int: __tvariable_set_numeric,\
	unsigned int: __tvariable_set_numeric,\
	long: __tvariable_set_numeric,\
	unsigned long: __tvariable_set_numeric,\
	unsigned char: __tvariable_set_numeric,\
	char*: __tvariable_set_string,\
	const char*: __tvariable_set_string\
)(V)

#define term_escape_f0(NAME) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = { .type = 0, .l = 0 }\
	   	}\
	)

#define term_escape_f1(NAME, P1) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1)\
	   	}\
	)

#define term_escape_f2(NAME, P1, P2) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2)\
	   	}\
	)

#define term_escape_f3(NAME, P1, P2, P3) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3)\
	   	}\
	)

#define term_escape_f4(NAME, P1, P2, P3, P4) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4)\
	   	}\
	)

#define term_escape_f5(NAME, P1, P2, P3, P4, P5) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5)\
	   	}\
	)

#define term_escape_f6(NAME, P1, P2, P3, P4, P5, P6) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6)\
	   	}\
	)

#define term_escape_f7(NAME, P1, P2, P3, P4, P5, P6, P7) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6),\
		[7] = TVARIABLE_AUTOTYPE(P7)\
	   	}\
	)


#define term_escape_f8(NAME, P1, P2, P3, P4, P5, P6, P7, P8) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6),\
		[7] = TVARIABLE_AUTOTYPE(P7),\
		[8] = TVARIABLE_AUTOTYPE(P8)\
	   	}\
	)

#define term_escape_f9(NAME, P1, P2, P3, P4, P5, P6, P7, P8, P9) term_escape_print(NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6),\
		[7] = TVARIABLE_AUTOTYPE(P7),\
		[8] = TVARIABLE_AUTOTYPE(P8),\
		[9] = TVARIABLE_AUTOTYPE(P9)\
	   	}\
	)

#ifndef __clang__
/** same term_escape_print but with stdarg for tvariable_s*/
	#define term_escapef(NAME, ...) __CONCAT_EXPAND__(term_escape_f,__VA_COUNT__(__VA_ARGS__))(NAME,##__VA_ARGS__)
#else
	#define term_escapef(NAME, ...)
#endif

#define term_escape_mk0(OUT, NAME) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = { .type = 0, .l = 0 }\
	   	}\
	)

#define term_escape_mk1(OUT, NAME, P1) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1)\
	   	}\
	)

#define term_escape_mk2(OUT, NAME, P1, P2) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2)\
	   	}\
	)

#define term_escape_mk3(OUT, NAME, P1, P2, P3) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3)\
	   	}\
	)

#define term_escape_mk4(OUT, NAME, P1, P2, P3, P4) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4)\
	   	}\
	)

#define term_escape_mk5(OUT, NAME, P1, P2, P3, P4, P5) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5)\
	   	}\
	)

#define term_escape_mk6(OUT, NAME, P1, P2, P3, P4, P5, P6) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6)\
	   	}\
	)

#define term_escape_mk7(OUT, NAME, P1, P2, P3, P4, P5, P6, P7) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6),\
		[7] = TVARIABLE_AUTOTYPE(P7)\
	   	}\
	)

#define term_escape_mk8(OUT, NAME, P1, P2, P3, P4, P5, P6, P7, P8) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6),\
		[7] = TVARIABLE_AUTOTYPE(P7),\
		[8] = TVARIABLE_AUTOTYPE(P8)\
	   	}\
	)

#define term_escape_mk9(OUT, NAME, P1, P2, P3, P4, P5, P6, P7, P8, P9) term_escape_string(OUT, NAME, (tvariable_s[10]){\
	   	[1] = TVARIABLE_AUTOTYPE(P1),\
		[2] = TVARIABLE_AUTOTYPE(P2),\
		[3] = TVARIABLE_AUTOTYPE(P3),\
		[4] = TVARIABLE_AUTOTYPE(P4),\
		[5] = TVARIABLE_AUTOTYPE(P5),\
		[6] = TVARIABLE_AUTOTYPE(P6),\
		[7] = TVARIABLE_AUTOTYPE(P7),\
		[8] = TVARIABLE_AUTOTYPE(P8),\
		[9] = TVARIABLE_AUTOTYPE(P9)\
	   	}\
	)

#ifndef __clang__
/** same term_escape_string but with stdarg for tvariable_s*/
	#define term_escapemk(OUT, NAME, ...) __CONCAT_EXPAND__(term_escape_mk,__VA_COUNT__(__VA_ARGS__))(OUT,NAME,##__VA_ARGS__)
#else
	#define term_escapemk(OUT, NAME, ...)
#endif

#endif 
