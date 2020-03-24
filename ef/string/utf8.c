#include <ef/type.h>
#include <ef/utf8.h>
#include <ef/memory.h>
#include <ef/err.h>

const char* lc_charset = NULL;

void utf_begin(void){
	setlocale (LC_ALL, "");
	lc_charset = locale_charset();
}

utf8_t* utf8_to(const utf8_t* str, size_t n){
	const utf8_t* prev = str;
	while( n-->0 && str ){
		prev = str;
		utf_t unused;
		str = utf_next(&unused, str);
	}
	return str ? U8(str) : U8(prev);
}	

void utf8_ins(utf8_t* dst, size_t dnch, utf8_t* src, size_t snch){
	utf_move_n(dst+snch, dst, dnch);
	utf_cpy_n(dst,src,snch);
}

void utf8_append(utf8_t* dst, utf8_t* src){
	utf8_ins(dst, utf_len(dst), src, utf_len(src));
}

void utf8_replace(utf8_t* dst, const utf8_t* src){
	utf_t chs;
	utf_t chd;
	utf8_t* prev = dst;

	while( (src = utf_next(&chs, src)) && dst ){
		utf_putch(dst, chs);
		prev = dst;
		dst = U8(utf_next(&chd,dst));
	}
	if( !dst ) utf_cpy(prev, src);
}

void utf8_del_n(utf8_t* dst, size_t dnch, size_t ndel){
	utf_move_n(dst, dst + ndel, dnch - ndel );
}

void utf8_del(utf8_t* dst, size_t ndel){
	utf8_del_n(dst, utf_len(dst), ndel);
}

void utf8_delu(utf8_t* dst, size_t ndel){
	iassert(dst);
	const utf8_t* src = dst;
	utf_t ch;
	while( ndel --> 0 && src ){
		src = utf_next(&ch, src);
	}
	if( src == NULL ){
		*dst = 0;
		return;
	}

	while( (src = utf_next(&ch,src)) ){
		utf_putch(dst,ch);
		dst =(utf8_t*)utf_next(&ch, dst);
	}
	*dst = 0;
}

int utf8_resize(utf8_t** str, size_t element){
	utf8_t* mem = realloc(*str, sizeof(utf8_t) * element);
	if( !mem ) return -1;
	*str = mem;
	return 0;
}

void utf8_fputchar(FILE* fd, utf_t ch){
	utf8_t pch[8] = {0};
	utf_putch(pch, ch);
	utf_fprintf(fd, "%U", pch);
}

utf8Iterator_s utf8_iterator(utf8_t* begin, size_t index){
	utf8Iterator_s it = { .begin = begin, .str = begin, .id = index };
	return it;
}

void utf8_iterator_rewind(utf8Iterator_s* it){
	it->str = it->begin;
	it->id = 0;
}

size_t utf8_iteretor_position(utf8Iterator_s* it){
	return it->id;
}

utf_t utf8_iterator_next(utf8Iterator_s* it){
	utf_t ret;
	if( *it->str == 0 ) return 0;
	utf8_t* next = (utf8_t*)utf_next(&ret, it->str);
	if( next ){
		it->str = next;
	}
	else{
		while( *it->str ) ++it->str;
	}
	++it->id;
	return ret;
}

utf_t utf8_iterator_next_to(utf8Iterator_s* it, size_t count){
	utf_t ret;
	do{		
		ret = utf8_iterator_next(it);
	}while( ret && --count > 0 );
	return ret;
}

utf_t utf8_iterator_last(utf8Iterator_s* it){
	while( utf8_iterator_next(it) );
	return 0;
}

utf_t utf8_iterator_last_valid(utf8Iterator_s* it){
	utf_t ret = 0;
	utf_t tmp = 0;
	do{
		ret = tmp;
		tmp = utf8_iterator_next(it);
	}while( tmp );

	utf8_iterator_prev(it);
	return ret;
}

utf_t utf8_iterator_prev(utf8Iterator_s* it){
	utf_t ret;
	if( it->str == it->begin ){
		return 0;
	}
	utf8_t* prev = (utf8_t*)utf_prev(&ret, it->str, it->begin);
	if( prev ){
		it->str = prev;
	}
	--it->id;
	return ret;
}

utf_t utf8_iterator_prev_to(utf8Iterator_s* it, size_t count){
	utf_t ret;
	do{		
		ret = utf8_iterator_prev(it);
	}while( ret && --count > 0 );
	return ret;
}

void utf8_iterator_replace(utf8Iterator_s* it, utf_t ch){
	if( *it->str == 0 ){
		utf_putch(it->str, ch);
		utf8_iterator_next(it);
		*it->str=0;
	}
	else{
		utf_putch(it->str, ch);
		utf8_iterator_next(it);
	}
}

void utf8_iterator_replace_str(utf8Iterator_s* it, utf8_t* str, size_t width){
	while( width --> 0 && *str ){
		utf_t ch;
		iassert(str);
		str = (utf8_t*)utf_next(&ch,str);
		utf8_iterator_replace(it, ch);
	}
}

void utf8_iterator_insert(utf8Iterator_s* it, utf_t ch){
	utf8Iterator_s lc = *it;
	utf8Iterator_s bk = *it;
	while( ch != 0 ){
		utf_t mem = utf8_iterator_next(&lc);
		utf8_iterator_replace(&bk,ch);
		dbg_info("mem:%c ch:(%d)%c",mem,ch,ch);
		ch = mem;
	}
	utf8_iterator_next(it);
	dbg_info("string8 '%s'", it->begin);
}

void utf8_iterator_insert_str(utf8Iterator_s* it, utf8_t* str, size_t width){
	while( width --> 0 && *str ){
		utf_t ch;
		str = (utf8_t*)utf_next(&ch,str);
		utf8_iterator_insert(it, ch);
	}
}

utf_t utf8_iterator_delete(utf8Iterator_s* it){
	utf8Iterator_s lc = *it;
	utf8Iterator_s bk = *it;
	utf_t ch = utf8_iterator_next(&lc);
	utf_t ret = ch;
	while( ch != 0 ){
		ch = utf8_iterator_next(&lc);
		utf8_iterator_replace(&bk, ch);
	}
	return ret;
}

utf_t utf8_iterator_delete_to(utf8Iterator_s* it, size_t count){
	utf_t ret = 0;
	while( count --> 0 ){
		ret = utf8_iterator_delete(it);
	}
	return ret;
}

void utf8_chomp(utf8_t* str){
	size_t len = utf_len(str);
	if( str[len-1] == '\n' ){
		str[len-1] = 0;
	}
}

utf8_t* utf8_gets(utf8_t* line, size_t max, FILE* fd){
	if( !fgets((char*)line, max, fd) ){
		return NULL;
	}
	if( utf_validate_n(line, max) ){
		return NULL;
	}
	return line;
}

utf8_t* utf8_gets_alloc(size_t* outsize, size_t chunk, int nl, FILE* fd){
	utf8_t* begin = mem_many(utf8_t, chunk);
	if( !begin ){
		err_pushno("malloc");
		return NULL;
	}
	size_t size = chunk;
	size_t wr = 0;
	int ch;
	while( (ch=fgetc(fd)) != EOF && ch != '\n' ){
		begin[wr++] = ch;
		if( wr >= size - 1 ){
			size += chunk;
			if( utf8_resize(&begin, size) ){
				free(begin);
				return NULL;
			}
		}
	}
	if( nl && ch == '\n' ){
		begin[wr++] = '\n';
	}
	begin[wr] = 0;

	const utf8_t* che;
	if( (che=utf_validate_n(begin, wr)) ){
		err_push("invalid utf8 on ch number %ld, '%s'", che - begin, che);
		free(begin);
		return NULL;
	}
	if( outsize ) *outsize = size;
	return begin;
}

__private int ishex(const char ch){
	if( ch >= '0' && ch <= '9' ) return 1;
	if( ch >= 'a' && ch <= 'f' ) return 1;
	if( ch >= 'A' && ch <= 'F' ) return 1;
	return 0;
}

__private err_t utf16_extract_escaped(utf16_t* out, const char* str, const char** end){
	char pair[3] = {[2] = 0};

	if( !*str || *str != '\\' ) return -1;
	++str;
		
	if( *str != 'u' && *str != 'U' ) return -1;
	++str;

	//dbg_info("strn:%.4s",str);

	if( !ishex(*str) ) return -1;
	pair[0] = *str++;
	if( !ishex(*str) ) return -1;
	pair[1] = *str++;
	//dbg_info("pairH:%lu",strtoul(pair, NULL, 16));
	*out = ((uint16_t)strtoul(pair, NULL, 16)) << 8;

	if( !ishex(*str) ) return -1;
	pair[0] = *str++;
	if( !ishex(*str) ) return -1;
	pair[1] = *str++;
	//dbg_info("pairL:%lu",strtoul(pair, NULL, 16));
	*out += strtoul(pair, NULL, 16);

	*end = str;

	return 0;
}

ssize_t utf8_from_seu16(utf8_t* out, size_t size, const char* str, const char** end){
	utf16_t u16[4] = {0};
	size_t nch = 0;
	const char* next = str;

	if( utf16_extract_escaped(&u16[0], str, &next) ){
		//dbg_warning("fail to extract escaped u16");
		return -1;
	}
	if( utf_validate_n(u16, 1) ){
		//dbg_info("single u16 is invalid");
		str = next;
		if( utf16_extract_escaped(&u16[1], str, &next) ){
			dbg_error("invalid surrogate");
			return -1;
		}
		if( utf_validate_n(u16, 2) ){
			dbg_error("the unicode is invalid");
			return -1;
		}
		nch = 1;
	}
	++nch;
	
	if( out ){
		//dbg_info("cast16:%s",(char*)u16);
		utf16_cast_n(u16, nch, out, &size);
		//dbg_info("casting:%lu ch", size);
		out[size] = 0;
		//dbg_info("casted:[0](%d)'%s'", *out, out);
	}
	else{
		utf8_t u8[32];
		utf16_cast_n(u16, nch, u8, &size);
	}
	
	if( end ) *end = next;

	return size;
}


















