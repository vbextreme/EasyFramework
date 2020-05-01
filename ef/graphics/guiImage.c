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

typedef struct guiImageTimer{
	gui_s* gui;
	guiComposite_s* cmp;
	unsigned id;
	unsigned count;
}guiImageTimer_s;

guiImage_s* gui_image_color_new(g2dColor_t color, unsigned width, unsigned height, unsigned flags){
	guiImage_s* img = mem_new(guiImage_s);
	if( !img ) err_fail("eom");
	img->pos.x = 0;
	img->pos.y = 0;
	img->pos.w = width;
	img->pos.h = height;
	img->src = img->pos;
	img->flags = flags;
	img->color = color;
	img->type = GUI_IMAGE_COLOR;
	img->flags = flags;
	img->res = NULL;
	return img;
}

guiImage_s* gui_image_fn_new(guiImageFN_f fn, void* data, unsigned width, unsigned height, unsigned flags){
	guiImage_s* img = mem_new(guiImage_s);
	if( !img ) err_fail("eom");
	img->pos.x = 0;
	img->pos.y = 0;
	img->pos.w = width;
	img->pos.h = height;
	img->src = img->pos;
	img->type = GUI_IMAGE_FN;
	img->flags = flags;
	img->data = data;
	img->fn = fn;
	img->res = NULL;
	return img;
}

guiImage_s* gui_image_custom_new(g2dImage_s* g2d, unsigned flags){
	guiImage_s* img = mem_new(guiImage_s);
	if( !img ) err_fail("eom");
	img->pos.x = 0;
	img->pos.y = 0;
	img->pos.w = g2d->w;
	img->pos.h = g2d->h;
	img->src = img->pos;
	img->type = GUI_IMAGE_CUSTOM;
	img->flags = flags;
	img->img = g2d;
	img->res = NULL;
	return img;
}

__private guiImage_s* gui_image_color_set(guiImage_s* img, g2dColor_t color, unsigned width, unsigned height){
	img->color = color;
	img->type = GUI_IMAGE_COLOR;
	img->pos.w = width;
	img->pos.h = height;
	img->src = img->pos;
	if( img->res ) free(img->res);
	img->res = NULL;
	return img;
}

__private guiImage_s* gui_image_image_set(guiImage_s* img, unsigned width, unsigned height, int ratio){
	img->type = GUI_IMAGE_IMG;
	if( width != img->img->w || height != img->img->h ){
		g2d_ratio(ratio, img->img->w, img->img->h, &width, &height); 
		g2dImage_s* scaled = g2d_resize(img->img, width, height);
		g2d_free(img->img);
		img->img = scaled;
	}
	img->pos.w = img->img->w;
	img->pos.h = img->img->h;
	img->src = img->pos;
	return img;
}

__private guiImage_s* gui_image_gif_set(guiImage_s* img, unsigned width, unsigned height, int ratio){
	img->type = GUI_IMAGE_GIF;
	if( width != img->img->w || height != img->img->h ){
		g2d_gif_resize(img->gif, width, height, ratio);
	}
	img->pos.w = img->gif->width;
	img->pos.h = img->gif->height;
	img->src = img->pos;
	return img;
}

__private guiImage_s* gui_image_media_set(guiImage_s* img, unsigned width, unsigned height){
	img->type = GUI_IMAGE_VIDEO;
	img->pos.w = width;
	img->pos.h = height;
	img->src = img->pos;
	free(img->res);
	img->res = NULL;
	return img;
}

guiImage_s* gui_image_new(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio){
	if( !width || !height ) return NULL;

	guiImage_s* img = mem_new(guiImage_s);
	img->pos.x = 0;
	img->pos.y = 0;
	img->flags = flags;
	img->res = NULL;
	if( !pathRelative ) return gui_image_color_set(img, color, width, height);

	__mem_free char* path = path_resolve(pathRelative);
	if( !file_exists(path) ) return gui_image_color_set(img, color, width, height);

	img->res = str_printf("%u*%u::%s", width, height, path);
	if( strlen(img->res) > PATH_MAX-1 ) err_fail("name too long");

   	img->img = g2d_load_png(path);
	if( img->img ){
		gui_image_image_set(img, width, height, ratio);
		gui_resource_new(img->res, img->img);
		return img;
	}

	img->img = g2d_load_jpeg(path);
	if( img->img ){
		gui_image_image_set(img, width, height, ratio);
		gui_resource_new(img->res, img->img);
		return img;
	}
	
	img->img = g2d_load_bmp(path);
	if( img->img ){
		gui_image_image_set(img, width, height, ratio);	
		gui_resource_new(img->res, img->img);
		return img;
	}
	
	img->img = g2d_load_svg(path, width, height);
	if( img->img ){
		gui_image_image_set(img, width, height, ratio);
		gui_resource_new(img->res, img->img);
		return img;
	}

	img->gif = g2d_load_gif(path);
	if( img->gif ){
		gui_image_gif_set(img, width, height, ratio);
		gui_resource_new(img->res, img->gif);
		return img;
	}

	img->video = media_load(path);
	if( img->video ){
		gui_image_media_set(img, width, height);
		return img;
	}

	return gui_image_color_set(img, color, width, height);
}

guiImage_s* gui_image_load(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio){
	if( !width || !height ) return NULL;
	if( !pathRelative ) NULL;

	__mem_free char* path = path_resolve(pathRelative);
	char* name = str_printf("%u*%u::%s", width, height, path);
	if( !name ) err_fail("eom");
	guiImage_s* img = mem_new(guiImage_s);
	if( !img ) err_fail("eom");
	img->pos.x = 0;
	img->pos.y = 0;
	img->flags = flags;

	guiResource_s* res = gui_resource(name);
	if( res ){
		switch( res->type ){
			default: 
			case GUI_RESOURCE_COLOR:
			case GUI_RESOURCE_MEDIA:
			case GUI_RESOURCE_FONTS:
			case GUI_RESOURCE_UTF:
			case GUI_RESOURCE_LONG:
			case GUI_RESOURCE_DOUBLE:
			case GUI_RESOURCE_POSITION:
			case GUI_RESOURCE_TEXT:
				err_fail("wrong resources"); 
			break;
			
			case GUI_RESOURCE_IMG:
				img->img = res->img;
			return gui_image_image_set(img, width, height, ratio);

			case GUI_RESOURCE_GIF:
				img->gif = res->gif;
			return gui_image_gif_set(img, width, height, ratio);	
		}
	}
	
	return gui_image_new(color, pathRelative, width, height, flags, ratio);
}

void gui_image_free(guiImage_s* img){
	if( img->res ){
		gui_resource_release(img->res);
		free(img->res);	
	}
	else{
		switch( img->type ){
			default:
			case GUI_IMAGE_COLOR:
			case GUI_IMAGE_FN:
			break;

			case GUI_IMAGE_CUSTOM:
			case GUI_IMAGE_IMG:
				g2d_free(img->img);
			break;

			case GUI_IMAGE_GIF:
				g2d_gif_free(img->gif);
			break;

			case GUI_IMAGE_VIDEO:
				media_free(img->video);
			break;
		}
	}
	free(img);
}

void gui_image_xy_set(guiImage_s* img, unsigned x, unsigned y){
	img->pos.x = x;
	img->pos.y = y;
}

void gui_image_src_xy_set(guiImage_s* img, unsigned x, unsigned y){
	img->src.x = x;
	img->src.y = y;
}

void gui_image_wh_set(guiImage_s* img, unsigned w, unsigned h){
	img->pos.w = img->src.w = w;
	img->pos.h = img->src.h = h;
}

__private void gid_color(gui_s* gui, guiImage_s* img){
	iassert( img->type == GUI_IMAGE_COLOR);
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
	if( flags & GUI_IMAGE_FLAGS_ALPHA ){
		g2d_bitblt_alpha(gui->surface->img, pos, img, src);
	}
	else{
		g2d_bitblt(gui->surface->img, pos, img, src);
	}
}

__private int gid_gif_timer(guiTimer_s* t){
	guiImageTimer_s* git = t->userdata;
	guiImage_s* img = git->cmp->img[git->id];

	gid_img(t->gui, &img->pos, img->gif->frames[img->frameid].img, &img->gif->frames[img->frameid].pos, img->flags);

	for( unsigned i = git->id + 1; i < git->count; ++i){
		gui_image_redraw(git->gui, git->cmp, i, git->count);
	}

	if( !(img->flags & GUI_IMAGE_FLAGS_PLAY) ){
		free(t->userdata); 
		return GUI_TIMER_FREE;
	}

	const long delay = img->gif->frames[img->frameid].delay == 0 ? 1 : img->gif->frames[img->frameid].delay;
	
	++img->frameid;
	if( img->frameid >= vector_count(img->gif->frames) ){
		img->frameid = 0;
		if( !(img->flags & GUI_IMAGE_FLAGS_LOOP) ){
			img->flags &= ~GUI_IMAGE_FLAGS_PLAY;
			free(git);
			return GUI_TIMER_FREE;
		}
	}
	gui_timer_change(t, delay);
	return GUI_TIMER_CUSTOM;
}

__private void gid_gif(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count){
	guiImage_s* img = cmp->img[id];

	if( img->flags & GUI_IMAGE_FLAGS_PLAY ){
		gid_img(gui, &img->pos, img->gif->frames[img->frameid].img, &img->gif->frames[img->frameid].pos, img->flags);
		return;
	}

	img->frameid = 0;
	img->flags |= GUI_IMAGE_FLAGS_PLAY;
	guiImageTimer_s* git = mem_new(guiImageTimer_s);
	git->gui = gui;
	git->cmp = cmp;
	git->id = id;
	git->count = count;
	gui_timer_new(gui, 1, gid_gif_timer, git);
}

__private int gid_media_timer(guiTimer_s* t){
	guiImageTimer_s* git = t->userdata;
	guiImage_s* img = git->cmp->img[git->id];

	int ret;
	while( (ret=media_decode(img->video)) == 0 );
	long delay = media_delay_get(img->video);
	delay = delay < 1000 ? 1 : delay/1000;

	if( ret == - 1){
		if( !(img->flags & GUI_IMAGE_FLAGS_LOOP) ){
			img->flags &= ~GUI_IMAGE_FLAGS_PLAY;
			free(git);
			return GUI_TIMER_FREE;
		}
		media_seek(img->video, 0);
	}

	for( unsigned i = git->id + 1; i < git->count; ++i){
		gui_image_redraw(git->gui, git->cmp, i, git->count);
	}

	gui_timer_change(t, delay);
	return GUI_TIMER_CUSTOM;
}

__private void gid_media(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count){
	guiImage_s* img = cmp->img[id];

	if( img->flags & GUI_IMAGE_FLAGS_PLAY ){
		media_redraw(img->video);
		return;
	}
	
	img->flags |= GUI_IMAGE_FLAGS_PLAY;
	media_resize_set(img->video, gui->surface->img);
	img->src.w = gui->surface->img->w;
	img->src.h = gui->surface->img->h;
	guiImageTimer_s* git = mem_new(guiImageTimer_s);	
	git->gui = gui;
	git->cmp = cmp;
	git->id = id;
	git->count = count;
	gui_timer_new(gui, 1, gid_media_timer, git);
}

__private void gid_fn(gui_s* gui, guiImage_s* img){
	img->fn(gui, img, img->data);
}

void gui_image_redraw(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count){
	switch( cmp->img[id]->type ){
		case GUI_IMAGE_COLOR: gid_color(gui, cmp->img[id]); break;
		case GUI_IMAGE_CUSTOM:
		case GUI_IMAGE_IMG:   gid_img(gui, &cmp->img[id]->pos, cmp->img[id]->img, &cmp->img[id]->src, cmp->img[id]->flags); break;
		case GUI_IMAGE_GIF:   gid_gif(gui, cmp, id, count); break;
		case GUI_IMAGE_VIDEO: gid_media(gui, cmp, id, count); break;
		case GUI_IMAGE_FN:    gid_fn(gui, cmp->img[id]); break;
	}
}

guiComposite_s* gui_composite_new(unsigned count){
	guiComposite_s* cmp = mem_new(guiComposite_s);
	if( !cmp ) err_fail("eom");
	cmp->flags = 0;
	cmp->img = vector_new(guiImage_s*, count, count);
	return cmp;
}

void gui_composite_free(guiComposite_s* cmp){
	vector_foreach(cmp, i){
		gui_image_free(cmp->img[i]);
	}
	vector_free(cmp->img);
	free(cmp);
}

guiComposite_s* gui_composite_add(guiComposite_s* cmp, guiImage_s* img){
	vector_push_back(cmp->img, img);
	return cmp;
}

void gui_composite_redraw(gui_s* gui, guiComposite_s* cmp){
	vector_foreach(cmp->img, i){
		gui_image_redraw(gui, cmp, i, vector_count(cmp->img));
	}
}


