#include <ef/str.h>
#include <ef/memory.h>
#include <ef/mth.h>
#include <ef/err.h>
#include <ctype.h>
#include <stdarg.h>

char* str_dup(const char* src, size_t optlen){
	if( optlen == 0 ) optlen = strlen(src);
	char* ret = mem_many(char, optlen+1);
	if( ret == NULL ) return NULL;		
	memcpy(ret, src, optlen);
	ret[optlen] = 0;
	return ret;
}

char* str_dup_ch(const char* src, const char ch){
	const char* end = strchr(src, ch);
	if( !end ) return NULL;
	if( end == src ) return NULL;
	return str_dup(src, end-src);
}

int str_equal(char const* a, size_t lena, char const* b, size_t lenb){
	if( lena == 0 ) lena = strlen(a);
	if( lenb == 0 ) lenb = strlen(b);
	if( lena != lenb ) return lena - lenb;
	return memcmp(a,b,lena);
}

const char* str_skip_h(const char* str) {
	while( *str && (*str == ' ' || *str == '\t') ) ++str;
	return str;
}

const char* str_skip_hn(const char* str){
	while( *str && (*str == ' ' || *str == '\t' || *str == '\n') ) ++str;
	return str;
}

const char* str_next_line(const char* str){
	while( *str && *str != '\n' ) ++str;
	if( *str ) ++str;
	return str;
}

char* str_ncpy(char* restrict dst, size_t lend, const char* restrict src, size_t lens){
	iassert(lend > 1);
	--lend;
	if( lens == 0 ) lens = strlen(src);

	while( lens-->0 && lend-->0 && *src){
		*dst++=*src++;
	}
	*dst = 0;
	return dst;
}

char* str_cpy(char* dst, size_t lend, const char* src){
	iassert(lend > 1);
	--lend;
	while( lend-->0 && *src ){
		*dst++=*src++;
	}
	*dst = 0;
	return dst;
}

const char* str_chr(const char* str, const char ch){
	const char* ret = strchr(str, ch);
	if( ret == NULL ){
		ret = &str[strlen(str)];
	}
	return ret;
}

char* string_append(char* dst, size_t* len, size_t* size, const char* src, size_t lenSrc){
	if( lenSrc == 0 ) lenSrc = strlen(src);

	if( lenSrc > (*size - *len)-1){
		size_t n = ROUND_UP(lenSrc+1, 128);
		*size += n;
		dst = realloc(dst, sizeof(char) * *size);
		if( dst == NULL ){
			dbg_fail("realloc");
		}
	}
	memcpy(&dst[*len], src, lenSrc);
	*len += lenSrc;
	dst[*len] = 0;
	return dst;
}

char* string_head(char* dst, size_t* len, size_t* size, const char* src, size_t lenSrc){
	if( lenSrc == 0 ) lenSrc = strlen(src);

	if( lenSrc > (*size - *len)-1){
		dbg_info("realloc");
		size_t n = ROUND_UP(lenSrc+1, 128);
		*size += n;
		dst = realloc(dst, sizeof(char) * *size);
		if( dst == NULL ){
			dbg_fail("realloc");
		}
	}
	memmove(&dst[lenSrc], dst, *len);
	memcpy(dst, src, lenSrc);
	*len += lenSrc;
	dst[*len] = 0;
	return dst;
}

char* str_vprintf(const char* format, va_list va1, va_list va2){
	size_t len = vsnprintf(NULL, 0, format, va1);
	if( len == 0 ){
		return NULL;
	}

	char* ret = mem_many(char, len+1);
	if( !ret ){
		dbg_fail("memory alloc");
	}
	vsprintf(ret, format, va2);
	return ret;
}

char* str_printf(const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	char* ret = str_vprintf(format, va1, va2);
	va_end(va1);
	va_end(va2);
	return ret;
}

void str_swap(char* restrict a, char* restrict b){
	for(; *a && *b; ++a, ++b ){
		SWAP(*a, *b);
	}
	if( *a == *b ) return;
	if( *a == 0 ){
		strcpy(a,b);
		*b = 0;
	}
	else{
		strcpy(b,a);
		*a = 0;
	}
}

ssize_t str_chomp(char* str){
	const ssize_t len = strlen(str);
	if( len > 1 && str[len-1] == '\n' ){
		str[len-1] = 0;
		return len-1;
	}
	return -1;
}

char* quote_printable_decode(size_t *len, const char* str){
	size_t strsize = strlen(str);
	char* ret = mem_many(char, strsize + 1);
	if( !ret ){
		err_pushno("eom");
		return NULL;
	}

	char* next = ret;
	while( *str ){
		if( *str != '=' ){
			*next++ = *str++;
		}
		else{
			++str;
			if( *str == '\r' ) ++str;
			if( *str == '\n' ){
				++str;
				continue;
			}	
			char val[3];
			val[0] = *str++;
			val[1] = *str++;
			val[2] = 0;
			*next++ = strtoul(val, NULL, 16);
		}
	}
	*next = 0;
	if( len ) *len = next - ret;
	return ret;
}

void str_toupper(char* dst, const char* src){
	while( (*dst++=toupper(*src++)) );
}

void str_tolower(char* dst, const char* src){
	while( (*dst++=toupper(*src++)) );
}
