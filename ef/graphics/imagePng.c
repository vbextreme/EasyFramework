#include <ef/image.h>
#include <ef/memory.h>
#include <ef/file.h>
#include <ef/err.h>
#include <png.h>
#include <setjmp.h>

#define PNG_BYTES_TO_CHECK 8
__private int file_is_png(FILE* fd){
	unsigned char buf[PNG_BYTES_TO_CHECK];
	if (fread(buf, 1, PNG_BYTES_TO_CHECK, fd) != PNG_BYTES_TO_CHECK)
		return 0;
	return(!png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
}

g2dImage_s* g2d_load_png(char const* path){	
	volatile g2dImage_s* img = NULL;

	volatile FILE* fd = fopen(path, "r");
	if( fd == NULL ){
		err_pushno("open png");
		dbg_error("open png");
		dbg_errno();
		return NULL;
	}
	if( !file_is_png((FILE*)fd) ){
		dbg_warning("is not png");
		fclose((FILE*)fd);
		errno = 666;
		return NULL;
	}

	volatile png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if( !png ){
		err_push("on create read struct");
		dbg_error("on create read struct");
		fclose((FILE*)fd);
		return NULL;
	}

	volatile png_infop info = png_create_info_struct(png);
	if( !info ){
		err_push("on create info struct");
		dbg_error("on create info struct");
		png_destroy_read_struct((png_structp*)&png, NULL, NULL);
		fclose((FILE*)fd);
		return NULL;
	}

	if( setjmp(png_jmpbuf(png)) ){
		err_push("on load png");
		dbg_error("on load png");
		png_destroy_read_struct((png_structp*)&png, NULL, NULL);
		g2d_free((g2dImage_s*)img);
		fclose((FILE*)fd);
		return NULL;
	}	
	
	png_init_io(png, (FILE*)fd);
    png_set_sig_bytes(png, PNG_BYTES_TO_CHECK);
	png_read_info(png, info);
	
	unsigned width = png_get_image_width(png, info);
    unsigned height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
	
	dbg_info("png size %u*%u", width, height);
	img = g2d_new(width, height, -1);

	png_set_bgr(png);

	if( bit_depth == 16 ){
		dbg_info("16bit");
		png_set_strip_16(png);
	}

	if( color_type == PNG_COLOR_TYPE_PALETTE ){
		dbg_info("set palette");
		png_set_palette_to_rgb(png);
	}

	if( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 ){
		dbg_info("low gray");
		png_set_expand_gray_1_2_4_to_8(png);
	}

	if( png_get_valid(png, info, PNG_INFO_tRNS) ){
		dbg_info("set rns alpha");
		png_set_tRNS_to_alpha(png);
	}

	if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE){
		dbg_info("set filler 0xFF filler after");
	  	png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}

	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA){
		dbg_info("gray to rgb");
		png_set_gray_to_rgb(png);
	}
	
	png_read_update_info(png, info);

	for(unsigned y = 0; y < height; y++) {
		unsigned const row = g2d_row(img, y);
		unsigned char* pix = (unsigned char*)g2d_color(img, row, 0);
		png_read_rows(png, &pix, NULL, 1);		
	}
	
	png_destroy_read_struct((png_structp*)&png, NULL, NULL);
	fclose((FILE*)fd);
	return (g2dImage_s*)img;
}

