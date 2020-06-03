#include <ef/ostr.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <stdarg.h>
#include <ctype.h>

char* ostr_new(const size_t size){
	ostr_s* s = mem_flexible_structure_new(ostr_s, sizeof(char), size);
	if( !s ) err_fail("eom");
	s->size = size;
	s->len = 0;
	*s->str = 0;
	return s->str;
}

void ostr_free(char* str){
	ostr_s* s = STRING(str);
	free(s);
}

void ostr_autofree(void* arg){
	void** pstr = (void**)arg;
	if( *pstr ) ostr_free(*pstr);
}

void ostr_clear(char* str){
	ostr_s* s = STRING(str);
	s->len = 0;
	*str = 0;
}

char* ostr_new_cstr(const char* str){
	size_t len = strlen(str);
	char* os = ostr_new(len+1);
	ostr_nputs(&os, str, len);
	return os;
}

char* ostr_resize(char* str, size_t size){
	ostr_s* s = STRING(str);
	if( s->size == size ) return str;

	s = mem_flexible_structure_realloc(s, ostr_s, 1, size); 
	if( s == NULL ) err_fail("realloc");

	s->size = size;
	if( s->len > s->size ){
		s->str[s->size-1] = 0;
		s->len = s->size;
	}

	return s->str;
}

void ostr_upsize(char** ptrs, size_t count){
	ostr_s* s = STRING(*ptrs);

	if( count + s->len > s->size - 1 ){
		size_t nwsz = s->size * 2;
		size_t rsz = ROUND_UP(s->size + count, nwsz);
		char* t = ostr_resize(*ptrs, rsz);
		*ptrs = t;
	}
}

void ostr_downsize(char** ptrs){
	ostr_s* s = STRING(*ptrs);

	if( s->len  / 4 > s->size ){
		size_t rsz = s->size / 2;
		char* t = ostr_resize(*ptrs, rsz);
		*ptrs = t;
	}
}

void ostr_putch(char** ptrs, char ch){
	ostr_upsize(ptrs, 1);
	ostr_s* s = STRING(*ptrs);
	s->str[s->len++] = ch;
	s->str[s->len] = 0;
}

void ostr_nputs(char** ptrs, const char* cstr, size_t len){
	if( len == 0 ) return;
	ostr_upsize(ptrs, len);
	ostr_s* s = STRING(*ptrs);
	memcpy(&s->str[s->len], cstr, len);
	s->len += len;
	s->str[s->len] = 0;
}

void ostr_puts(char** ptrs, const char* cstr){
	size_t len = strlen(cstr);
	ostr_nputs(ptrs, cstr, len);
}

//0123
//1234

void ostr_expand(char** ptrs, size_t index, size_t count){
	ostr_upsize(ptrs, count);
	ostr_s* s = STRING(*ptrs);
	memmove(&s->str[index+count], &s->str[index], s->len - index);
	s->len += count;
	s->str[s->len] = 0;
}

void ostr_insch(char** ptrs, size_t index, char ch){
	ostr_expand(ptrs, index, 1);
	ostr_s* s = STRING(*ptrs);
	s->str[index] = ch;
}

void ostr_insncstr(char** ptrs, size_t index, const char* str, size_t len){
	ostr_expand(ptrs, index, len);
	ostr_s* s = STRING(*ptrs);
	memcpy(&s->str[index], str, len);
}

void ostr_inscstr(char** ptrs, size_t index, const char* str){
	ostr_insncstr(ptrs, index, str, strlen(str));
}

void ostr_vprintf(char** ptrs, const char* format, va_list va1, va_list va2){
	size_t len = vsnprintf(NULL, 0, format, va1);
	if( len == 0 ) return;
	ostr_upsize(ptrs, len);
	ostr_s* s = STRING(*ptrs);
	vsprintf(&s->str[s->len], format, va2);
	s->len += len;
}

void ostr_printf(char** ptrs, const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	ostr_vprintf(ptrs, format, va1, va2);
	va_end(va1);
	va_end(va2);
}

void ostr_unch(char** ptrs){
	ostr_s* s = STRING(*ptrs);
	if( !s->len ) return;
	s->str[--s->len] = 0;
	ostr_downsize(ptrs);
}

void ostr_ndel(char** ptrs, size_t index, size_t count){
	ostr_s* s = STRING(*ptrs);
	if( !s->len || index >= s->len ) return;
	if( index + count >= s->len ){
		s->str[index] = 0;
		s->len = index;
	}
	else{
		memmove(&s->str[index], &s->str[index+count], s->len - (index+count));
	}
	s->len -= count;
	s->str[s->len] = 0;
	ostr_downsize(ptrs);
}

void ostr_del(char** ptrs, size_t index){
	ostr_ndel(ptrs, index, 1);
}

void ostr_toupper(char* os){
	while( (*os=toupper(*os)) ) ++os;
}

void ostr_tolower(char* os){
	while( (*os=tolower(*os)) ) ++os;
}





