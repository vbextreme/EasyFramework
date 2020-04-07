#include <ef/imageFiles.h>
#include <ef/imagePng.h>
#include <ef/imageJpeg.h>
#include <ef/imageSvg.h>
#include <ef/file.h>
#include <ef/err.h>

g2dImage_s* g2d_load(char const* path, unsigned width, unsigned height){
	if( !file_exists(path) ){
		err_pushno("file not exists");
		return NULL;
	}
	
	errno = 0;
   	g2dImage_s* ret = g2d_load_png(path);
	if( ret ){
		if( width && height ){
			g2dImage_s* scaled = g2d_resize(ret, width, height);
			g2d_free(ret);
			ret = scaled;
		}
		return ret;
	}
	if( errno != 666 ){
		err_push("on load png");
		return NULL;
	}

	errno = 0;
	ret = g2d_load_jpeg(path);
	if( ret ){
		if( width && height ){
			g2dImage_s* scaled = g2d_resize(ret, width, height);
			g2d_free(ret);
			ret = scaled;
		}
		return ret;
	}
	if( errno != 666 ){
		err_push("on load jpeg");
		return NULL;
	}

	errno = 0;
	ret = g2d_load_svg(path, width, height);
	if( ret ){
		return ret;
	}
	if( errno != 666 ){
		err_push("on load svg");
		return NULL;
	}

	dbg_error("unknow image format");
	err_push("unknow image format");
	return NULL;
}

