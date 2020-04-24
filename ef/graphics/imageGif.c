#include <ef/imageGif.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/file.h>
#include <ef/err.h>

#include <gif_lib.h>

/* fork
 * https://chromium.googlesource.com/webm/libwebp/+/0.5.1/examples/gifdec.c
 */

g2dColor_t gif_get_background_color(const ColorMapObject* const colormap, int bgcolorindex, int transparentindex){
	if( colormap == NULL || colormap->Colors == NULL || bgcolorindex >= colormap->ColorCount) {
		dbg_info("bk white");
		return g2d_color_gen(G2D_MODE_ARGB, 255, 255, 255, 255);
	}

	if( transparentindex != NO_TRANSPARENT_COLOR && bgcolorindex == transparentindex){
		dbg_info("bk transparent");
		return g2d_color_gen(G2D_MODE_ARGB, 0, 255, 255, 255);
	}

	dbg_info("bk colormap");
	const GifColorType color = colormap->Colors[bgcolorindex];
	return g2d_color_gen(G2D_MODE_ARGB, 255, color.Red, color.Green, color.Blue);
}

gif_s* g2d_load_gif(char const* path){
	int err;
	GifFileType* raw = DGifOpenFileName(path, &err);
	if( !raw ){
		err_push("gif error(%d):%s", err, GifErrorString(err));
		return NULL;
	}

	if( DGifSlurp(raw) == GIF_ERROR ){
		err_push("gif error(%d):%s", err, GifErrorString(err));
		DGifCloseFile(raw, NULL);
		return NULL;	
	}
	dbg_info("frame count: %d size: %d %d ", raw->ImageCount, raw->SWidth, raw->SHeight);
	
	gif_s* gif = mem_new(gif_s);
	if( !gif ) err_fail("malloc");
	gif->frames = vector_new(gifFrame_s, raw->ImageCount, 1);
	if( !gif->frames ) err_fail("malloc");
	gif->width = raw->SWidth;
	gif->height = raw->SHeight;

	for( int f = 0; f < raw->ImageCount; ++f ){
		gifFrame_s* frame = vector_get_push_back(gif->frames);
		frame->dispose = 0;
		frame->delay = 250;
		frame->transindex = -1;

		for(int b = 0; b < raw->SavedImages[f].ExtensionBlockCount; ++b ){
			if( raw->SavedImages[f].ExtensionBlocks[b].Function == GRAPHICS_EXT_FUNC_CODE ){
				GraphicsControlBlock gcb;
				DGifExtensionToGCB(raw->SavedImages[f].ExtensionBlocks[b].ByteCount, raw->SavedImages[f].ExtensionBlocks[b].Bytes, &gcb);
				frame->delay = gcb.DelayTime * 10;
				frame->dispose = gcb.DisposalMode;
				frame->transindex = gcb.TransparentColor;
				//dbg_info("frame %d block %d delay: %d dispose: %d transindex: %d", f, b, frame->delay, frame->dispose, frame->transindex);
				break;
			}
		}

		frame->pos.x = raw->SavedImages[f].ImageDesc.Left;
		frame->pos.y = raw->SavedImages[f].ImageDesc.Top;
		frame->pos.w = raw->SavedImages[f].ImageDesc.Width;
		frame->pos.h = raw->SavedImages[f].ImageDesc.Height;
		
		
		frame->img = g2d_new(raw->SWidth, raw->SHeight, -1);
		if( !frame->img ) err_fail("malloc");
	
		ColorMapObject* cmap = raw->SavedImages[f].ImageDesc.ColorMap ? raw->SavedImages[f].ImageDesc.ColorMap : raw->SColorMap;	

		switch( frame->dispose ){
			default: case DISPOSAL_UNSPECIFIED:{
				dbg_info("dispose unspecified");
				g2dCoord_s pos = { .x = 0, .y = 0, .w = frame->img->w, .h = frame->img->h };
				g2d_clear(frame->img, g2d_color_make(frame->img, 255, 255, 255, 255), &pos);
			}
			break;

			case DISPOSE_DO_NOT:{
				dbg_info("dispose do not");
				if( f < 1 ) break;
				gifFrame_s* prev = &gif->frames[f-1];
				g2dCoord_s prevpos = { .x = 0, .y = 0, .w = prev->img->w, .h = prev->img->h };
				g2dCoord_s pos = { .x = 0, .y = 0, .w = frame->img->w, .h = frame->img->h };
				g2d_bitblt(frame->img, &pos, prev->img, &prevpos);
			}
			break;

			case DISPOSE_BACKGROUND:{
				dbg_info("dispose bk");
				g2d_clear(frame->img, gif_get_background_color(cmap, raw->SBackGroundColor, frame->transindex), &frame->pos);
			}
			break;

			case DISPOSE_PREVIOUS:{
				if( f < 1 ) break;
				dbg_info("dispose previous");
				gifFrame_s* prev = &gif->frames[f-1];
				g2d_bitblt_alpha(frame->img, &frame->pos, prev->img, &prev->pos);
			}
			break;
		}

		unsigned char* buf = raw->SavedImages[f].RasterBits;
//5 6 7 8
		dbg_info("interlace: %d transindex: %d pos: %d %d %d*%d ", raw->SavedImages[f].ImageDesc.Interlace, frame->transindex, frame->pos.x, frame->pos.y, frame->pos.w, frame->pos.h);
		//dbg_info("count: %d need: %d", raw->SavedImages[f].Im 


		if( raw->SavedImages[f].ImageDesc.Interlace ){
			const int interlace_offsets[] = { 0, 4, 2, 1 };
			const int interlace_jumps[]   = { 8, 8, 4, 2 };
			for( unsigned pass = 0; pass < 4; ++pass) {
				for( unsigned y = interlace_offsets[pass]; y < frame->pos.h && y + frame->pos.y < frame->img->h; y += interlace_jumps[pass]) {
					const unsigned row = g2d_row(frame->img, y + frame->pos.y);
					g2dColor_t* pix = g2d_color(frame->img, row, frame->pos.x);
					for( unsigned x = 0; x < frame->pos.w && x + frame->pos.x < frame->img->w; ++x ){
						if( frame->transindex >= 0 && *buf == frame->transindex ){
							++buf;
						}
						else{
							GifColorType rgb= cmap->Colors[*buf++];
							pix[x] = g2d_color_make(frame->img, 255, rgb.Red, rgb.Green, rgb.Blue);
						}
					}
				}
			}	
		}
		else{
			for( unsigned y = 0; y < frame->pos.h && y + frame->pos.y < frame->img->h; ++y){
				const unsigned row = g2d_row(frame->img, y + frame->pos.y);
				g2dColor_t* pix = g2d_color(frame->img, row, frame->pos.x);
				for( unsigned x = 0; x < frame->pos.w && x + frame->pos.x < frame->img->w; ++x ){
					if( frame->transindex >= 0 && *buf == frame->transindex ){
						++buf;
					}
					else{
						GifColorType rgb= cmap->Colors[*buf++];
						pix[x] = g2d_color_make(frame->img, 255, rgb.Red, rgb.Green, rgb.Blue);
					}
				}
			}
		}
	}

	DGifCloseFile(raw, NULL);
	return gif;
}

void g2d_gif_free(gif_s* gif){
	vector_foreach(gif->frames, i){
		g2d_free(gif->frames[i].img);
	}
	vector_free(gif->frames);
	free(gif);
}

void g2d_gif_resize(gif_s* gif, unsigned width, unsigned height, int ratio){
	unsigned w,h;
	vector_foreach(gif->frames, i){
		g2dImage_s* old = gif->frames[i].img;
		w = width;
		h = height;
		g2d_ratio(ratio, gif->frames[i].img->w, gif->frames[i].img->h, &w, &h);
		gif->frames[i].img = g2d_resize(old, w, h);
		g2d_free(old);
	}
}
