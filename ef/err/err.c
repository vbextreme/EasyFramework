#define ERR_DEC_OBJ _Y_
#include <ef/err.h>
#include <ef/mth.h>
#include <stdarg.h>

__thread eerr_s gerr;

void err_begin(void){
	gerr.size = ERR_REALLOC;
	gerr.descript = malloc(sizeof(char)*gerr.size);
	if( gerr.descript == 0 ){
		fputs("error on allocating memory for ef.err\n",stderr);
		exit(1);
	}
	gerr.len = 0;

	gerr.rsz = ERR_RESTORE_STACK_SIZE;
	gerr.restore = malloc(sizeof(size_t) * gerr.rsz);
	if( gerr.restore == NULL ){
		err_fail("cant allocating memory for restoring error");
	}
	gerr.rc = 0;
	gerr.flags = 0;
}

void err_end(void){
	free(gerr.descript);
	free(gerr.restore);
	gerr.descript = 0;
	gerr.restore = 0;
}

__printf(1,2) void err_pushf(const char* format, ...){
	if( gerr.descript == 0 || !(gerr.flags & ERR_ENABLED) ) return;
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	const size_t len = vsnprintf(NULL, 0, format, va1);
	if( len == 0 ) goto ONERR;
	if( len + 1 > gerr.size - gerr.len ){
		const size_t newsize = ROUND_UP(gerr.size + len + 1,ERR_REALLOC);
		char* newmem = realloc(gerr.descript, sizeof(char) * newsize);
		if( newmem == NULL ) goto ONERR;
		gerr.descript = newmem;
		gerr.size = newsize;
	}
	vsprintf(&gerr.descript[gerr.len], format, va2);
	gerr.len += len;
ONERR:
	va_end(va1);
	va_end(va2);
}

void err_store(void){
	if( gerr.rc >= gerr.rsz ){
		gerr.rsz += ERR_RESTORE_STACK_SIZE;
		gerr.restore = realloc(gerr.restore, sizeof(size_t) * gerr.rsz);
		if( gerr.restore == NULL ){
			err_fail("can't realloc restore error");
		}
	}
	gerr.restore[gerr.rc++] = gerr.flags;
}

void err_restore(void){
	if( gerr.rc == 0 ) return;
	gerr.flags = gerr.restore[--gerr.rc];
}

void err_disable(void){
	err_store();
	gerr.flags &= ~ERR_ENABLED;
}

void err_enable(void){
	err_store();
	gerr.flags |= ERR_ENABLED;
}

const char* err_descript(void){
	return gerr.descript;
}

void err_print(void){
	fputs(gerr.descript, stderr);
}

void err_clear(void){
	gerr.descript[0] = 0;
	gerr.len = 0;
}
