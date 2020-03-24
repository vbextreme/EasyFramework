#include <ef/file.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/mth.h>
#include <ef/err.h>

#include <stdarg.h>

stream_s* stream_open_fd(int fd, size_t chunk){
	if( fd == -1 ){
		err_push("can't open stream with fd -1");
		return NULL;
	}
	if( chunk == 0 ) chunk = STREAM_CHUNK;

	stream_s* sm = malloc(sizeof(stream_s) + chunk * 2);
	if( !sm ){
		err_pushno("malloc");	
		return NULL;
	}

	sm->read = NULL;
	sm->write = NULL;
	sm->kbhit = NULL;
	sm->rwuserdata = NULL;
	sm->fd = fd;
	sm->r.len = sm->w.len = 0;
	sm->r.cursor = sm->w.cursor = 0;
	sm->r.size = sm->w.size = chunk;
	
	sm->r.buf = &sm->rwbuffer[chunk]; 
	sm->w.buf = &sm->rwbuffer[0];
	return sm;
}

stream_s* stream_open(const char* path, const char* mode, int privileges, size_t chunk){
	return stream_open_fd(fd_open(path, mode, privileges), chunk);
}

stream_s* stream_open_tmp(char* outpathmax, const char* path, int privileges, size_t chunk){
	filetmp_s ft;
	if( fd_open_tmp(&ft, path, privileges) ) return NULL;
	strcpy(outpathmax, ft.name);
	return stream_open_fd(ft.fd, chunk);
}

stream_s* stream_open_mem(const char* name, size_t chunk){
	int fd = fd_mem_create(name);
	if( fd < 0 ){
		err_pushno("memcreate(%s)", name);
		return NULL;
	}
	return stream_open_fd(fd, chunk);
}

void stream_replace_io(stream_s* sm, customio_f r, customio_f w, customio_f kbhit, void* iouserdata){
	sm->read = r;
	sm->write = w;
	sm->kbhit = kbhit;
	sm->rwuserdata = iouserdata;
}

void stream_detach(stream_s* sm){
	free(sm);
}

void stream_close(stream_s* sm){
	stream_flush(sm);
	fd_close(sm->fd);
	free(sm);
}

void stream_close_auto(stream_s** sm){
	if( *sm ){
		stream_close(*sm);
	}
}

ssize_t stream_size(stream_s* sm){
	return fd_size(sm->fd);
}

ssize_t stream_kbhit(stream_s* sm){
	ssize_t kb = sm->kbhit ? sm->kbhit(sm, NULL, 0) : fd_kbhit(sm->fd);
	if( kb == -1 ){
		return -1;
	}
	if( sm->r.len ){
		kb += (sm->r.len - (sm->r.cursor));
	}
	return kb;
}

__private ssize_t stream_rfill(stream_s* sm){
	ssize_t nr;
   	if( sm->read ){
		nr = sm->read(sm, sm->r.buf, sm->r.size);	
	}
	else{
		nr = fd_read(sm->fd, sm->r.buf, sm->r.size);
	}
	if( nr == -1 ) return -1;
	sm->r.len = nr;
	sm->r.cursor = 0;
	return nr;
}

#define stream_rempty(SM) ((SM)->r.len == 0 || (SM)->r.len == (SM)->r.cursor)
#define stream_ravailable(SM) ((SM)->r.len - (SM)->r.cursor)

#define stream_fast_read_char(SM,CH) ({\
	int ret = 1;\
	if( stream_rempty(SM) ){\
		ret = stream_read(SM, &CH, 1);\
	}\
	else{\
		CH = (SM)->r.buf[(SM)->r.cursor++];\
	}\
	ret;\
})

		
ssize_t stream_read(stream_s* sm, void* buf, size_t size){
	char* rd = buf;
	//dbg_info("read(%lu)", size);
	while( size > 0 ){
		if( stream_rempty(sm) ){
			ssize_t ret = stream_rfill(sm);
			if( ret <= 0 ){
				//dbg_warning("rfille end");
				return ret;
			}
			
			//if( stream_rempty(sm) && stream_rfill(sm) <= 0 ){
			//dbg_error("stream empty and error fill");
			//return -1;
			//return rd - (char*)buf;
		}
		//if( stream_rempty(sm) ){
			//dbg_info("stream empty");
			//return rd - (char*)(buf);
		//}
		size_t ncp = size < stream_ravailable(sm) ? size : stream_ravailable(sm);
		memcpy(rd, &sm->r.buf[sm->r.cursor], ncp);
		sm->r.cursor += ncp;
		size -= ncp;
		rd += ncp;
	}

	//dbg_info("readed(%lu)", rd - (char*)(buf));
	return rd - (char*)(buf);
}

void stream_rollback(stream_s* sm, void* data, size_t size){
	//dbg_info("rollback:%ld start buffer size:%ld", size, stream_kbhit(sm));

	const size_t tav = stream_ravailable(sm) + sm->r.cursor;
	if( tav < size ){
		const size_t nsz = ROUND_UP(size - tav, sm->r.size) + sm->r.size ;
		char* tmp = realloc(sm->r.buf, sizeof(char) * nsz);
		if( tmp == NULL ){
			err_fail("realloc stream");
		}
		sm->r.buf = tmp;
		sm->r.size = nsz;
		//dbg_info("need increase stream to %lu", nsz);
	}

	//dbg_info("rollback.begin:<<%.*s>>", (int)stream_ravailable(sm), &sm->r.buf[sm->r.cursor]);
	memmove(&sm->r.buf[size], &sm->r.buf[sm->r.cursor], stream_ravailable(sm));
	memcpy(sm->r.buf,data,size);
	sm->r.len = stream_ravailable(sm) + size;
	sm->r.cursor = 0;
	//dbg_info("rollback.new:<<%.*s>>", (int)stream_ravailable(sm), &sm->r.buf[sm->r.cursor]);
	//dbg_info("end buffer size:%ld", stream_kbhit(sm));
}

void* stream_slurp(size_t* outlen, stream_s* sm, int addNullChar){
	size_t size = sm->r.size + addNullChar;
	size_t len = 0;
	char* out = mem_many(char, size);

	if( out == NULL ){
		err_pushno("malloc");
		return NULL;
	}

	ssize_t nr;
	while( (nr = stream_read(sm, &out[len], size-len)) > 0 ){
		size += sm->r.size;
		char* tmp = realloc(out, sizeof(char)*size);
		if( tmp == NULL ){
			err_pushno("realloc");
			free(out);
			return NULL;
		}
		out = tmp;
		len += nr;
	}
	if( nr < 0 ){
		free(out);
		return NULL;
	
	}
	len += nr;
	if( outlen ) *outlen = len;
	if( addNullChar ) out[len] = 0;

	return out;
}

ssize_t stream_inp_char(stream_s* sm, int* ch){
	return stream_fast_read_char(sm, *ch);
}

ssize_t stream_inp_string(stream_s* sm, char** out, char endch, int addch){
	size_t size = STREAM_STRING_CHUNK;
	char* str = mem_many(char,size+1+addch);
	size_t len = 0;
	char ch;
	
	while( stream_fast_read_char(sm, ch) == 1 && ch != endch ){
		str[len++] = ch;
		if( len >= size - (1+addch) ){
			size += STREAM_STRING_CHUNK;
			char* tmp = realloc(str, sizeof(char)*size);
			if( tmp == NULL ){
				//dbg_error("realloc");
				err_pushno("realloc");
				free(str);
				*out = NULL;
				return -1;
			}
			str = tmp;
		}
	}
	
	//dbg_info("readed:%lu",len);

	if( len == 0 ){
		//dbg_warning("no more string len:%lu", len);
		free(str);
		*out = NULL;
		return 0;
	}
	if( addch ) str[len++] = endch;
	str[len] = 0;
#if STREAM_STRING_FIT_REALLOC == _Y_
	*out = realloc(str, len+1);
	if( *out == NULL ){
		//dbg_error("realloc");
		err_pushno("realloc");
		free(str);
		return -1;
	}
#else
	*out = str;
#endif
	return len;
}

ssize_t stream_inp_toanyof(stream_s* sm, char** out, char* endch, int addch){
	size_t size = STREAM_STRING_CHUNK;
	char* str = mem_many(char,size+1+addch);
	size_t len = 0;
	char ch;
	
	while( stream_fast_read_char(sm, ch) == 1 && !strchr(endch, ch) ){
		//dbg_info("readed:%d(%c)",ch,ch);
		str[len++] = ch;
		if( len >= size - (1+addch) ){
			size += STREAM_STRING_CHUNK;
			char* tmp = realloc(str, sizeof(char)*size);
			if( tmp == NULL ){
				//dbg_error("realloc");
				err_pushno("realloc");
				free(str);
				*out = NULL;
				return -1;
			}
			str = tmp;
		}
	}
	if( len == 0  ){
		//dbg_warning("no more string len:%lu", len);
		free(str);
		*out = NULL;
		return 0;
	}
	if( addch ) str[len++] = ch;
	str[len] = 0;
#if STREAM_STRING_FIT_REALLOC == _Y_
	*out = realloc(str, len+1);
	if( *out == NULL ){
		//dbg_error("realloc");
		err_pushno("realloc");
		free(str);
		return -1;
	}
#else
	*out = str;
#endif
	return len;
}

ssize_t stream_inp_strstr(stream_s* sm, char** out, char* endstr, int addch){
	size_t size = STREAM_STRING_CHUNK;
	char* str = mem_many(char,size+1);
	size_t len = 0;
	char ch;
	size_t lenEnd = strlen(endstr);
	size_t iEnd = 0;
	
	while( iEnd < lenEnd && stream_fast_read_char(sm, ch) == 1 ){
		
		str[len++] = ch;
		if( endstr[iEnd] == ch ){
			++iEnd;
		}
		else{
			iEnd = 0;
		}

		if( len >= size - 1 ){
			size += STREAM_STRING_CHUNK;
			char* tmp = realloc(str, sizeof(char)*size);
			if( tmp == NULL ){
				err_pushno("realloc");
				free(str);
				*out = NULL;
				return -1;
			}
			str = tmp;
		}
	}
	if( !addch ){
		len -= lenEnd;
		str[len] = 0;
	}

	if( len == 0  ){
		free(str);
		*out = NULL;
		return 0;
	}

#if STREAM_STRING_FIT_REALLOC == _Y_
	*out = realloc(str, len+1);
	if( *out == NULL ){
		err_pushno("realloc");
		free(str);
		return -1;
	}
#else
	*out = str;
#endif
	return len;
}

__private ssize_t stream_inp_num(stream_s* sm, char* out){
	size_t size = STREAM_NUM_MAX_DIGIT - 1;
	char ch;
	char* begin = out;
	int ret;
	while( size-->0 && (ret=stream_fast_read_char(sm, ch)) == 1){
		dbg_info("readch:%d(%c)", ch, ch);
		if( strchr("0123456789-.eabcdefEABCDEFxX", ch) ){
			*out++ = ch;
		}
		else{
			stream_rollback(sm, &ch, 1);
			break;
		}
	}
	if( ret < 0 ){
		dbg_error("stream_read");
		return -1;
	}
	*out = 0;
	dbg_info("readed num:%s",begin);
	if( size == STREAM_NUM_MAX_DIGIT - 1 ) return -1;
	return out-begin;
}

ssize_t stream_inp_long(stream_s* sm, long* num, int base){
	char inp[STREAM_NUM_MAX_DIGIT];
	ssize_t ret;
	if( (ret = stream_inp_num(sm, inp)) < 1 ) return ret;
	char* en = NULL;
	*num = strtol(inp, &en, base);
	if( *en != 0 ){
		if( en == inp ) return -1;
		stream_rollback(sm, en, strlen(en));
	}
	return 1;
}

ssize_t stream_inp_ulong(stream_s* sm, unsigned long* num, int base){
	char inp[STREAM_NUM_MAX_DIGIT];
	ssize_t ret;
	if( (ret=stream_inp_num(sm, inp)) ) return ret;
	char* en = NULL;
	*num = strtoul(inp, &en, base);
	if( *en != 0 ){
		if( en == inp ) return -1;
		stream_rollback(sm, en, strlen(en));
	}
	return 1;
}

ssize_t stream_inp_double(stream_s* sm, double* num){
	char inp[STREAM_NUM_MAX_DIGIT];
	ssize_t ret;
	if( (ret=stream_inp_num(sm, inp)) ) return ret;
	char* en = NULL;
	*num = strtod(inp, &en);
	if( *en != 0 ){
		if( en == inp ) return -1;
		stream_rollback(sm, en, strlen(en));
	}
	return 1;
}

ssize_t stream_inp_float(stream_s* sm, float* num){
	double d = 0;
	ssize_t ret = stream_inp_double(sm, &d);
	*num = d;
	return ret;
}

#define stream_wfull(SM) ((SM)->w.len == (SM)->w.size)
#define stream_wavailable(SM) ((SM)->w.size - (SM)->w.len)

void stream_flush(stream_s* sm){
	ssize_t nw;
	size_t nt = 0;
	if( sm->write ){
		while( nt < sm->w.len && (nw = sm->write(sm, &sm->w.buf[nt], sm->w.len - nt)) > 0 ){
			nt += nw;
		}
	}
	else{
		while( nt < sm->w.len && (nw = fd_write(sm->fd, &sm->w.buf[nt], sm->w.len - nt)) > 0 ){
			nt += nw;
		}
	}
	sm->w.len = 0;
	//sm->w.cursor = 0;
}

ssize_t stream_write(stream_s* sm, void* buf, size_t size){
	char* mv = buf;

	while( size > 0 ){
		if( stream_wfull(sm) ) stream_flush(sm);
		size_t nc = size < stream_wavailable(sm) ? size : stream_wavailable(sm);
		memcpy(&sm->w.buf[sm->w.len], mv, nc);
		sm->w.len += nc;
		size -= nc;
		mv += nc;
	}	

	return 0;
}

err_t stream_out_char(stream_s* sm, char ch){
	return stream_write(sm, &ch, 1);
}

err_t stream_out_string(stream_s* sm, char* str, size_t len){
	if( len == 0 ) len = strlen(str);
	return stream_write(sm, str, len);
}

err_t stream_out_long(stream_s* sm, long num, int base){
	char nstr[STREAM_NUM_MAX_DIGIT];
	size_t n = STREAM_NUM_MAX_DIGIT - 1;
	int sign = num < 0 ?  1 : 0;
	do{
		long tmp = num;
		nstr[n--] = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35+(tmp-num*base)];
		iassert( n > 0 );
	}while( num );
	if( sign ){
		nstr[n] = '-';
	}
	else{
		++n;
	}
	return stream_write(sm, &nstr[n], STREAM_NUM_MAX_DIGIT - n);
}

err_t stream_out_ulong(stream_s* sm, unsigned long num, int base){
	char nstr[STREAM_NUM_MAX_DIGIT];
	size_t n = STREAM_NUM_MAX_DIGIT - 1;
	do{
		long tmp = num;
		nstr[n--] = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35+(tmp-num*base)];
		iassert( n > 0 );
	}while( num );
	++n;
	return stream_write(sm, &nstr[n], STREAM_NUM_MAX_DIGIT - n);
}

err_t stream_out_double(stream_s* sm, double num, int dec){
	char nstr[STREAM_NUM_MAX_DIGIT];
	ssize_t nw = snprintf(nstr, STREAM_NUM_MAX_DIGIT, "%.*f", dec, num);
	if( nw > 0 ){
		stream_write(sm, nstr, nw);
		return 0;
	}
	return -1;
}


__printf(2,3) err_t stream_printf(stream_s* sm, const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	
	size_t len = vsnprintf(NULL, 0, format, va1);
	if( len == 0 ){
		va_end(va1);
		va_end(va2);
		return -1;
	}

	__mem_free char* mem = mem_many(char, len+1);
	if( !mem ){
		err_pushno("malloc");
		return -1;
	}
	vsprintf(mem, format, va2);
	stream_write(sm, mem, len);
	scan_build_unknown_cleanup(mem);
	return 0;
}

err_t stream_stat(stream_s* sm, stat_s* stat){
	if( fd_stat(sm->fd, stat) ){
		err_pushno("stat");
		return -1;
	}
	return 0;
}

ssize_t stream_tell(stream_s* sm){
	ssize_t ret = lseek(sm->fd, 0, SEEK_CUR);
	if( ret == -1 ){
		err_pushno("seek");
		return -1;
	}
	return ret;
}

err_t stream_seek(stream_s* sm, ssize_t offset, int mode){
	if( mode == SEEK_CUR ){
		const ssize_t newc = sm->r.cursor + offset;
		if( newc >= 0 && newc < (ssize_t)sm->r.size ){
			sm->r.cursor = newc;
			return 0;
		}
	}
	ssize_t ret = lseek(sm->fd, offset, mode);
	if( ret == -1 ){
		err_pushno("seek");
		return -1;
	}
	sm->r.len = sm->r.cursor = 0;
	return ret;
}

err_t stream_skip_anyof(stream_s* sm, const char* lst){
	char ch;
	ssize_t r;

	while( (r=stream_fast_read_char(sm, ch)) == 1 && strchr(lst,ch));
		//dbg_info("skip %d",ch);
	
	if( r == 1 ){
		//dbg_info("push back char skipped because is not in list");
		stream_rollback(sm, &ch, 1);
	}

	return 0;
}

err_t stream_skip_to(stream_s* sm, const char* lst){
	char ch;
	ssize_t r;

	while( (r=stream_fast_read_char(sm, ch)) == 1 && !strchr(lst,ch));
	if( r == 1 ){
		stream_rollback(sm, &ch, 1);
	}

	return 0;
}

err_t stream_skip_next(stream_s* sm, const char* lst){
	char ch;
	ssize_t r;

	while( (r=stream_fast_read_char(sm, ch)) == 1 && !strchr(lst,ch));

	return 0;
}
