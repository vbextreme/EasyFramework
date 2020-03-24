#include <ef/type.h>
#include <ef/file.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/mth.h>
#include <ef/err.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <time.h>
#include <utime.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

void fd_close_auto(int* fd){
	if( *fd == -1 ){
		return;
	}
	close(*fd);
	*fd = -1;
}

int fd_open(const char* path, const char* mode, int privileges){
	int flags = 0;

	if( *mode == '!' ){
		flags |= O_EXCL;
		++mode;
	}

	if( *mode == '&' ){
		flags |= O_NONBLOCK;
		++mode;
	}

	switch( *mode++ ){
		case 'r':
			flags |= O_RDONLY;
		break;
		case 'w':
			flags |= O_WRONLY | O_CREAT | O_TRUNC;
		break;
		case 'a':
			flags |= O_WRONLY | O_CREAT | O_APPEND;
		break;
		default:
		return -1;
	}

	switch( *mode++ ){
		case 0:
		break;
		case '+':
			flags &= ~(O_RDONLY | O_WRONLY);
			flags |= O_RDWR;
		break;
		default:
		return -1;
	}



	int fd = open(path, flags, privileges);
	if( fd < 0 ) err_pushno("open %s", path);
	return fd;
}

err_t fd_open_tmp(filetmp_s* out, const char* path, int privileges){
	static size_t count = 0;
	if( strlen(path) + 32 > PATH_MAX ) return -1;
	const char* pre = path;
	const char* post = strchr(path,'%');
	if( post == NULL ) return -1;
	const size_t sizePre = post-pre;
	++post;
	err_disable();
	do{
		sprintf(out->name, "%.*s%lu%s", (int)sizePre, path, count++, post);
		out->fd = fd_open(out->name, "!w+", privileges);
	}while( out->fd < 0 && errno == EEXIST );
	err_restore();
	if( out->fd < 0 ){
		err_pushno("try to opening temp file %s", path);
		return -1;
	}
	return 0;
}

ssize_t fd_kbhit(int fd){
	int ret;
	if( ioctl(fd, FIONREAD, &ret) ){
		err_pushno("fionread(%d)", fd);
		return -1;
	}
	return ret;
}

ssize_t fd_read(int fd, void* buf, size_t size){
	ssize_t ret;
	//dbg_info("fdread(%lu)", size);
	while( (ret=read(fd, buf, size)) < 0 && errno == EAGAIN );
	if( ret < 0 ){
		err_pushno("read(%d)", fd);
	}
	//dbg_info("fdread('%s')", (char*)buf);

	return ret;
}

ssize_t fd_read_chunk(int fd, void* buf, size_t chunk){
	ssize_t nr;
	ssize_t len = 0;
	char* next = buf;
	while( chunk > 0 && (nr = fd_read(fd, &next[len], chunk)) > 0 ){
		chunk -= nr;
		len += nr;
	}
	if( nr < 0 ) return -1;
	return len;
}

void* fd_slurp(size_t* outlen, int fd, size_t chunk, int nullchar){
	size_t next = 0;
	ssize_t nc;
	char* buf = mem_many(char, chunk);
	if( !buf ) return NULL;

	while( (nc = fd_read_chunk(fd, &buf[next], chunk)) == (ssize_t)chunk ){
		next += FILE_CHUNK;
		char* tmp = realloc(buf, sizeof(char) * (next+chunk));
		if( tmp == NULL ){
			free( buf );
			return NULL;
		}
		buf = tmp;
	}
	if( nc < 0 ){
		free(buf);
		return NULL;
	}
	if( next + nc < 1 ){
		free(buf);
		return NULL;
	}

	if( outlen ) *outlen = next + nc;

	if( nullchar ){
		buf[next+nc] = 0;
	}

	return buf;
}


ssize_t fd_write(int fd, void* buf, size_t size){
	ssize_t ret;
	while( (ret=write(fd, buf, size)) < 0 ){
		if( errno != EAGAIN ) return ret;
	}
	if( ret < 0 ){
		err_pushno("write(%d)", fd);
	}
	return ret;
}

size_t fd_size(int fd){
	size_t pos = fd_tell(fd);
	fd_toend(fd);
	size_t size = fd_tell(fd);
	fd_seek(fd,pos,SEEK_SET);
	return size;
}

void* fd_mmap(int fd, size_t len, char* mode){
	int prot = 0;
	int flags = MAP_PRIVATE;

	switch( *mode ){
		case '*':
			flags &= ~MAP_PRIVATE;
			flags |= MAP_SHARED;
			++mode;
		break;
	}

	switch( *mode++ ){
		case 'r':
			prot |= PROT_READ;
		break;
		case 'w':
			prot |= PROT_WRITE;
		break;
		
		case 'e':
			prot |= PROT_EXEC;
		break;

		default:
		return NULL;
	}

	switch( *mode ){
		case 0:
		break;
		case '+':
			prot |= PROT_READ | PROT_WRITE;
		break;
		default:
		return NULL;
	}

	char* mem = mmap(0, len, prot, flags, fd, 0);
	if( mem == (char*)-1 ){
		err_pushno("mmap(%d)", fd);
		return NULL;
	}
	return mem;
}

err_t fd_copy(int fdo, int fdi){
	char buf[FILE_BUFFER];
	ssize_t rd;
	while( (rd = fd_read(fdi, buf, FILE_BUFFER * sizeof(char))) > 0 ){
		ssize_t wr = fd_write(fdo, buf, rd * sizeof(char));
		if( wr != rd ) return -1;	
	}
	return rd < 0 ? -1 : 0;
}

ssize_t fd_read_cb(int fd, rbuffer_s* cb, void* out, size_t size){
	char* om = out;
	ssize_t rr = 0;
	while( rr < (ssize_t)size ){
		if( rbuffer_isempty(cb) ){
			size_t calw = rbuffer_available_linear_write(cb);
			if( calw < 1 ){
				//dbg_info("fill buffer: %d", 1);		
				ssize_t nr = fd_read(fd, rbuffer_addr_w(cb), cb->sof);
				//dbg_info("readed:%ld",nr);
				if( nr < 1 ){
					//dbg_info("read return %ld", nr);
					return nr;
				}
				//dbg_info("add the %ld element", nr);
				rbuffer_sync_write(cb, 1);
			}
			else{
				//dbg_info("fill buffer: %ld", calw);
				ssize_t nr = fd_read(fd, rbuffer_addr_w(cb), cb->sof * calw);
				//dbg_info("readed:%ld",nr);
				if( nr < 1 ){
					//dbg_info("read return %ld", nr);
					return nr;
				}
				//dbg_info("add the %ld element", nr);
				rbuffer_sync_write(cb, nr);
			}
		}
		if( rbuffer_read(cb, om) ){
			//dbg_warning("ring read error");
			break;
		}
		om += cb->sof;
		++rr;
	}
	return rr;
}

char* fd_read_cbline(int fd, rbuffer_s* cb, int withnl){
	size_t size = 32;
	size_t len = 0;
	char* out = mem_many(char, size);
	char* ptr = out;
	if( !out ) return NULL;
	char ch;

	while( fd_read_cb(fd, cb, &ch, 1) > 0 ){
		//dbg_info("getted(%c)", ch);
		if( ch == '\n' ){
			if( withnl ) *ptr++ = ch;
			break;
		}
		*ptr++ = ch;
		++len;
		if( len >= (size-1) ){
			size += 32;
			size_t pos = ptr - out;
			out = realloc(out, size);
			if( !out ){
				err_fail("realloc fd_read_cbline");
			}
			ptr = out + pos;
		}
	}
	*ptr = 0;
	if( *out == 0 ){
		free(out);
		return NULL;
	}
	return out;
}

int fd_timeout(int fd, long timeoutms){
	struct timeval to = { 
		.tv_sec = 0,
		.tv_usec = timeoutms * 1000
	};

	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(fd, &rfd);
	int ret = select( fd+1, &rfd, NULL, NULL, &to);
	if( ret < 0 ){
		err_pushno("select fd %d", fd);
		return -1;
	}
	return ret == 0 ? 1 : 0;
}



