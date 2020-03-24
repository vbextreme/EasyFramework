#include <ef/file.h>
#include <ef/err.h>

char const* file_extension(char const* name){
	size_t p = strlen(name)-1;
	while( p > 0 && name[p] != '.' ) --p;
	return name[p] == '.' ? &name[p+1] : NULL;
}

int file_exists(const char* path){
	stat_s b;
	if( stat(path, &b) ){
		err_push("file %s not exist", path);
		return 0;
	}
	return 1;
}

void file_close_auto(file_t** file){
	if( *file == NULL ){
		return;
	}
	fclose(*file);
	*file = NULL;
}

FILE* file_dup(FILE* file, char* mode){
	int fd = fileno(file);
	int newfd = dup(fd);
	return fdopen(newfd, mode);
}

__private err_t rec_rm(char* path){
	__dir_close dir_s* local = dir_open(path);
	if( local == NULL ){
		dbg_error("open dir %s", path);
		dbg_errno();
		return -1;
	}

	const size_t plen = strlen(path);
	dir_foreach(local, file){
		if( dirent_type(file) == FILE_DIRECTORY ){
			if( dirent_currentback(file) ) continue;
			if( path_add(path, dirent_name(file)) ) return -1;
			if( rec_rm(path) ) return -1;
		}
		else{
			if( path_add(path, dirent_name(file)) ) return -1;
			unlink(path);
		}
		path[plen] = 0;
	}

	return rmdir(path);
}

err_t file_rm(const char* path){
	stat_s st;
	if( stat(path, &st) ){
		err_pushno("stats");
		return -1;
	}
	if( S_ISDIR(st.st_mode) ){
		char ph[PATH_MAX];
		strcpy(ph, path);
		return rec_rm(ph);
	}
	else{
		unlink(path);
	}

	return 0;	
}
