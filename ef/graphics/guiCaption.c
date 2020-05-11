#include <ef/guiCaption.h>
#include <ef/guiResources.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/ft.h>
#include <ef/err.h>

guiCaption_s* gui_caption_new(ftFonts_s* font, g2dColor_t foreground, unsigned flags){
	guiCaption_s* cap = mem_new(guiCaption_s);
	if( !cap ) err_fail("eom");
	cap->text = NULL;
	cap->textWidth = 0;
	cap->textHeight = 0;
	cap->fonts = font;
	cap->foreground = foreground;
	cap->render =  gui_image_custom_new(
		g2d_new(100, 100, -1), 
		GUI_IMAGE_FLAGS_ALPHA
	);
	cap->flags = flags;
	cap->scroll.x = 0;
	cap->scroll.y = 0;
	return cap;
}

void gui_caption_free(guiCaption_s* lbl){
	if( lbl->text ) free(lbl->text);
	free(lbl);
}

void gui_caption_render_new(guiCaption_s* cap){
	cap->render =  gui_image_custom_new(
		g2d_new(100, 100, -1), 
		GUI_IMAGE_FLAGS_ALPHA
	);
}

void gui_caption_text_set(gui_s* gui, guiCaption_s* cap, const utf8_t* text){
	if( cap->text ) free(cap->text);
	cap->text = (utf8_t*)str_dup((const char*)text, 0);
	if( cap->flags & GUI_CAPTION_AUTOWRAP ){
		cap->textWidth = gui->surface->img->w;
		cap->textHeight =  ft_autowrap_height(cap->fonts, cap->text, cap->textWidth);
	}
	else{
		cap->textHeight = ft_multiline_height(cap->fonts, cap->text);
		cap->textWidth  = ft_multiline_lenght(cap->fonts, cap->text);
	}
	cap->flags |= GUI_CAPTION_RENDERING;
}

void gui_caption_render(gui_s* gui, guiCaption_s* cap){
	if( !(cap->flags & GUI_CAPTION_RENDERING) ) return;

	if( !cap->text ) return;
	cap->flags &= ~GUI_CAPTION_RENDERING;
	
	if( cap->render->img->w < cap->textWidth || cap->render->img->h < cap->textHeight ){
		g2d_free(cap->render->img);
		cap->render->img = g2d_new(cap->textWidth, cap->textHeight, -1);
	}

	g2dCoord_s pen = {
		.x = 0,
		.y = 0,
		.w = cap->textWidth,
		.h = cap->textHeight
	};
	g2d_clear(cap->render->img, gui_color( 0, 255, 255, 255), &pen);

	if( cap->flags & GUI_CAPTION_AUTOWRAP ){
		g2d_string_autowrap(cap->render->img, &pen, cap->fonts, cap->text, cap->foreground, pen.x, 1);
	}
	else{
		const utf8_t* txt = cap->text;
		while( (txt=g2d_string(cap->render->img, &pen, cap->fonts, txt, cap->foreground, pen.x,1)) );
	}

	unsigned x = cap->scroll.x;
	unsigned y = cap->scroll.y;
	if( cap->flags & GUI_CAPTION_CENTER_X ){
		x = abs((int)gui->surface->img->w / 2 - (int)cap->textWidth / 2);
	}
	if( cap->flags & GUI_CAPTION_CENTER_Y ){
		y = abs((int)gui->surface->img->h / 2 - (int)cap->textHeight / 2);
	}
	cap->render->pos.x = x;
	cap->render->pos.y = y;
	cap->render->src.x = 0;
	cap->render->src.y = 0;
	cap->render->pos.w = cap->render->src.w = cap->textWidth;
	cap->render->pos.h = cap->render->src.h = cap->textHeight;
}

void gui_caption_scroll(gui_s* gui, guiCaption_s* cap, unsigned x, unsigned y){
	if( x < gui->surface->img->w - 5 ) cap->scroll.x = x;
	if( y < gui->surface->img->h - 5 ) cap->scroll.y = y;
}

int gui_caption_themes(gui_s* gui, guiCaption_s* cap, const char* name){
	dbg_info("caption themes name:%s", name);

	int vbool = 0;
	if( !gui_themes_bool_set(name, GUI_THEME_CAPTION_CENTER_X, &vbool) ){
		if( vbool ) cap->flags |= GUI_CAPTION_CENTER_X;
		else cap->flags &= ~GUI_CAPTION_CENTER_X;
		cap->flags |= GUI_CAPTION_RENDERING;
	}

	if( !gui_themes_bool_set(name, GUI_THEME_CAPTION_CENTER_Y, &vbool) ){
		if( vbool ) cap->flags |= GUI_CAPTION_CENTER_Y;
		else cap->flags &= ~GUI_CAPTION_CENTER_Y;
		cap->flags |= GUI_CAPTION_RENDERING;
	}

	if( !gui_themes_bool_set(name, GUI_THEME_CAPTION_AUTOWRAP, &vbool) ){
		if( vbool ) cap->flags |= GUI_CAPTION_AUTOWRAP;
		else cap->flags &= ~GUI_CAPTION_AUTOWRAP;
		cap->flags |= GUI_CAPTION_RENDERING;
	}

	if( !gui_themes_uint_set(name, GUI_THEME_FOREGROUND, &cap->foreground) ) cap->flags |= GUI_CAPTION_RENDERING;
	
	ftFonts_s* old = cap->fonts;
	__mem_free char* capfont = str_printf("%s.caption", name);
	gui_themes_fonts_set(capfont, &cap->fonts);
	if( cap->fonts != old ){
		gui_resource_release(old->groupName);
		cap->flags |= GUI_CAPTION_RENDERING;
	}

	__mem_free char* caption = gui_themes_string(name, GUI_THEME_CAPTION);
	if( caption ) gui_caption_text_set(gui, cap, (utf8_t*)caption);

	return 0;
}

