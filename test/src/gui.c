#include "test.h"
#include <ef/gui.h>
#include <ef/guiCapton.h>
#include <ef/guiLabel.h>
#include <ef/guiButton.h>
#include <ef/guiText.h>
#include <ef/guiBar.h>
#include <ef/guiImage.h>
#include <ef/guiResources.h>
#include <ef/guiDiv.h>

#include <ef/ft.h>
#include <ef/image.h>
#include <ef/imageFiles.h>
#include <ef/imageGif.h>
#include <ef/media.h>
#include <ef/xorg.h>
#include <ef/os.h>
#include <ef/utf8.h>
#include <ef/delay.h>

typedef struct player{
	gui_s* player;
	gui_s* bar;
	const char* path;
	media_s* video;
	guiTimer_s* timer;
}player_s;

/*@test -g --gui 'test gui'*/

void font_load(ftFonts_s* tfont,const char* name, const char* path,  unsigned size){
	ftFont_s* f = ft_fonts_load(tfont, path, name);
	if( !f ) err_fail("load font %s", path);
	if( ft_font_size(f, size, size) ) err_fail("size %s", name);
}

int main_exit(__unused gui_s* gui, __unused xorgEvent_s* ev){
	return -1;
}

int player_clock_frame(guiTimer_s* timer){
	player_s* p = timer->userdata;
	int ret;
	while( (ret=media_decode(p->video)) == 0 );
	if( ret < 0 ){
		media_free(p->video);
		p->video = NULL;
		return GUI_TIMER_FREE;
	}
	long delay = media_delay_get(p->video);
	delay = delay < 1000 ? 1 : delay / 1000;

	gui_draw(p->player);
	dbg_error("CURRENT TIME: %f", media_time(p->video));
	gui_bar_current_set(p->bar, media_time(p->video));	
	gui_bar_redraw(p->bar);
	gui_draw(p->bar);

	gui_timer_change(p->timer, delay);
	return GUI_TIMER_CUSTOM;
}

int bStart_click(gui_s* gui,__unused xorgEvent_s* ev){
	player_s* p = gui->userdata;
	if( p->video ) return 0;

	__mem_free char* path = path_resolve(p->path);
	dbg_info("open video:%s", path);
	p->video = media_load(path);
	if( !p->video ){
		err_print();
		dbg_error("load video");
		return 0;
	}	
	media_resize_set(p->video, p->player->surface->img);
	p->timer = gui_timer_new(p->player, 1, player_clock_frame, p);
	gui_bar_max_set(p->bar, media_duration(p->video)/1000.0);
	gui_bar_redraw(p->bar);
	gui_draw(p->bar);

	return 0;
}

int bar2_timer(guiTimer_s* timer){
	static int fillmode = 1;
	gui_s* bar2 = timer->gui;
	if( fillmode ){
		double cur = gui_bar_current(bar2);
		cur += 1.0;
		if( cur > 100.0 ){
			gui_bar_current_set(bar2, 100.0);
			fillmode = !fillmode;
		}
		else{
			gui_bar_current_set(bar2, cur);
			gui_redraw(bar2);
			gui_draw(bar2);
			return GUI_TIMER_NEXT;
		}
	}

	if( !fillmode ){
		double min = gui_bar_min(bar2);
		min += 1.0;
		if( min > 100.0 ){
			gui_bar_min_set(bar2, 0.0);
			gui_bar_current_set(bar2, 0.0);
			fillmode = !fillmode;
		}
		else{
			gui_bar_min_set(bar2, min);
		}
		gui_redraw(bar2);
		gui_draw(bar2);
	}

	return GUI_TIMER_NEXT;
}

/*@fn*/
void test_gui(__unused const char* argA, __unused const char* argB){
	err_enable();
	gui_begin();
	ftFonts_s* tfont = ft_fonts_new("defaultThemes");
	font_load(tfont, "master", "Dejavu", 18);
	font_load(tfont, "masterFall", "FiraSans", 18);
	font_load(tfont, "fallback", "Symbola", 18);

	gui_s* main = gui_div_attach(
			gui_new(
				NULL, "test", "window", GUI_MODE_NORMAL,
				0, 50, 50, 600, 600, 
				gui_color(255,0,0,0),
				gui_composite_add(
				gui_composite_new(4),
					gui_image_color_new(
						gui_color(255,125,125,125),
						600,600,
						0
					)
				),
				0,NULL
			),
			gui_div_new(GUI_DIV_TABLE, GUI_DIV_FLAGS_FIT)
	);
	main->destroy = main_exit;
	

/*
	gui_s* main = gui_new(
		NULL, "test", "window", GUI_MODE_NORMAL,
		0, 50, 50, 600, 600, 
		gui_color(255,0,0,0),
		gui_composite_add(
		gui_composite_new(4),
			gui_image_color_new(
				gui_color(255,125,125,125),
				600,600,
				0
			)
		),
		0,NULL
	);
	main->destroy = main_exit;
*/

	gui_s* lbl = 	gui_label_attach(
		gui_new(
			main, "labl", "label", GUI_MODE_NORMAL,
			1, 3, 3, main->position.w - 6, 50,
			gui_color(255,0,0,0),
			gui_composite_add(
				gui_composite_new(4),
				gui_image_color_new(
					gui_color(255, 70, 100, 70),
					main->position.w - 6, 50,
					0
				)
			),
			0, NULL
		),
		gui_label_new(gui_caption_new(tfont, gui_color(255,40,40,40), GUI_CAPTION_CENTER_X | GUI_CAPTION_CENTER_Y))
	);
	gui_label_text_set(lbl, U8("video play"));
	gui_div_table_attach(
			gui_div_table_create_row(main, 12, 1),
			lbl,
			0,
			100,
			-1
	);

	gui_s* player = gui_new(
		main, "player", "window", GUI_MODE_NORMAL,
		1, 3, 56, main->position.w-6, 300, 
		gui_color(255,0,0,0),
		gui_composite_add(
			gui_composite_new(4),
			gui_image_color_new(
				gui_color(255,45,45,45),
				main->position.w-6, 300,
				0
			)
		),
		0,NULL
	);
	gui_div_table_attach(
			gui_div_table_create_row(main, 50, 1),
			player,
			0,
			100,
			-1
	);

	gui_s* bar = gui_bar_attach(
		gui_new(
			main, "bar", "progrssbar", GUI_MODE_NORMAL,
			1, 3, 359, main->position.w-6, 20,
			gui_color(255,0,0,0),
			gui_composite_add(
				gui_composite_new(4),
				gui_image_color_new(
					gui_color(255,45,45,45),
					main->position.w-6, 20,
					0
				)
			),
			0, NULL
		),
		gui_bar_new(
			gui_caption_new(tfont, gui_color(255,200,200,200), GUI_CAPTION_CENTER_X | GUI_CAPTION_CENTER_Y),
			gui_image_color_new(gui_color(255,90,40,40), main->position.w-6, 20, 0),
			0.0,
			0.0,
			0.0,
			GUI_BAR_HORIZONTAL | GUI_BAR_SHOW_CURRENT | GUI_BAR_SHOW_MAX
		)
	);
	gui_bar_text_set(bar, U8("seconds"));
	gui_div_table_attach(
			gui_div_table_create_row(main, 5, 1),
			bar,
			0,
			100,
			-1
	);

	player_s p = {
		.player = player,
		.bar = bar,
		.path = "~/Video/musicali/skioffi_yolandi.mp4",
		.video = NULL,
		.timer = NULL
	};
	gui_s* bStart = gui_button_attach(
		gui_new(
			main, "but", "button", GUI_MODE_NORMAL,
			1, 3, 400, 50, 50,
			gui_color(255,0,0,0),	
			gui_composite_add(
				gui_composite_new(4),
				gui_image_color_new(
					gui_color(255, 70, 70, 100),
					50, 50,
					0
				)
			),
			0, &p
		),
		gui_button_new(
			gui_caption_new(tfont, gui_color(255,40,40,40), GUI_CAPTION_CENTER_X | GUI_CAPTION_CENTER_Y),
			gui_image_color_new(gui_color(255,10,10,40), 50, 50, 0),
			gui_image_color_new(gui_color(255,80,80,120), 50, 50, 0),
			bStart_click
		)
	);
	gui_button_text_set(bStart, U8("start"));
	gui_div_table_attach(
			gui_div_table_create_row(main, 12, 2),
			bStart,
			0,
			50,
			-1
	);

	g2dColor_t fillcolor = gui_color(255, 40, 80, 120);	
	gui_s* bar2 = gui_bar_attach(
		gui_new(
			main, "bar2", "progrssbar", GUI_MODE_NORMAL,
			0, 3, 359, main->position.w-6, 20,
			gui_color(255,0,0,0),
			gui_composite_add(
				gui_composite_new(4),
				gui_image_color_new(
					gui_color(255,125,125,125),
					main->position.w-6, 20,
					0
				)
			),
			0, NULL
		),
		gui_bar_new(
			gui_caption_new(tfont, gui_color(255,200,200,200), GUI_CAPTION_CENTER_X | GUI_CAPTION_CENTER_Y),
			gui_image_fn_new(gui_bar_circle_fn, &fillcolor, main->position.w-6, 20, 0),
			0.0,
			100.0,
			0.0,
			GUI_BAR_CIRCLE | GUI_BAR_SHOW_CURRENT | GUI_BAR_MIN_ANGLE
		)
	);
	gui_div_table_attach(
			gui_div_table_row_get(main, 3),
			bar2,
			1,
			50,
			-1
	);
	bar2->userMargin.left = 2;
	bar2->userMargin.top = 2;
	guiTimer_s* tbar2 = gui_timer_new(bar2, 50, bar2_timer, bar2);

/*
	gui_s* txt = gui_text_attach(
		gui_new(
			main, "txt", "text", GUI_MODE_NORMAL,
			0, 10, btn->position.y + btn->position.h + 10, 200, 100,
			gui_color(255,0,0,0),	
			gui_background_new( gui_color(255, 200, 200, 200), NULL, NULL, NULL, GUI_BK_COLOR), 
			0, NULL
		),
		gui_text_new(
			tfont, gui_color(255,40,40,40), gui_color(255, 30, 30 ,200), gui_color(255,20,20,20), 4, 600,
			GUI_TEXT_SCROLL_X | GUI_TEXT_SCROLL_Y | GUI_TEXT_INSERT | GUI_TEXT_CUR_VISIBLE | GUI_TEXT_CURSOR_LIGHT
		)
	);

	btn->userdata = txt;
*/
	gui_redraw(lbl);
	gui_redraw(player);
	gui_redraw(bar);
	gui_redraw(bar2);
	gui_redraw(bStart);
	gui_redraw(main);
	gui_div_align(main);
	gui_show(main, 1);
	gui_show(lbl, 1);
	gui_show(player, 1);
	gui_show(bar, 1);
	gui_show(bStart, 1);
	gui_show(bar2,1);
	gui_focus(bStart);

	gui_loop();

	gui_end();	
	err_restore();
}

