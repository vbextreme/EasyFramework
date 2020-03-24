#include <ef/file.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

#include <pwd.h>

void path_kill_back(char* path){
	size_t len = strlen(path);
	if( len == 1 ) return;
	if( path[len-1] == '/' ) path[len-1] = 0;
	char* bs = strrchr(path, '/');
	if( bs ) *bs = 0;
}

void path_set_last(char* path){
	char* bs = strrchr(path, '/');
	if( !bs ) return;
	if( *(bs+1) == 0 ){
		*bs = 0;
		bs = strrchr(path, '/');
	}
	if( bs ){
		strcpy(path, bs+1);	
	}
}

err_t path_current(char* path){
	iassert(path);
	if( !getcwd(path, PATH_MAX) ){
		err_pushno("getcwd");
		return -1;
	}
	return 0;
}

char* path_current_new(void){
	return getcwd(NULL, 0);
}

err_t path_add(char* path, const char* next){
	size_t l = strlen(path);
	if( l + strlen(next) + 1 > PATH_MAX ){
		err_push("path to long: %s/%s", path, next);
		return -1;
	}
	if( path[l-1] != '/' ){
		path[l++] = '/';
	}
	if( *next == '/' ) ++next;
	strcpy(&path[l], next);
	return 0;
}

err_t path_home(char* path){
        char *hd;
        if( (hd = secure_getenv("HOME")) == NULL ){
                struct passwd* spwd = getpwuid(getuid());
                if( !spwd ){
                        err_pushno("no home available");
                        *path = 0;
                        return -1;
                }
                strcpy(path, spwd->pw_dir);
        }
        else{
                strcpy(path, hd);
        }
        return 0;
}

char* path_resolve(const char* path){
	char tmp[PATH_MAX];

	if( *path == '~' ){
		if( path_home(tmp) ){
			return NULL;
		}
		strcpy(&tmp[strlen(tmp)], &path[1]);
		return str_dup(tmp,0);
	}
	else if( *path == '.' ){
		path_current(tmp);
		if( path[1] == '.' ){
			path_kill_back(tmp);
			if( path[2] ){
				size_t l = strlen(tmp);
				if( path[2] != '/' ){
					tmp[l++] = '/';
					tmp[l] = 0;
				}
				strcpy(&tmp[strlen(tmp)], &path[2]);
				return str_dup(tmp,0);
			}
			return str_dup(tmp, 0);
		}
		if( path[1] == '/' ){
			size_t l = strlen(tmp);
			if( path[1] != '/' ){
				tmp[l++] = '/';
				tmp[l] = 0;
			}
			strcpy(&tmp[strlen(tmp)], &path[2]);
			return str_dup(tmp,0);
		}
		return str_dup(tmp, 0);
	}
	else if( *path != '/' ){
		path_current(tmp);	
		size_t l = strlen(tmp);
		if( tmp[l-1] != '/' ){
			tmp[l++] = '/';
			tmp[l] = 0;
		}
		strcpy(&tmp[strlen(tmp)], path);
		return str_dup(tmp,0);
	}
	return str_dup(path,0);
}

char* path_resolve_custom(const char* path, const char* home, const char* current){
	char tmp[PATH_MAX];

	if( *path == '~' ){
		if( !home ){
			return NULL;
		}
		strcpy(&tmp[strlen(tmp)], &path[1]);
		return str_dup(tmp,0);
	}
	else if( *path == '.' ){
		strcpy(tmp, current);
		if( path[1] == '.' ){
			path_kill_back(tmp);
			if( path[2] ){
				size_t l = strlen(tmp);
				if( path[2] != '/' ){
					tmp[l++] = '/';
					tmp[l] = 0;
				}
				strcpy(&tmp[strlen(tmp)], &path[2]);
				return str_dup(tmp,0);
			}
			return str_dup(tmp, 0);
		}
		if( path[1] == '/' ){
			size_t l = strlen(tmp);
			if( path[1] != '/' ){
				tmp[l++] = '/';
				tmp[l] = 0;
			}
			strcpy(&tmp[strlen(tmp)], &path[2]);
			return str_dup(tmp,0);
		}
		return str_dup(tmp, 0);
	}
	else if( *path != '/' ){
		strcpy(tmp,current);
		size_t l = strlen(tmp);
		if( tmp[l-1] != '/' ){
			tmp[l++] = '/';
			tmp[l] = 0;
		}
		strcpy(&tmp[strlen(tmp)], path);
		return str_dup(tmp,0);
	}
	return str_dup(path,0);
}

