#include <ef/type.h>
#include <ef/memory.h>
#include <ef/file.h>
#include <ef/str.h>
#include <ef/delay.h>
#include <ef/err.h>
#include <ef/terminfo.h>

#include <string.h>
#include <stdarg.h>

#include "titostring.h"

#define TI_OFFSET_STACK 2048
#define TI_HASH_SIZE    412
#define TI_HASH_MIN     10
#define TI_HASH_KEY     128
#define TI_TMP_NAME     1024
#define TERM_STACK_MAX  128
#define TERM_IF_MAX     16

termInfo_s localTermInfo;

const char* term_name(void){
	return getenv(ENV_TERM);
}

const char* term_name_extend(void){
	return getenv(ENV_TERMEX);
}

const char* term_name_ef(void){
	return getenv(ENV_TERMEF);
}

__private FILE* term_database_open(const char* path, const char* dbname){
	char fname[PATH_MAX];
	if( path == NULL ){
		strcpy(fname, TERM_DATABASE_DIRECTORY);
	}
	else{
		strcpy(fname, path);
	}

	if( dbname ){
		size_t nf = strlen(fname);
		size_t dbnlen = strlen(dbname);
		if( nf + 4 + dbnlen > PATH_MAX ){
			dbg_error("path to long");
			return NULL;
		}
	
		fname[nf++] = '/';
		fname[nf++] = *dbname;
		fname[nf++] = '/';
		memcpy(&fname[nf], dbname, dbnlen+1);
	}
	else{
		strcpy(fname, path);
	}
	
	dbg_info("term file: '%s'", fname);
	FILE* ret = fopen( fname, "r");
	if( !ret ){
		err_pushno("open '%s'", fname);
	}
	return ret;
}

#define term_database_read16(PTR,FD) do{\
   	if( fread(PTR, sizeof(int16_t), 1, FD) != 1 ){\
		dbg_error("fail to read %s", #PTR);\
		dbg_errno();\
		return -1;\
	}\
}while(0)

#define term_database_read8(PTR,FD) do{\
   	if( fread(PTR, sizeof(int8_t), 1, FD) != 1 ){\
		dbg_error("fail to read %s", #PTR);\
		dbg_errno();\
		return -1;\
	}\
}while(0)

#define term_database_readstr(PTR,LEN,FD) do{\
   	if( fread(PTR, sizeof(char), LEN, FD) != (size_t)(LEN) ){\
		dbg_error("fail to read %s", #PTR);\
		dbg_errno();\
		return -1;\
	}\
}while(0)

#define term_database_readv16(PTR,COUNT,FD) do{\
   	if( fread(PTR, sizeof(int16_t), COUNT, FD) != (COUNT) ){\
		dbg_error("fail to read %s", #PTR);\
		dbg_errno();\
		return -1;\
	}\
}while(0)

#define term_database_readvch(OSIZE,PTR,FD) do{\
	int ch;\
	OSIZE=0;\
	while( (ch=fgetc(FD)) != '\0' && ch != EOF ){\
		PTR[OSIZE++]=ch;\
	}\
	PTR[OSIZE] = 0;\
   	if( OSIZE == 0 ){\
		dbg_error("fail to read %s", #PTR);\
		dbg_errno();\
	}\
}while(0)

__private char* term_escape_escape(int ch){
	switch( ch ){
		case 0x1B: return "\\E";
		case '\a': return "\\A";
		case '\b': return "\\B";
		case '\f': return "\\F";
		case '\n': return "\\N";
		case '\r': return "\\R";
		case '\t': return "\\T";
		case '\v': return "\\V";
		default: return NULL;
	}
}

char* term_escape_character(int ch){
	static char esc[12];
	esc[0] = ch;
	esc[1] = 0;
	char* sr = term_escape_escape(ch);
	if( sr ) return sr;
	return esc;
}

char* term_escape_str(char* out, const char* ch){
	char* begin = out;
	while( *ch ){
		char* esc = term_escape_escape(*ch);
		if( esc ){ 
			strcpy(out, esc);
			out+=strlen(esc);
		}
		else{
			*out++=*ch;
		}
		++ch;
	}
	*out = 0;
	return begin;
}

__private size_t term_escape_askey(char* out, const char* escape){
	static unsigned keyset[128] = {
		['%'] = 1, ['l'] = 1, ['+'] = 1, ['-'] = 1, ['*'] = 1, ['/'] = 1, ['m'] = 1, ['&'] = 1, ['|'] = 1, ['^'] = 1, 
		['='] = 1, ['<'] = 1, ['>'] = 1, ['!'] = 1, ['~'] = 1, ['i'] = 1, ['?'] = 1, ['t'] = 1, ['e'] = 1, [';'] = 1,
		['c'] = 2, ['d'] = 2, ['o'] = 2, ['x'] = 2,	['X'] = 2, ['s'] = 2,
		['p'] = 3, ['P'] = 3, ['g'] = 3,
		['\''] = 4, ['{'] = 4,
		[':'] = 5, ['.'] = 5,
		['0'] = 5, ['1'] = 5, ['2'] = 5, ['3'] = 5, ['4'] = 5, ['5'] = 5, ['6'] = 5, ['7'] = 5, ['8'] = 5, ['9'] = 5
	};

	iassert(escape);

	char* begin = out;
	while( *escape ){
		if( *escape == '%' ){
			++escape;
			iassert((unsigned)*escape < 128);
			switch( keyset[(unsigned)*escape] ){
				default: case 0: dbg_warning("unknow %% simbol %c",*escape); break;
				case 1:	break;
				
UNSAFE_BEGIN("-Wimplicit-fallthrough");
				case 5:
					while(*escape && *escape != 'd' && *escape != 'o' && *escape != 'x' && *escape != 'X' && *escape != 's')
						++escape;
					iassert( *escape != 0 );
				case 2:
					*out++='%';
				break;
UNSAFE_END;

				case 3: ++escape; break;
				
				case 4:{
					char ch = *escape == '{' ? '}' : *escape;
					++escape;
					while(*escape && *escape != ch ) ++escape;
					iassert( *escape != 0 );
				}
				break;
			}
			++escape;
		}
		else if ( *escape == '$' ){
			while( *escape && *escape != '>' ) escape++;
			if( *escape ) ++escape;
		}
		else{
			*out++ = *escape++;
		}
	}
	*out = 0;

	return out-begin;
}

__private err_t term_database_headers(tidatabase_s* db, FILE* fd){
	term_database_read16(&db->header.magic, fd);
	if( db->header.magic != TERM_MAGIC ){
		dbg_error("magic");
		err_push("term info magic");
		return -1;
	}
	term_database_read16(&db->header.nameSize, fd);
	term_database_read16(&db->header.booleanSize, fd);
	term_database_read16(&db->header.numberSize, fd);
	term_database_read16(&db->header.offsetStringCount, fd);
	term_database_read16(&db->header.stringTableSize, fd);
	
	dbg_info("name size = %d",db->header.nameSize);
	dbg_info("boolean size = %d",db->header.booleanSize);
	dbg_info("numebers size = %d",db->header.numberSize);
	dbg_info("offset size = %d",db->header.offsetStringCount);
	dbg_info("table size = %d",db->header.stringTableSize);

	return 0;
}

__private err_t term_database_extend(tidatabase_s* db, FILE* fd, FILE* fn){
	term_database_read16(&db->extend.booleanSize, fd);
	term_database_read16(&db->extend.numberSize, fd);
	term_database_read16(&db->extend.stringCount, fd);
	term_database_read16(&db->extend.stringTableSize, fd);
	term_database_read16(&db->extend.lastoffset, fd);
	
	dbg_info("ex boolean size = %d",db->extend.booleanSize);
	dbg_info("ex numebers size = %d",db->extend.numberSize);
	dbg_info("ex str count = %d",db->extend.stringCount);
	dbg_info("ex str table size = %d",db->extend.stringTableSize);
	dbg_info("ex str last offset  = %d",db->extend.lastoffset);

	size_t pos = ftell(fd);
	dbg_info("current position %lu", pos);
	pos += db->extend.booleanSize + (db->extend.booleanSize & 1);
	pos += db->extend.numberSize * sizeof(int16_t);
	pos += db->extend.stringTableSize * sizeof(int16_t);

	if( fseek(fn, pos, SEEK_SET) ){
		err_pushno("can't seek to name");
		dbg_error("can't seek to name");
		dbg_errno();
		return -1;
	}

	dbg_info("position list name %lu", pos);
	dbg_info("skip caps");
	__unused char tmp[TI_TMP_NAME];
	size_t sz;
	for( size_t i = 0; i < (size_t)db->extend.stringCount; ++i){
		term_database_readvch(sz,tmp,fn);
	}

	return 0;
}

__private err_t term_database_name(termInfo_s* ti, tidatabase_s* db, FILE* fd){
	ti->dbname = mem_many(char, db->header.nameSize+1);
	iassert(ti->dbname);
	term_database_readstr(ti->dbname, db->header.nameSize,fd);
	ti->dbname[db->header.nameSize] = 0;
	dbg_info("database name:: %s", ti->dbname);
	return 0;
}

__private err_t term_database_boolean(termInfo_s* ti, tidatabase_s* db, FILE* fd){
	size_t stdcount = db->header.booleanSize;
	if( stdcount > lenght_stack_vector(termBoolToStr) ){
		dbg_error("malformed database");
		err_push("malformed database");
		return -1;
	}

	for( size_t i = 0; i < stdcount; ++i ){
		int8_t dt;
		term_database_read8(&dt, fd);
		if( dt == -1 ){
			dbg_info("bool %s not used", termBoolToStr[i]);
			continue;
		}
		tiData_s* tid = mem_new(tiData_s);
		tid->type = TI_TYPE_BOOL;
		tid->bol = dt;
		
		rbhash_add(ti->cap, termBoolToStr[i], strlen(termBoolToStr[i]), tid);
		dbg_info("bool %s = %d", termBoolToStr[i], dt);
	}
	
	if( stdcount & 1 ){
		int8_t dt;
		term_database_read8(&dt, fd);
	}

	return 0;
}

__private err_t term_database_numbers(termInfo_s* ti, tidatabase_s* db, FILE* fd){
	size_t stdcount = db->header.numberSize;
	if( stdcount > lenght_stack_vector(termNumToStr) ){
		dbg_error("malformed database");
		err_push("malformed database");
		return -1;
	}

	for( size_t i = 0; i < stdcount; ++i ){
		int16_t dt;
		term_database_read16(&dt, fd);
		if( dt == -1 ){
			dbg_info("num %s not used", termNumToStr[i]);
			continue;
		}
		tiData_s* tid = mem_new(tiData_s);
		tid->type = TI_TYPE_NUM;
		tid->num = dt;
		rbhash_add(ti->cap, termNumToStr[i], strlen(termNumToStr[i]), tid);
		dbg_info("num %s = %d", termNumToStr[i], dt);
	}

	return 0;
}

__private err_t term_database_strings(termInfo_s* ti, tidatabase_s* db, FILE* fd){
	int16_t offset[TI_OFFSET_STACK];
	size_t stdcount = db->header.offsetStringCount;
	size_t ssize = db->header.stringTableSize;

	if( stdcount == 0 ) return 0;
	if( stdcount >= TI_OFFSET_STACK ){
		dbg_fail("stdcount > TI_OFFSET_STACK");
	}

	term_database_readv16(offset, stdcount, fd);

	if( stdcount > lenght_stack_vector(termStrToStr) ){
		dbg_error("malformed database");
		err_push("malformed database");
		return -1;
	}

	size_t beginstr = ftell(fd);
	for( size_t i = 0; i < stdcount; ++i ){
		if( offset[i] == -1 ){
			dbg_info("str %s not used", termStrToStr[i]);
			continue;
		}

		char tmp[TI_TMP_NAME];
		size_t len;
		if( -1 == fseek(fd, beginstr + offset[i], SEEK_SET) ){
			err_pushno("out of seek");
			dbg_error("out of seek");
			dbg_errno();
			return -1;
		}
		term_database_readvch(len, tmp, fd);
		if( len == 0 ){
			dbg_info("str %s not implement", termStrToStr[i]);
			continue;
		}
		dbg_info("prepare: %s", termStrToStr[i]);
		tiData_s* tid = mem_new(tiData_s);
		iassert(tid);
		tid->type = TI_TYPE_STR;
		tid->str = str_dup(tmp, len);
		rbhash_add(ti->cap, termStrToStr[i], strlen(termStrToStr[i]), tid);
		
		len = term_escape_askey(tmp, tid->str);
		
		//trie_insert(&ti->pac, tmp, termStrToStr[i]);

		#if DEBUG_ENABLE 
			char tmpdbg[2048];
		#endif
		dbg_info("str %s = %s", termStrToStr[i], term_escape_str(tmpdbg,tid->str));
		ssize -= len;
		iassert(ssize < (size_t)db->header.stringTableSize);
	}

	if( -1 == fseek(fd, beginstr + db->header.stringTableSize, SEEK_SET) ){
		err_pushno("to end string");
		dbg_error("to end string");
		dbg_errno();
		return -1;
	}

	return 0;
}

__private err_t term_database_boolean_ex(termInfo_s* ti, tidatabase_s* db, FILE* fd, FILE* fn){
	size_t stdcount = db->extend.booleanSize;

	for( size_t i = 0; i < stdcount; ++i ){
		int8_t dt;
		char name[TI_TMP_NAME];
		size_t lename;

		term_database_read8(&dt, fd);
		if( dt == -1 ){
			dbg_info("bool not used");
			continue;
		}

		term_database_readvch(lename, name, fn);
		if( lename == 0 ) return -1;
		tiData_s* tid = mem_new(tiData_s);
		tid->type = TI_TYPE_BOOL;
		tid->bol = dt;
		
		if( rbhash_add_unique(ti->cap, name, lename, tid) ){
			dbg_warning("not unique %s", name);
		}
		else{
			dbg_info("bool %s = %d", name, dt);
		}
	}
	
	if( stdcount & 1 ){
		int8_t dt;
		term_database_read8(&dt, fd);
	}

	return 0;
}

__private err_t term_database_numbers_ex(termInfo_s* ti, tidatabase_s* db, FILE* fd, FILE* fn){
	size_t stdcount = db->extend.numberSize;

	for( size_t i = 0; i < stdcount; ++i ){
		int8_t dt;
		char name[TI_TMP_NAME];
		size_t lename;

		term_database_read8(&dt, fd);
		if( dt == -1 ){
			dbg_info("num not used");
			continue;
		}

		term_database_readvch(lename, name, fn);
		if( lename == 0 ) return -1;
		tiData_s* tid = mem_new(tiData_s);
		tid->type = TI_TYPE_NUM;
		tid->num = dt;
		
		if( rbhash_add_unique(ti->cap, name, lename, tid) ){
			dbg_warning("not unique %s", name);
		}
		else{
			dbg_info("num %s = %d", name, dt);
		}
	}

	return 0;
}

__private err_t term_database_strings_ex(termInfo_s* ti, tidatabase_s* db, FILE* fd, FILE* fn){
	int16_t offset[TI_OFFSET_STACK];
	size_t stdcount = db->extend.stringTableSize;

	if( db->extend.stringCount == 0 ) return 0;
	term_database_readv16(offset, stdcount, fd);

	//size_t beginstr = ftell(fd);
	//dbg_info("begin pos %lu", beginstr);

	for( size_t i = 0; i < (size_t)db->extend.stringCount; ++i ){
		char tmp[TI_TMP_NAME];
		size_t len;
		term_database_readvch(len, tmp, fd);
		if( len == 0 ) return -1;
		char name[TI_TMP_NAME];
		size_t lename;
		term_database_readvch(lename, name, fn);

		tiData_s* tid = mem_new(tiData_s);
		tid->type = TI_TYPE_STR;
		tid->str = str_dup(tmp,len);
		char* keyname = str_dup(name, lename);
		if( rbhash_add_unique(ti->cap, keyname, lename, tid) ){
			dbg_warning("cap not unique %s", name);
		}
		else{
			#if DEBUG_ENABLE 
				char tmpdbg[2048];
			#endif
			dbg_info("str %s = %s", name, term_escape_str(tmpdbg,tid->str));
		}

		len = term_escape_askey(tmp, tid->str);
		//trie_insert(&ti->pac, tmp, keyname);
	}

	return 0;
}

__private void term_tid_free(__unused uint32_t hash, __unused const char* name, tiData_s* dt){
	if( dt->type == TI_TYPE_STR ){
		free(dt->str);
	}
	free(dt);
}

void term_begin(void){
	localTermInfo.cap = rbhash_new(TI_HASH_SIZE, TI_HASH_MIN, TI_HASH_KEY, hash_fasthash, (rbhashfree_f)term_tid_free);
	iassert(localTermInfo.cap);
	localTermInfo.dbname = NULL;
}

void term_end(void){
	if( localTermInfo.cap ) rbhash_free(localTermInfo.cap);
	if( localTermInfo.dbname ) free(localTermInfo.dbname);
	localTermInfo.cap = NULL;
	localTermInfo.dbname = NULL;
}

err_t term_load(char* path, const char* dbname){
	__file_close FILE* fd = term_database_open(path, dbname);
	if( fd == NULL ) return -1;
	
	tidatabase_s db;
	if( term_database_headers(&db, fd) ) return -1;
	if( term_database_name(&localTermInfo, &db, fd) ) return -1;
	if( term_database_boolean(&localTermInfo, &db, fd) ) return -1;
	if( term_database_numbers(&localTermInfo, &db, fd) ) return -1;
	if( term_database_strings(&localTermInfo, &db, fd) ) return -1;

	__file_close FILE* sfd = term_database_open(path, dbname);
	if( sfd == NULL ){
		err_pushno("dup open %s %s", path, dbname);
		dbg_error("file_dup");
		dbg_errno();
		return -1;
	}

	if( term_database_extend(&db, fd, sfd) ) return 0;
	if( term_database_boolean_ex(&localTermInfo, &db, fd, sfd) ) return -1;
	if( term_database_numbers_ex(&localTermInfo, &db, fd, sfd) ) return -1;
	if( term_database_strings_ex(&localTermInfo, &db, fd, sfd) ) return -1;

	return 0;
}

typedef struct tstack{
	size_t size;
	size_t id;
	tvariable_s stk[TERM_STACK_MAX];
}tstack_s;
	
__private inline void tstack_push_long(tstack_s* stack, long val){
	iassert( stack->id < stack->size );
	stack->stk[stack->id++].l = val;
}

__private inline void tstack_push_string(tstack_s* stack, char* val){
	iassert( stack->id < stack->size );
	stack->stk[stack->id++].s = val;
}

__private inline void tstack_push(tstack_s* stack, tvariable_s* var){
	iassert( stack->id < stack->size );
	stack->stk[stack->id].type = var->type;
	switch( var->type ){
		case 0: stack->stk[stack->id++].l = var->l; break;
		case 1: stack->stk[stack->id++].s = var->s; break;
		default: dbg_fail("push unknow stack param type"); break;
	}
}

__private inline long tstack_pop_long(tstack_s* stack){
	iassert( stack->id >0 );
	return stack->stk[--stack->id].l;
}

__private inline char* tstack_pop_string(tstack_s* stack){
	iassert( stack->id >0 );
	return stack->stk[--stack->id].s;
}

__private inline tvariable_s* tstack_pop(tstack_s* stack){
	iassert( stack->id >0 );
	return &stack->stk[--stack->id];
}

__private const char* te_get_format(const char** format){
	__private char buf[80] = "%";
	char* b=&buf[1];
	const char* f = *format;

	while( *f && *f != 'd' && *f != 'o' && *f != 'x' && *f != 'X' && *f != 's' ){
		*b++=*f++;
	}
	iassert( *f != 0 );
	*b++ = *f;
	*b = 0;
	*format = f;
	return buf;
}	

__private const char* escape_to_end(const char* format){
	do{
		format = strchr(format, '%');
		iassert( format != NULL );
		++format;
		iassert( *format != 0 );
		if( *format == '?' ){
			format = escape_to_end(format+1);
			format += 2;
			continue;
		}
	}while( *format != ';' );
	return format - 1;
}

__private const char* escape_to_else(const char* format){
	do{
		format = strchr(format, '%');
		iassert( format != NULL );
		++format;
		iassert( *format != 0 );
		if( *format == '?' ){
			format = escape_to_end(format+1);
			format += 2;
			continue;
		}
	}while( *format != 'e' && *format != ';' );
	return format - 1;
}
/*
__private const char* escape_delay(const char* format){
	if( *format != '<' ){
		dbg_error("delay format error");
		return format;
	}
	++format;
	char* en;
	size_t ms = strtol(format, &en, 10);
	if( *en != '>' ){
		dbg_error("delay format closed error");
		return en;
	}
	delay_ms(ms);
	return en+1;
}
*/

char* term_escape_make(char* out, const char* format, tvariable_s* param){
	__private tvariable_s var['z'-'a'];

	tstack_s stack;
	stack.id = 0;
	stack.size = TERM_STACK_MAX;
	const char* memformat = NULL;
	int statif[TERM_IF_MAX] = {0};
	size_t curif = 0;
	char* begin = out;

	while( *format ){
		while( *format && *format != '%' ){
			*out++=*format++;
		}
		if( 0 == *format ) break;
		
		//if( '$' == *format ){
		//	format = escape_delay(format+1);
		//	continue;
		//}
		
		++format;
		int next;
		do{
			next = 0;
			switch( *format ){
				default:
					dbg_warning("unknow %% simbol %c",*format);
					++format;
				break;

				case 0:	dbg_warning("end of format with no symbol"); break;
	
				case '%':
					*out++='%';
					++format;
				break;
				
				UNSAFE_BEGIN("-Wimplicit-fallthrough");				
				case ':':
					++format;
				case '0':
				case '.':
				case '1'...'9':
					memformat = te_get_format(&format);
					next = 1;
				break;
				UNSAFE_END;

				case 'x': case 'c':	case 'X': case 'o':	case 'd':{
					int val = tstack_pop_long(&stack);
					if( memformat != NULL ){
						out += sprintf(out, memformat, val);
					}
					else{
						char ff[3];
						ff[0] = '%';
						ff[1] = *format;
						ff[2] = 0;
						out += sprintf(out, ff,val);
					}
					memformat = NULL;
					++format;
				}
				break;
	
				case 's':{
					char* val = tstack_pop_string(&stack);
					if( memformat != NULL ){
						out += sprintf(out, memformat, val);
					}
					else{
						out += sprintf(out, "%s",val);
					}
					memformat = NULL;
					++format;
				}
				break;
				
				case 'p':{
					++format;
					iassert(*format > '0' && *format <= '9');
					size_t id = *format - '0';
					switch( param[id].type ){
						case 0: tstack_push_long(&stack, param[id].l); break;
						case 1: tstack_push_string(&stack, param[id].s); break;
						default: dbg_fail("p unknow stack param type"); break;
					}
					++format;
				}
				break;

				case 'P':{
					++format;
					iassert(*format >= 'a' && *format <= 'z');
					size_t id = *format - 'a';
					tvariable_s* p = tstack_pop(&stack);
					var[id].type = p->type;
					switch( p->type ){
						case 0: var[id].l = p->l; break;
						case 1: var[id].s = p->s; break;
						default: dbg_fail("P unknow stack type"); break;
					}
					++format;
				}
				break;

				case 'g':{
					++format;
					iassert(*format >= 'a' && *format <= 'z');
					size_t id = *format - 'a';
					tstack_push(&stack, &var[id]);
					++format;
				}
				break;

				case '\'':
					++format;
					tstack_push_long(&stack, *format++);
					iassert( *format == '\'');
					++format;
				break;

				case '{':{
					char* endl = NULL;
					++format;
					long n = strtol(format,&endl,10);
					iassert( endl != NULL );
					iassert( *endl == '}' );
					tstack_push_long(&stack, n);
					format = endl+1;
				}
				break;

				case 'l':{
					char* str = tstack_pop_string(&stack); 
					iassert( str != NULL );
					long n = strlen(str);
					iassert( n != -1 );
					tstack_push_long(&stack, n);
					++format;
				}
				break;

				case '+':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a + b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '-':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a - b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '/':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a / b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '*':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a * b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case 'm':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a % b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '&':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a & b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '|':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a | b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '^':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a ^ b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '!':{
					long a = tstack_pop_long(&stack); 
					long r = !a;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '~':{
					long a = tstack_pop_long(&stack); 
					long r = ~a;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '=':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a == b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '<':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					long r = a < b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case '>':{
					long a = tstack_pop_long(&stack); 
					long b = tstack_pop_long(&stack); 
					dbg_info("compare %ld > %ld",a,b);
					long r = a > b;
				    tstack_push_long(&stack, r);
					++format;
				}
				break;

				case 'i':{
					iassert( param[1].type == 0 );
					iassert( param[2].type == 0 );
					++param[1].l;
					++param[2].l;
					++format;
				}
				break;

				case '?':
					statif[curif]=1;
					++curif;
					dbg_info("start if %lu", curif);
					++format;
				break;

				case 't':{
					long condition = tstack_pop_long(&stack); 
					if( 0 == condition ){
						format = escape_to_else(format);
						iassert(curif > 0);
						statif[curif-1]=2;
					}else{
						++format;
						dbg_info("then");
					}
				}
				break;

				case 'e':
					iassert(curif > 0);
					if( statif[curif-1] != 2 ){
						format = escape_to_end(format);
					}
					else{
						dbg_info("else");
						++format;
					}
				break;

				case ';':
					dbg_info("endif %lu", curif);
					iassert(curif > 0);
					--curif;
					++format;
				break;
			}//switch( *format )
		}while( next );
	}//while( *format )
	*out = 0;
	return begin;
}	

err_t term_escape_string(char* out, char* name, tvariable_s* var){
	tiData_s* dt = rbhash_find(localTermInfo.cap, name, strlen(name));
	if( !dt ){
		dbg_warning("cap %s not exists", name);
		return -1;
	}
	if( dt->type != TI_TYPE_STR ){
		dbg_warning("cap %s isn't str", name);
		dbg_error("cap %s is not a string", name);
		return -1;
	}
	term_escape_make(out, dt->str, var);
	return 0;
}

void term_escape_print(char* name, tvariable_s* var){
	char mk[256];
	if( term_escape_string(mk, name, var) ) return;
	term_print(mk);
}

tiData_s* term_info(char* name){
	static tiData_s err = {.type = TI_TYPE_UNSET, .str = NULL};
	tiData_s* ret = rbhash_find(localTermInfo.cap, name, strlen(name));
	return ret ? ret : &err;
}

int term_info_number(char* name){
	return term_info(name)->num;
}

int term_info_bool(char* name){
	return term_info(name)->bol;
}

const char* term_info_string(char* name){
	return term_info(name)->str;
}

/*
const char* term_name_from_escape(termInfo_s* ti, char* escape){
	const char* name;
	if( !(name=trie_search(&ti->pac, escape)) ){
		dbg_warning("pac not exists");
		return NULL;
	}
	return name;
}*/
