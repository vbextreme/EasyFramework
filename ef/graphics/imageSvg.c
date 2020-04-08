#include <ef/image.h>
#include <ef/memory.h>
#include <ef/file.h>
#include <ef/err.h>
#include <librsvg-2.0/librsvg/rsvg.h>

//only for ycm
#ifdef __clang__
	#include <cairo/cairo.h>
	int rsvg_handle_render_cairo(RsvgHandle *handle, cairo_t *cr);
#endif

__private unsigned char* svg_file_load(size_t* size, const char* path){
	int fd = fd_open(path, "r", 0);
	if( fd < 0 ){
		err_push("load %s", path);
		return NULL;
	}
	unsigned char* svgfile = fd_slurp(size, fd, 4096, 1);
	if( !svgfile ){
		err_push("reading %s", path);
		return NULL;
	}
	fd_close(fd);
	return svgfile;
}

g2dImage_s* g2d_load_svg(char const* path, unsigned width, unsigned height){
	dbg_info("load svg %s", path);

    RsvgHandle *handle;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;
    
	size_t svgsize = 0;
	__mem_free unsigned char* memblock = svg_file_load(&svgsize, path);
	if( !memblock ) return NULL;

    //rsvg_set_default_dpi(72.0);
    handle = rsvg_handle_new_from_data(memblock, svgsize, NULL);
    if( !handle ){
        err_push("rsvg_handle_new_from_file");
        return NULL;
    }

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    cr = cairo_create(surface);

    rsvg_handle_render_cairo(handle, cr);

    status = cairo_status(cr);
    if (status){
        err_push("cairo_status!\n");
        err_push("%s", cairo_status_to_string(status));
        return NULL;
    }

    size_t stride = cairo_image_surface_get_stride(surface);
    unsigned char* buffer = cairo_image_surface_get_data(surface);
	cairo_surface_flush(surface);
	g2dImage_s* img = g2d_new(width, height, -1);
	
	for( size_t y = 0; y < height; ++y){
		unsigned imgRow = g2d_row(img, y);
		g2dColor_t* imgPix = g2d_color(img, imgRow, 0);
		unsigned dataRow = y * stride;
		g2dColor_t* dataPix = (g2dColor_t*)&buffer[dataRow];
		for( size_t x = 0; x < width; ++x){
			imgPix[x] = dataPix[x];
		}
	}
   
   	cairo_destroy (cr);
    cairo_surface_destroy (surface);
	
	return img;
}

