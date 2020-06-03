#ifndef __EF_OBJECTSTRING_H__
#define __EF_OBJECTSTRING_H__

#include <ef/type.h>

typedef struct ostr{
	size_t size;  /**< size of allocated mem*/
	size_t len;   /**< used object*/
	char   str[0]; /**< pointer to mem*/
}ostr_s;

#define STRING(S) ((ostr_s*)(ADDR(S)-sizeof(ostr_s)) )

#define ostr_len(S) (STRING(S)->len)

char* ostr_new(const size_t size);
void ostr_free(char* str);
void ostr_autofree(void* arg);
#define __ostr_free __cleanup(ostr_autofree)
void ostr_clear(char* str);
char* ostr_new_cstr(const char* str);
char* ostr_resize(char* str, size_t size);
void ostr_upsize(char** ptrs, size_t count);
void ostr_downsize(char** ptrs);
void ostr_putch(char** ptrs, char ch);
void ostr_nputs(char** ptrs, const char* cstr, size_t len);
void ostr_puts(char** ptrs, const char* cstr);
void ostr_expand(char** ptrs, size_t index, size_t count);
void ostr_insch(char** ptrs, size_t index, char ch);
void ostr_insncstr(char** ptrs, size_t index, const char* str, size_t len);
void ostr_inscstr(char** ptrs, size_t index, const char* str);
void ostr_vprintf(char** ptrs, const char* format, va_list va1, va_list va2);
__printf(2,3) void ostr_printf(char** ptrs, const char* format, ...);
void ostr_unch(char** ptrs);
void ostr_ndel(char** ptrs, size_t index, size_t count);
void ostr_del(char** ptrs, size_t index);
void ostr_toupper(char* os);
void str_tolower(char* dst, const char* src);

#endif
