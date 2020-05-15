#include <ef/guiResources.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/rbhash.h>
#include <ef/err.h>

#define GUI_HASH_SIZE     32
#define GUI_HASH_MIN      10
#define GUI_HASH_KEY_SIZE PATH_MAX

__private rbhash_s* resources;

__private void resource_free(__unused uint32_t hash, __unused const char* name, void* data){
	guiResource_s* res = data;
	switch( res->type ){
		case GUI_RESOURCE_TEXT:     free(res->text);            break;
		case GUI_RESOURCE_IMG:      g2d_free(res->img);         break;
		case GUI_RESOURCE_SVG:      svg_free(res->svg);         break;
		case GUI_RESOURCE_GIF:      g2d_gif_free(res->gif);     break;
		case GUI_RESOURCE_MEDIA:    media_free(res->media);     break;
		case GUI_RESOURCE_FONTS:    ft_fonts_free(res->fonts);  break;
		case GUI_RESOURCE_UTF:      break;
		case GUI_RESOURCE_LONG:     break;
		case GUI_RESOURCE_DOUBLE:   break;
		case GUI_RESOURCE_COLOR:    break;
		case GUI_RESOURCE_POSITION: break;
	}
	free(res);
}

void gui_resources_init(void){
	resources = rbhash_new(GUI_HASH_SIZE, GUI_HASH_MIN, GUI_HASH_KEY_SIZE, hash_fasthash, resource_free);
	if( !resources ) err_fail("rbhash");
}

void gui_resources_free(void){
	rbhash_free(resources);
}

__private guiResource_s* resource_set(guiResource_s* res, const char* name){
	dbg_info("add resource name %s", name);
	guiResource_s* old = rbhash_find(resources, name, 0);
	if( old ){
		dbg_warning("replace resource");
		rbhash_remove(resources, name, 0);
	}
	rbhash_add(resources, name, 0, res);
	return res;
}

__private guiResource_s* resource_new(guiResource_e type){
	guiResource_s* res = mem_new(guiResource_s);
	if( !res ) err_fail("eom");
	res->type = type;
	res->reference = 1;
	return res;
}

guiResource_s* gui_resource_long_new(const char* name, long value){
	guiResource_s* res = resource_new(GUI_RESOURCE_LONG);
	res->l = value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_double_new(const char* name, double value){
	guiResource_s* res = resource_new(GUI_RESOURCE_DOUBLE);
	res->d = value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_utf_new(const char* name, utf_t value){
	guiResource_s* res = resource_new(GUI_RESOURCE_UTF);
	res->utf = value;
	return resource_set(res,name);
}

guiResource_s* gui_resource_text_new(const char* name, const utf8_t* value){
	guiResource_s* res = resource_new(GUI_RESOURCE_TEXT);
	res->text = (utf8_t*)str_dup((const char*)value, 0);
	return resource_set(res, name);
}

guiResource_s* gui_resource_color_new(const char* name, g2dColor_t value){
	guiResource_s* res = resource_new(GUI_RESOURCE_COLOR);
	res->color = value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_position_new(const char* name, g2dCoord_s* value){
	guiResource_s* res = resource_new(GUI_RESOURCE_POSITION);
	res->position = *value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_img_new(const char* name, g2dImage_s* value){
	guiResource_s* res = resource_new(GUI_RESOURCE_IMG);
	res->img = value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_svg_new(const char* name, svg_s* value){
	guiResource_s* res = resource_new(GUI_RESOURCE_SVG);
	res->svg = value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_gif_new(const char* name, gif_s* value){
	guiResource_s* res = resource_new(GUI_RESOURCE_GIF);
	res->gif = value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_media_new(const char* name, media_s* value){
	guiResource_s* res = resource_new(GUI_RESOURCE_MEDIA);
	res->media = value;
	return resource_set(res, name);
}

guiResource_s* gui_resource_fonts_new(const char* name, ftFonts_s* value){
	guiResource_s* res = resource_new(GUI_RESOURCE_FONTS);
	res->fonts = value;
	dbg_info("add fonts resources: %s", res->fonts->groupName);
	return resource_set(res, name);
}

guiResource_s* gui_resource(const char* name){
	if( !name ) return NULL;
	guiResource_s* res = rbhash_find(resources, name, 0);
	if( res ) ++res->reference;
	return res;
}

err_t gui_resource_release(const char* name){
	if( !name ) return -1;
	guiResource_s* res = rbhash_find(resources, name, 0);
	if( res ){
		if( res->reference > 0 ) --res->reference;
		if( !res->reference ){
			dbg_info("no other reference, remove");
			rbhash_remove(resources, name, 0);
		}
		return 0;
	}
	return -1;
}
