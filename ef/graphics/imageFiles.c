#include <ef/imageFiles.h>
#include <ef/imagePng.h>
#include <ef/file.h>
#include <ef/err.h>


err_t g2d_load(g2dImage_s* img, char const* path){
	if( !file_exists(path) ){
		err_pushno("file not exists");
		return -1;
	}
	switch( g2d_load_png(img, path) ){
		case 0: return 0;
		default: case -1: return -1;
		case -2: break;
	}

	err_push("unknow image format");	
	dbg_error("unknow image format");
	return -1;
}

