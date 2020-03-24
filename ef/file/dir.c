#include <ef/file.h>
#include <ef/err.h>

void dir_close_auto(dir_s** dir){
	if( *dir == NULL ) return;
	dir_close(*dir);
	*dir = NULL;
}

int dirent_currentback(dirent_s* ent){
	int ret = 0;
	const char* name = ent->d_name;
	if( *name == '.' ){
		++ret;
		++name;
	}
	if( *name == '.' ){
		++ret;
		++name;
	}
	return *name == 0 ? ret : 0;
}

