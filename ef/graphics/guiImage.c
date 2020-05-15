#include <ef/gui.h>
#include <ef/guiImage.h>
#include <ef/imagePng.h>
#include <ef/imageJpeg.h>
#include <ef/imageBmp.h>
#include <ef/imageSvg.h>
#include <ef/guiResources.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/file.h>
#include <ef/str.h>
#include <ef/vector.h>

typedef struct guiLayerTimer{
	gui_s* gui;
	guiComposite_s* cmp;
	unsigned id;
	unsigned count;
}guiLayerTimer_s;

guiLayer_s* gui_layer_color_new(g2dColor_t color, unsigned width, unsigned height, unsigned flags){
	guiLayer_s* img = mem_new(guiLayer_s);
	if( !img ) err_fail("eom");
	img->pos.x = 0;
	img->pos.y = 0;
	img->pos.w = width;
	img->pos.h = height;
	img->src = img->pos;
	img->flags = flags;
	img->color = color;
	img->type = GUI_LAYER_COLOR;
	img->flags = flags;
	img->res = NULL;
	img->free = NULL;
	return img;
}

guiLayer_s* gui_layer_fn_new(guiLayerFN_f fn, void* data, guiLayerFree_f freefn, unsigned width, unsigned height, unsigned flags){
	guiLayer_s* img = mem_new(guiLayer_s);
	if( !img ) err_fail("eom");
	img->pos.x = 0;
	img->pos.y = 0;
	img->pos.w = width;
	img->pos.h = height;
	img->src = img->pos;
	img->type = GUI_LAYER_FN;
	img->flags = flags;
	img->data = data;
	img->fn = fn;
	img->free = freefn;
	img->res = NULL;
	return img;
}

guiLayer_s* gui_layer_custom_new(g2dImage_s* g2d, unsigned flags){
	guiLayer_s* img = mem_new(guiLayer_s);
	if( !img ) err_fail("eom");
	img->pos.x = 0;
	img->pos.y = 0;
	img->pos.w = g2d->w;
	img->pos.h = g2d->h;
	img->src = img->pos;
	img->type = GUI_LAYER_CUSTOM;
	img->flags = flags;
	img->img = g2d;
	img->res = NULL;
	img->free = NULL;
	return img;
}

__private guiLayer_s* gui_layer_color_set(guiLayer_s* img, g2dColor_t color, unsigned width, unsigned height){
	img->color = color;
	img->type = GUI_LAYER_COLOR;
	img->src.w = img->pos.w = width;
	img->src.h = img->pos.h = height;
	if( img->res ) free(img->res);
	img->res = NULL;
	return img;
}

__private guiLayer_s* gui_layer_image_set(guiLayer_s* img, unsigned width, unsigned height, int ratio){
	img->type = GUI_LAYER_IMG;
	g2d_ratio(ratio, img->img->w, img->img->h, &width, &height); 
	g2dImage_s* scaled = g2d_resize(img->img, width, height);
	if( !scaled ) err_fail("scaled image");
	img->img = scaled;
	img->src.w = img->pos.w = img->img->w;
	img->src.h = img->pos.h = img->img->h;
	return img;
}

__private guiLayer_s* gui_layer_svg_set(guiLayer_s* img, svg_s* svg, unsigned width, unsigned height){
	img->type = GUI_LAYER_SVG;
	g2dImage_s* render = svg_render(svg, width, height);
	if( !render ) err_fail("render image");
	img->img = render;
	img->src.w = img->pos.w = img->img->w;
	img->src.h = img->pos.h = img->img->h;
	return img;
}

__private guiLayer_s* gui_layer_gif_set(guiLayer_s* img, unsigned width, unsigned height, int ratio){
	img->type = GUI_LAYER_GIF;
	if( width != img->img->w || height != img->img->h ){
		g2d_gif_resize(img->gif, width, height, ratio);
	}
	img->src.w = img->pos.w = img->gif->width;
	img->src.h = img->pos.h = img->gif->height;
	img->src = img->pos;
	return img;
}

__private guiLayer_s* gui_layer_media_set(guiLayer_s* img, unsigned width, unsigned height){
	img->type = GUI_LAYER_VIDEO;
	img->src.w = img->pos.w = width;
	img->src.h = img->pos.h = height;
	free(img->res);
	img->res = NULL;
	return img;
}

guiLayer_s* gui_layer_new(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio){
	if( !width || !height ){
		dbg_error("wh");	
		return NULL;
	}

	guiLayer_s* img = mem_new(guiLayer_s);
	img->free = NULL;
	img->src.x = img->pos.x = 0;
	img->src.y = img->pos.y = 0;
	img->flags = flags;
	img->res = NULL;
	if( !pathRelative ){
		return	gui_layer_color_set(img, color, width, height);
	}

	img->res = path_resolve(pathRelative);
	dbg_info("path:%s",img->res);
	if( !file_exists(img->res) ){
		dbg_error("file not exists");
		return gui_layer_color_set(img, color, width, height);
	}

   	img->img = g2d_load_png(img->res);
	if( img->img ){
		gui_resource_new(img->res, img->img);
		gui_layer_image_set(img, width, height, ratio);
		return img;
	}

	img->img = g2d_load_jpeg(img->res);
	if( img->img ){
		gui_resource_new(img->res, img->img);
		gui_layer_image_set(img, width, height, ratio);
		return img;
	}
	
	img->img = g2d_load_bmp(img->res);
	if( img->img ){
		gui_resource_new(img->res, img->img);
		gui_layer_image_set(img, width, height, ratio);	
		return img;
	}
	
	svg_s* svg = svg_load(img->res);
	if( svg ){
		gui_resource_new(img->res, svg);
		return gui_layer_svg_set(img, svg, width, height);
	}

	img->gif = g2d_load_gif(img->res);
	if( img->gif ){
		gui_layer_gif_set(img, width, height, ratio);
		return img;
	}

	img->video = media_load(img->res);
	if( img->video ){
		gui_layer_media_set(img, width, height);
		return img;
	}
	
	dbg_error("unknow format");
	return gui_layer_color_set(img, color, width, height);
}

guiLayer_s* gui_layer_load(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio){
	if( !width || !height ){
		dbg_error("fail, no width || height");
		return NULL;
	}
	if( !pathRelative ){
		dbg_error("no realtive path");	
		NULL;
	}

	char* path = path_resolve(pathRelative);
	guiResource_s* res = gui_resource(path);

	if( res ){
		switch( res->type ){
			default: 
			case GUI_RESOURCE_COLOR:
			case GUI_RESOURCE_MEDIA:
			case GUI_RESOURCE_FONTS:
			case GUI_RESOURCE_UTF:
			case GUI_RESOURCE_LONG:
			case GUI_RESOURCE_DOUBLE:
			case GUI_RESOURCE_TEXT:
			case GUI_RESOURCE_POSITION:
			case GUI_RESOURCE_GIF:
				err_fail("wrong resources"); 
			break;
			
			case GUI_RESOURCE_SVG:{
				guiLayer_s* img = mem_new(guiLayer_s);
				if( !img ) err_fail("eom");
				img->res = path;
				img->src.x = img->pos.x = 0;
				img->src.y = img->pos.y = 0;
				img->flags = flags;
			return gui_layer_svg_set(img, res->svg, width, height);
			}
			break;

			case GUI_RESOURCE_IMG:{
				guiLayer_s* img = mem_new(guiLayer_s);
				if( !img ) err_fail("eom");
				img->res = path;
				img->src.x = img->pos.x = 0;
				img->src.y = img->pos.y = 0;
				img->flags = flags;
				img->img = res->img;
			return gui_layer_image_set(img, width, height, ratio);
			}
		}
	}
	
	free(path);
	return gui_layer_new(color, pathRelative, width, height, flags, ratio);
}

void gui_layer_free(guiLayer_s* img){
	if( !img ) return;	
	if( img->free ){
		img->free(img->data);
	}

	if( img->res ){
		dbg_info("release:'%s'", img->res);
		gui_resource_release(img->res);
	}
	else{
		switch( img->type ){
			default:
			case GUI_LAYER_COLOR:
			case GUI_LAYER_FN:
			break;

			case GUI_LAYER_CUSTOM:
			case GUI_LAYER_IMG:
			case GUI_LAYER_SVG:
				g2d_free(img->img);
			break;

			case GUI_LAYER_GIF:
				g2d_gif_free(img->gif);
			break;

			case GUI_LAYER_VIDEO:
				media_free(img->video);
			break;
		}
	}
	free(img);
}

void gui_layer_resize(gui_s* gui, guiLayer_s* img, unsigned width, unsigned height, int ratio){
	if( !width || !height ) return;

	dbg_info("%s: %u*%u", gui->name, width, height);

	if( img->flags & GUI_LAYER_FLAGS_PERC ){
		dbg_info("\t%u*%u per %f %f", width, height, img->per.w, img->per.h);
		img->pos.x = (width * img->per.x) / 100.0;
 		img->pos.y = (height * img->per.y) / 100.0;
		width = (width * img->per.w) / 100.0;
		height = (height * img->per.h) / 100.0;
		dbg_info("\tresize: %u %u %u*%u", img->pos.x, img->pos.y, width, height); 
	}

	switch( img->type ){
		default:
		case GUI_LAYER_COLOR:
		case GUI_LAYER_FN:
			img->pos.w = width;
			img->pos.h = height;
			img->src = img->pos;
		break;

		case GUI_LAYER_CUSTOM: break;

		case GUI_LAYER_SVG:{
			g2d_free(img->img);
			guiResource_s* res = gui_resource(img->res);
			if( !res ) err_fail("resize image %s", img->res);
			iassert(res->type == GUI_RESOURCE_SVG);
			gui_layer_svg_set(img, res->svg, width, height);
		}
		break;

		case GUI_LAYER_IMG:{
			g2d_free(img->img);
			guiResource_s* res = gui_resource(img->res);
			if( !res ) err_fail("resize image %s", img->res);
			iassert( res->type == GUI_RESOURCE_IMG );
			img->img = res->img;
			gui_layer_image_set(img, width, height, ratio);
		}
		break;
	
		case GUI_LAYER_GIF:
			g2d_gif_resize(img->gif, width, height, ratio);
		break;

		case GUI_LAYER_VIDEO:
			media_resize_set(img->video, gui->surface->img);
		break;
	}
}

void gui_layer_xy_set(guiLayer_s* img, unsigned x, unsigned y){
	img->pos.x = x;
	img->pos.y = y;
}

void gui_layer_src_xy_set(guiLayer_s* img, unsigned x, unsigned y){
	img->src.x = x;
	img->src.y = y;
}

void gui_layer_wh_set(guiLayer_s* img, unsigned w, unsigned h){
	img->pos.w = img->src.w = w;
	img->pos.h = img->src.h = h;
}

void gui_layer_perc_set(guiLayer_s* img, double x, double y, double w, double h){
	img->flags |= GUI_LAYER_FLAGS_PERC;
	img->per.w = w;
	img->per.h = h;
	img->per.x = x;
 	img->per.y = y;
}

__private void gid_color(gui_s* gui, guiLayer_s* img){
	iassert( img->type == GUI_LAYER_COLOR);
	if( img->pos.x + img->pos.w > gui->surface->img->w ){
		dbg_warning("try to draw offscreen x:%u w:%u", img->pos.x, img->pos.w);
		img->pos.w = gui->surface->img->w - img->pos.x;
	}
	if( img->pos.y + img->pos.h > gui->surface->img->h ){
		dbg_warning("try to draw offscreen y:%u h:%u", img->pos.y, img->pos.h);
		img->pos.h = gui->surface->img->h - img->pos.y;
	}
	g2d_clear(gui->surface->img, img->color, &img->pos);
}

__private void gid_img(gui_s* gui, g2dCoord_s* pos, g2dImage_s* img, g2dCoord_s* src, unsigned flags){
	if( pos->x + pos->w > gui->surface->img->w ) pos->w = gui->surface->img->w - pos->x;
	if( pos->y + pos->h > gui->surface->img->h ) pos->h = gui->surface->img->h - pos->y;
	src->w = pos->w;
	src->h = pos->h;
	if( flags & GUI_LAYER_FLAGS_ALPHA ){
		g2d_bitblt_alpha(gui->surface->img, pos, img, src);
	}
	else{
		g2d_bitblt(gui->surface->img, pos, img, src);
	}
}

__private int gid_gif_timer(guiTimer_s* t){
	guiLayerTimer_s* git = t->userdata;
	guiLayer_s* img = git->cmp->layers[git->id];

	gid_img(t->gui, &img->pos, img->gif->frames[img->frameid].img, &img->gif->frames[img->frameid].pos, img->flags);

	for( unsigned i = git->id + 1; i < git->count; ++i){
		gui_layer_redraw(git->gui, git->cmp, i, git->count);
	}

	if( !(img->flags & GUI_LAYER_FLAGS_PLAY) ){
		free(t->userdata); 
		return GUI_TIMER_FREE;
	}

	const long delay = img->gif->frames[img->frameid].delay == 0 ? 1 : img->gif->frames[img->frameid].delay;
	
	++img->frameid;
	if( img->frameid >= vector_count(img->gif->frames) ){
		img->frameid = 0;
		if( !(img->flags & GUI_LAYER_FLAGS_LOOP) ){
			img->flags &= ~GUI_LAYER_FLAGS_PLAY;
			free(git);
			return GUI_TIMER_FREE;
		}
	}
	gui_timer_change(t, delay);
	return GUI_TIMER_CUSTOM;
}

__private void gid_gif(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count){
	guiLayer_s* img = cmp->layers[id];

	if( img->flags & GUI_LAYER_FLAGS_PLAY ){
		gid_img(gui, &img->pos, img->gif->frames[img->frameid].img, &img->gif->frames[img->frameid].pos, img->flags);
		return;
	}

	img->frameid = 0;
	img->flags |= GUI_LAYER_FLAGS_PLAY;
	guiLayerTimer_s* git = mem_new(guiLayerTimer_s);
	git->gui = gui;
	git->cmp = cmp;
	git->id = id;
	git->count = count;
	gui_timer_new(gui, 1, gid_gif_timer, git);
}

__private int gid_media_timer(guiTimer_s* t){
	guiLayerTimer_s* git = t->userdata;
	guiLayer_s* img = git->cmp->layers[git->id];

	int ret;
	while( (ret=media_decode(img->video)) == 0 );
	long delay = media_delay_get(img->video);
	delay = delay < 1000 ? 1 : delay/1000;

	if( ret == - 1){
		if( !(img->flags & GUI_LAYER_FLAGS_LOOP) ){
			img->flags &= ~GUI_LAYER_FLAGS_PLAY;
			free(git);
			return GUI_TIMER_FREE;
		}
		media_seek(img->video, 0);
	}

	for( unsigned i = git->id + 1; i < git->count; ++i){
		gui_layer_redraw(git->gui, git->cmp, i, git->count);
	}

	gui_timer_change(t, delay);
	return GUI_TIMER_CUSTOM;
}

__private void gid_media(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count){
	guiLayer_s* img = cmp->layers[id];

	if( img->flags & GUI_LAYER_FLAGS_PLAY ){
		media_redraw(img->video);
		return;
	}
	
	img->flags |= GUI_LAYER_FLAGS_PLAY;
	media_resize_set(img->video, gui->surface->img);
	img->src.w = gui->surface->img->w;
	img->src.h = gui->surface->img->h;
	guiLayerTimer_s* git = mem_new(guiLayerTimer_s);	
	git->gui = gui;
	git->cmp = cmp;
	git->id = id;
	git->count = count;
	gui_timer_new(gui, 1, gid_media_timer, git);
}

inline __private void gid_fn(gui_s* gui, guiLayer_s** img){
	(*img)->fn(gui, img, (*img)->data);
}

void gui_layer_redraw(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count){
	switch( cmp->layers[id]->type ){
		case GUI_LAYER_COLOR: gid_color(gui, cmp->layers[id]); break;
		case GUI_LAYER_CUSTOM:
		case GUI_LAYER_SVG:
		case GUI_LAYER_IMG:   gid_img(gui, &cmp->layers[id]->pos, cmp->layers[id]->img, &cmp->layers[id]->src, cmp->layers[id]->flags); break;
		case GUI_LAYER_GIF:   gid_gif(gui, cmp, id, count); break;
		case GUI_LAYER_VIDEO: gid_media(gui, cmp, id, count); break;
		case GUI_LAYER_FN:    gid_fn(gui, &cmp->layers[id]); break;
	}
}

guiComposite_s* gui_composite_new(unsigned count){
	guiComposite_s* cmp = mem_new(guiComposite_s);
	if( !cmp ) err_fail("eom");
	cmp->flags = 0;
	cmp->layers = vector_new(guiLayer_s*, count, (vfree_f)gui_layer_free);
	return cmp;
}

void gui_composite_free(guiComposite_s* cmp){
	vector_free(cmp->layers);
	free(cmp);
}

guiComposite_s* gui_composite_add(guiComposite_s* cmp, guiLayer_s* layer){
	vector_push_back(cmp->layers, layer);
	return cmp;
}

void gui_composite_remove(guiComposite_s* cmp, size_t id){
	if( id >= vector_count(cmp->layers) ) return;	
	vector_remove(cmp->layers, id);
}

void gui_composite_replace(guiComposite_s* cmp, guiLayer_s* oldl, guiLayer_s* newl){
	vector_foreach(cmp->layers, i){
		if( cmp->layers[i] == oldl ){
			gui_layer_free(oldl);
			cmp->layers[i] = newl;
			return;
		}
	}
	dbg_error("layers not exists");
}

void gui_composite_redraw(gui_s* gui, guiComposite_s* cmp){
	if( !cmp ) return;
	vector_foreach(cmp->layers, i){
		gui_layer_redraw(gui, cmp, i, vector_count(cmp->layers));
	}
}

void gui_composite_resize(gui_s* gui, guiComposite_s* cmp, unsigned width, unsigned height){
	vector_foreach(cmp->layers, i){
		gui_layer_resize(gui, cmp->layers[i], width, height , -1);
	}
}
