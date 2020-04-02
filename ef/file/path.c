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
	//dbg_info("get path:%s",path);
	char cur[PATH_MAX];
	char out[PATH_MAX];

	size_t lpath = strlen(path);
	if( lpath > PATH_MAX - 1 ){
		err_push("path %s to long", path);
		return NULL;
	}

	if( !str_equal(path, lpath, "~", 1) || !str_ancmp(path, "~/") ){
		if( path_home(cur) ){
			return NULL;
		}
		if( lpath + strlen(cur) > PATH_MAX -1 ){
			err_push("path %s + %s to long", cur, path);
			return NULL;
		}
		if( path[1] && path[2] ){
			strcpy(&cur[strlen(cur)], &path[2]);
		}
	}
	else if( !str_equal(path, lpath, ".", 1) || !str_ancmp(path, "./") ){
		path_current(cur);
		if( lpath + strlen(cur) > PATH_MAX - 1){
			err_push("path %s + %s to long", cur, path);
			return NULL;
		}
		if( path[1] && path[2] ){
			strcpy(&cur[strlen(cur)], &path[1]);
		}
	}
	else if( !str_equal(path, lpath, "..", 1) || !str_ancmp(path, "../") ){
		path_current(cur);
		path_kill_back(cur);
		if( lpath + strlen(cur) > PATH_MAX - 1){
			err_push("path %s + %s to long", cur, path);
			return NULL;
		}	
		if( path[2] && path[3] ){
			strcpy(&cur[strlen(cur)], &path[2]);
		}
	}
	else{
		strcpy(cur, path);
	}

	char* parse = cur;
	char* pout = out;
	while( *parse ){
		if( *parse == '.' ){
			if( *(parse+1) == '/' ){
				parse += 2;
				continue;
			}
			if( *(parse+1) == '.' ){
				if( *(parse+2) == 0 ){
					*pout = 0;
					path_kill_back(out);
					pout = out + strlen(out);
					parse += 2;
					continue;
				}
				if ( *(parse+2) == '/' ){
					*pout = 0;
					path_kill_back(out);
					pout = out + strlen(out);
					parse += 2;
					continue;
				}
			}
		}
		*pout++ = *parse++;
	}
	*pout = 0;
	return str_dup(out, pout-out);
}

//TODO change same path resolve
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

