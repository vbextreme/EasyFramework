#include <ef/file.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/mth.h>
#include <ef/err.h>

tar_t* ftar_open(const char* path, int orfd, const char* mode){
	int flags = 0;

	switch( *mode ){
		case 'r':
			flags = O_RDONLY;
		break;
		default:
			err_push("support only read, write mode is a to do");
		return NULL;
	}
	tar_t* tar = NULL;
	if( orfd != -1 && tar_fdopen(&tar, orfd, path, NULL, flags, 0, TAR_GNU | TAR_CHECK_MAGIC | TAR_CHECK_VERSION) ){
		err_pushno("fd(%d) open tar", orfd);
		return NULL;
	}
	else if( path && tar_open(&tar, path, NULL, flags, 0, TAR_GNU | TAR_CHECK_MAGIC | TAR_CHECK_VERSION) ){
		err_pushno("path(%s) open tar", path);
		return NULL;
	}
	return tar;
}

void ftar_close_auto(tar_t** tar){
	if( *tar == NULL ) return;
	ftar_close(*tar);
	*tar = NULL;
}

err_t ftar_extract_reg(tar_t* tar, int fdout){
	char buf[T_BLOCKSIZE];
	const off_t size = th_get_size(tar);
	int i;
	for (i = size; i > 0; i -= T_BLOCKSIZE) {
		int n_buf = tar_block_read(tar, buf);
		if (n_buf == EOF) {
			break;
		}
		else if( n_buf > T_BLOCKSIZE ){
			n_buf = T_BLOCKSIZE;
		}
		ssize_t nw = fd_write(fdout, buf, n_buf);
		if( nw != n_buf ){
			return -1;
		}
    }
	return 0;
}

