#include <ef/image.h>
#include <ef/memory.h>
#include <ef/file.h>
#include <ef/err.h>

#include <jpeglib.h>
#include <setjmp.h>

typedef struct jpegErrorMgr {
	struct jpeg_error_mgr pub;
	int notjpeg;
	char jpegLastErrorMsg[JMSG_LENGTH_MAX];
	jmp_buf setjmp_buffer;
}jpegErrorMgr_s;

__private void jpeg_error_exit(j_common_ptr cinfo){
	jpegErrorMgr_s* myerr = (jpegErrorMgr_s*) cinfo->err;
	if( cinfo->err->msg_code == 55 ){
		myerr->notjpeg = 1;
	}
	else{
		myerr->notjpeg = 0;
		(*(cinfo->err->format_message))(cinfo, myerr->jpegLastErrorMsg);
	}
	longjmp(myerr->setjmp_buffer, 1);
}

g2dImage_s* g2d_load_jpeg(char const* path){
	volatile g2dImage_s* img = NULL;

	struct jpeg_decompress_struct cinfo;
	jpegErrorMgr_s jerr;
	
	FILE* infile = fopen(path, "rb");
	if( !infile ){
		err_pushno("open jpeg file");
		return NULL;
	}

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpeg_error_exit;
	
	if( setjmp(jerr.setjmp_buffer) ){
		if( jerr.notjpeg ){
			dbg_warning("not a jpeg");
			errno = 666;
		}
		else{
			err_push("%s", jerr.jpegLastErrorMsg);
		}
		if( img ) g2d_free((g2dImage_s*)img);
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return NULL;
	}
	
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	
	dbg_info("output_width: %d", cinfo.output_width);
	dbg_info("output_components: %d", cinfo.output_components);
	dbg_info("output_height: %d", cinfo.output_height);

	img = g2d_new(cinfo.output_width, cinfo.output_height, -1);
	
	const unsigned w = cinfo.output_width;
	const unsigned h = cinfo.output_height;
	const unsigned channel = cinfo.output_components;
	const unsigned size = w * channel;
	__mem_free unsigned char* buf = malloc(size);
	unsigned y = 0;
	if(channel == 4 ){
		while (cinfo.output_scanline < h) {
			jpeg_read_scanlines(&cinfo, &buf, 1);
			unsigned const row = g2d_row(img, y);
			g2dColor_t* pix = g2d_color(img, row, 0);
			for( size_t x = 0; x < img->w; ++x){
				pix[x] = g2d_color_make(img, buf[x*channel+4], buf[x*channel], buf[x*channel+1], buf[x*channel+2]);
			}
			++y;
		}
	}
	else{
		while (cinfo.output_scanline < h) {
			jpeg_read_scanlines(&cinfo, &buf, 1);
			unsigned const row = g2d_row(img, y);
			g2dColor_t* pix = g2d_color(img, row, 0);
			for( size_t x = 0; x < img->w; ++x){
				pix[x] = g2d_color_make(img, 255, buf[x*channel], buf[x*channel+1], buf[x*channel+2]);
			}
			++y;
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	return (g2dImage_s*)img;
}

