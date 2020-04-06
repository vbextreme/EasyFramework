#include <ef/imageFiles.h>
#include <ef/imagePng.h>
#include <ef/file.h>
#include <ef/err.h>

g2dImage_s* g2d_load(char const* path){
	if( !file_exists(path) ){
		err_pushno("file not exists");
		return NULL;
	}
	
	errno = 0;
   	g2dImage_s* ret = g2d_load_png(path);
	if( ret ) return ret;
	if( !ret && errno != 666 ){
		err_push("on load png");
		return NULL;
	}

	dbg_error("unknow image format");
	err_push("unknow image format");
	return NULL;
}

