#include <ef/media.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/err.h>
#include <ef/delay.h>

// fork
// https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/0_hello_world.c

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct media{
	AVFormatContext* avfctx;
	AVCodec* avcodec;
	AVCodecParameters* avcodecpar;
	int videoindex;
	AVCodecContext* avcodecctx;
	AVFrame* avframe;
	AVPacket* avpacket;
	double durate;
	long lastPTS;
	AVRational timebase;
	double fps;
	long pts;
	long synctime;
	size_t seek;
	size_t currentframe;
	unsigned width;
	unsigned height;
	int revcstate;
	int pacstate;
	g2dImage_s* frame;
	g2dImage_s* frameScaled;
	struct SwsContext *swsctx;
}media_s;

void media_free(media_s* media){
	if( media->avfctx ) avformat_close_input(&media->avfctx);
	if( media->avpacket ) av_packet_free(&media->avpacket);
	if( media->avframe ) av_frame_free(&media->avframe);
	if( media->avcodecctx ) avcodec_free_context(&media->avcodecctx);
	if( media->frame ) g2d_free(media->frame);
	free(media);
}

media_s* media_load(const char* path){
	media_s* media = mem_zero(media_s);
	if( !media ) err_fail("malloc");
	media->videoindex = -1;
	media->pacstate = -1;
	media->lastPTS = AV_NOPTS_VALUE;
	media->currentframe = 0;
	media->seek = 0;

	media->avfctx = avformat_alloc_context();
	if( !media->avfctx ){
		err_push("alloc av format context");
		media_free(media);
		return NULL;
	}

	if( avformat_open_input(&media->avfctx, path, NULL, NULL) ){
		err_push("could not open the file");
		media_free(media);
		return NULL;
	}

	if( avformat_find_stream_info(media->avfctx,  NULL) < 0){
		err_push("could not get the stream info");
		media_free(media);
		return NULL;
	}
	
	for( unsigned i = 0; i < media->avfctx->nb_streams; i++){
		AVCodecParameters *lccodecpar = media->avfctx->streams[i]->codecpar;
		
		AVCodec *lccodec = avcodec_find_decoder(lccodecpar->codec_id);
		if( !lccodec ){
			err_push("unsupported codec!");
			media_free(media);
			return NULL;
		}

		if( lccodecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			if( media->videoindex == -1 ){
				media->videoindex = i;
				media->avcodec = lccodec;
				media->avcodecpar = lccodecpar;
				if( media->avfctx->streams[i]->duration != AV_NOPTS_VALUE ){
					//media->durate = media->avfctx->streams[i]->duration / AV_TIME_BASE;
					media->durate = media->avfctx->streams[i]->duration * av_q2d(media->avfctx->streams[i]->time_base) * 1000.0;
				}
				else{
					media->durate = -1;
				}
				media->width = lccodecpar->width;
				media->height = lccodecpar->height;
				media->timebase = media->avfctx->streams[i]->time_base;
				if( media->avfctx->streams[i]->avg_frame_rate.num && media->avfctx->streams[i]->avg_frame_rate.den ){
					media->fps = av_q2d(media->avfctx->streams[i]->avg_frame_rate);
				}
				else if( media->avfctx->streams[i]->r_frame_rate.num && media->avfctx->streams[i]->r_frame_rate.den ){
					media->fps = av_q2d(media->avfctx->streams[i]->r_frame_rate);
				}
				else{
					media->fps = -1.0;
				}
				dbg_info("find stream:%d %u*%u %ffps %f", media->videoindex, media->width, media->height, media->fps, media->durate);
			}
		}
	   	//else if( lccodecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			//logging("Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
		//}
	}

	media->avcodecctx = avcodec_alloc_context3(media->avcodec);
	if( !media->avcodecctx ){
		err_push("to allocated memory for AVCodecContext");
		media_free(media);
		return NULL;
	}

	if( avcodec_parameters_to_context(media->avcodecctx,  media->avcodecpar) < 0 ){
		err_push("to copy codec params to codec context");
		media_free(media);
		return NULL;
	}

	if( avcodec_open2(media->avcodecctx, media->avcodec, NULL) < 0 ){
		err_push("to open codec through avcodec_open2");
		media_free(media);
		return NULL;
	}

	media->avframe = av_frame_alloc();
	if( !media->avframe ){
		err_push("to allocated memory for AVFrame");
		media_free(media);
		return NULL;
	}
  
	media->avpacket = av_packet_alloc();
	if( !media->avpacket ){
		err_push("to allocated memory for AVPacket");
		return NULL;
	}

	//media->format = media->avcodec->pix_fmts;

	media->frame = g2d_new( media->width, media->height, -1);
	if( !media->frame ) err_fail("eom");
	
	return media;
}

void media_resize_set(media_s* media, g2dImage_s* img){
	media->frameScaled = img;
}

__const inline __private float cubic_hermite(const float A, const float B, const float C, const float D, const float t){
	const float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
	const float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
	const float c = -A / 2.0f + C / 2.0f;
	return a*t*t*t + b*t*t + c*t + B;
}

__private g2dColor_t sample_bicubic(g2dImage_s* dst, AVFrame* frame, float u, float v){
	float x = (u * frame->width)-0.5;
	int xint = (int)x;
	float xfract = x-floor(x);

	float y = (v * frame->height) - 0.5;
	int yint = (int)y;
	float yfract = y - floor(y);
	
	if( xint == 0 ){
		++xint;
	}
	else if( xint >= frame->width - 1 ){
		xint = frame->width - 3;
	}
	else if( xint >= frame->width - 2 ){
		xint = frame->width - 3;
	}
	
	if( yint == 0 ){
		++yint;
	}
	else if( yint >= frame->height - 1 ){
		yint = frame->height - 3;
	}
	else if( yint >= frame->height - 2 ){
		yint = frame->height - 3;
	}
	
	uint8_t p[4];
	unsigned row[3][4];
	float col[4][3];

	row[0][0] = (yint-1) * frame->linesize[0];
	row[0][1] = (yint)   * frame->linesize[0];
	row[0][2] = (yint+1) * frame->linesize[0];
	row[0][3] = (yint+2) * frame->linesize[0];
	row[1][0] = (yint-1)/2 * frame->linesize[1];
	row[1][1] = (yint)/2   * frame->linesize[1];
	row[1][2] = (yint+1)/2 * frame->linesize[1];
	row[1][3] = (yint+2)/2 * frame->linesize[1];
	row[2][0] = (yint-1)/2 * frame->linesize[2];
	row[2][1] = (yint/2)   * frame->linesize[2];
	row[2][2] = (yint+1)/2 * frame->linesize[2];
	row[2][3] = (yint+2)/2 * frame->linesize[2];


	for( unsigned y = 0; y < 4; ++y){
		p[0] = frame->data[0][row[0][y] + xint-1];
		p[1] = frame->data[0][row[0][y] + xint  ];
		p[2] = frame->data[0][row[0][y] + xint+1];
		p[3] = frame->data[0][row[0][y] + xint+2];
		col[y][0] = cubic_hermite( p[0], p[1], p[2], p[3], xfract);

		p[0] = frame->data[1][row[1][y] + (xint-1)/2];
		p[1] = frame->data[1][row[1][y] + (xint)  /2];
		p[2] = frame->data[1][row[1][y] + (xint+1)/2];
		p[3] = frame->data[1][row[1][y] + (xint+2)/2];
		col[y][1] = cubic_hermite( p[0], p[1], p[2], p[3], xfract);

		p[0] = frame->data[2][row[2][y] + (xint-1)/2];
		p[1] = frame->data[2][row[2][y] + (xint)  /2];
		p[2] = frame->data[2][row[2][y] + (xint+1)/2];
		p[3] = frame->data[2][row[2][y] + (xint+2)/2];
		col[y][2] = cubic_hermite( p[0], p[1], p[2], p[3], xfract);

	}

	float value = cubic_hermite(col[0][0], col[1][0], col[2][0], col[3][0], yfract);
	const unsigned char Y = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;

	value = cubic_hermite(col[0][1], col[1][1], col[2][1], col[3][1], yfract);
	const unsigned char U = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;
	
	value = cubic_hermite(col[0][2], col[1][2], col[2][2], col[3][2], yfract);
	const unsigned char V = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;
	
	int r = Y + 1.402 * (V-128);
	int g = Y - 0.344 * (U-128) - 0.714*(V-128);
	int b = Y + 1.772 * (U-128);
	if( r > 255 ) r = 255;
	else if( r < 0 ) r = 0;
	if( g > 255 ) g = 255;
	else if( g < 0 ) g = 0;
	if( b > 255 ) b = 255;
	else if( b < 0 ) b = 0;

	//const unsigned char r = Y + 1.402 * (V-128);
	//const unsigned char g = Y - 0.344 * (U-128) - 0.714*(V-128);
	//const unsigned char b = Y + 1.772*(U-128);	

	return g2d_color_make(dst, 255, r, g, b);
} 

__private void frame_resize_to(g2dImage_s* dst, AVFrame* frame){
	dbg_info("resize %u*%u -> %u*%u", frame->width, frame->height, dst->w, dst->h);
	const unsigned w = dst->w;
	const unsigned h = dst->h;
	__parallef
	for( unsigned y = 0; y < h; ++y ){
		const float v = (const float)y / (const float)(h - 1);
		unsigned const row = g2d_row(dst, y);
		g2dColor_t* dcol = g2d_color(dst, row, 0);
		for( unsigned x = 0; x < w; ++x ){
            const float u = (const float)x / (const float)(w - 1);
            dcol[x] = sample_bicubic(dst, frame, u, v);
        }
    }
}

__private void frame_yuv_to_rgb(g2dImage_s* dst, AVFrame* frame){
	const unsigned w = dst->w;
	const unsigned h = dst->h;
	__parallef
	for( size_t y = 0; y < h; ++y){
		const unsigned row = g2d_row(dst, y);
		g2dColor_t* pix = g2d_color(dst, row, 0);
		const unsigned rowYFrame = frame->linesize[0] * y;
		const unsigned rowUFrame = frame->linesize[1] * (y>>1);
		const unsigned rowVFrame = frame->linesize[2] * (y>>1);	
		for( size_t x = 0; x < w; ++x ){
			const unsigned char Y = frame->data[0][rowYFrame + x];
			const unsigned char U = frame->data[1][rowUFrame + (x>>1)];
			const unsigned char V = frame->data[2][rowVFrame + (x>>1)];
			int r = Y + 1.402 * (V-128);
			int g = Y - 0.344 * (U-128) - 0.714*(V-128);
		   	int b = Y + 1.772 * (U-128);
			if( r > 255 ) r = 255;
			else if( r < 0 ) r = 0;
			if( g > 255 ) g = 255;
			else if( g < 0 ) g = 0;
			if( b > 255 ) b = 255;
			else if( b < 0 ) b = 0;

			pix[x] = g2d_color_make(dst, 255, r, g, b);
		}
	}
}

#ifdef DEBUG_ENABLE
void dbg_format(enum AVPixelFormat format){
    switch( format ){
	case AV_PIX_FMT_NONE:
	case AV_PIX_FMT_YUV420P: dbg_info("format AV_PIX_FMT_YUV420P"); break; 
	case AV_PIX_FMT_YUYV422: dbg_info("format AV_PIX_FMT_YUYV422"); break;  
	case AV_PIX_FMT_RGB24: dbg_info("format AV_PIX_FMT_RGB24"); break;  
	case AV_PIX_FMT_BGR24: dbg_info("format AV_PIX_FMT_BGR24"); break;     
	case AV_PIX_FMT_YUV422P: dbg_info("format AV_PIX_FMT_YUV422P"); break;
	case AV_PIX_FMT_YUV444P: dbg_info("format AV_PIX_FMT_YUV444P"); break;  
	case AV_PIX_FMT_YUV410P: dbg_info("format AV_PIX_FMT_YUV410P"); break;   
	case AV_PIX_FMT_YUV411P: dbg_info("format AV_PIX_FMT_YUV411P"); break;   
	case AV_PIX_FMT_GRAY8: dbg_info("format AV_PIX_FMT_GRAY8"); break;  
	case AV_PIX_FMT_MONOWHITE: dbg_info("format AV_PIX_FMT_MONOWHITE"); break;
	case AV_PIX_FMT_MONOBLACK: dbg_info("format AV_PIX_FMT_MONOBLACK"); break;
	case AV_PIX_FMT_PAL8: dbg_info("format AV_PIX_FMT_PAL8"); break;
	case AV_PIX_FMT_YUVJ420P: dbg_info("format AV_PIX_FMT_YUVJ420P"); break;
	case AV_PIX_FMT_YUVJ422P: dbg_info("format AV_PIX_FMT_YUVJ422P"); break;
	case AV_PIX_FMT_YUVJ444P: dbg_info("format AV_PIX_FMT_YUVJ444P"); break;
#if FF_API_XVMC
	case AV_PIX_FMT_XVMC_MPEG2_MC: dbg_info("format AV_PIX_FMT_XVMC_MPEG2_MC"); break;
	case AV_PIX_FMT_XVMC_MPEG2_IDCT: dbg_info("format AV_PIX_FMT_XVMC_MPEG2_IDCT"); break;
	case AV_PIX_FMT_XVMC: dbg_info("format AV_PIX_FMT_XVMC"); break;
	case AV_PIX_FMT_XVMC_MPEG2_IDCT: dbg_info("format AV_PIX_FMT_XVMC_MPEG2_IDCT"); break;
#endif
	case AV_PIX_FMT_UYVY422: dbg_info("format AV_PIX_FMT_UYVY422"); break;
	case AV_PIX_FMT_UYYVYY411: dbg_info("format AV_PIX_FMT_UYYVYY411"); break; 
	case AV_PIX_FMT_BGR8: dbg_info("format AV_PIX_FMT_BGR8"); break;      
	case AV_PIX_FMT_BGR4: dbg_info("format AV_PIX_FMT_BGR4"); break;      
	case AV_PIX_FMT_BGR4_BYTE: dbg_info("format AV_PIX_FMT_BGR4_BYTE"); break; 
	case AV_PIX_FMT_RGB8: dbg_info("format AV_PIX_FMT_RGB8"); break;      
	case AV_PIX_FMT_RGB4: dbg_info("format AV_PIX_FMT_RGB4"); break;      
	case AV_PIX_FMT_RGB4_BYTE: dbg_info("format AV_PIX_FMT_RGB4_BYTE"); break; 
	case AV_PIX_FMT_NV12: dbg_info("format AV_PIX_FMT_NV12"); break;      
	case AV_PIX_FMT_NV21: dbg_info("format AV_PIX_FMT_NV21"); break;      

	case AV_PIX_FMT_ARGB: dbg_info("format AV_PIX_FMT_ARGB"); break;      
	case AV_PIX_FMT_RGBA: dbg_info("format AV_PIX_FMT_RGBA"); break;      
	case AV_PIX_FMT_ABGR: dbg_info("format AV_PIX_FMT_ABGR"); break;      
	case AV_PIX_FMT_BGRA: dbg_info("format AV_PIX_FMT_BGRA"); break;      

	case AV_PIX_FMT_GRAY16BE: dbg_info("format AV_PIX_FMT_GRAY16BE"); break;  
	case AV_PIX_FMT_GRAY16LE: dbg_info("format AV_PIX_FMT_GRAY16LE"); break;  
	case AV_PIX_FMT_YUV440P: dbg_info("format AV_PIX_FMT_YUV440P"); break;   
	case AV_PIX_FMT_YUVJ440P: dbg_info("format AV_PIX_FMT_YUVJ440P"); break;  
	case AV_PIX_FMT_YUVA420P: dbg_info("format AV_PIX_FMT_YUVA420P"); break;  
#if FF_API_VDPAU
	case AV_PIX_FMT_VDPAU_H264: dbg_info("format AV_PIX_FMT_VDPAU_H264"); break;
	case AV_PIX_FMT_VDPAU_MPEG1: dbg_info("format AV_PIX_FMT_VDPAU_MPEG1"); break;
	case AV_PIX_FMT_VDPAU_MPEG2: dbg_info("format AV_PIX_FMT_VDPAU_MPEG2"); break;
	case AV_PIX_FMT_VDPAU_WMV3: dbg_info("format AV_PIX_FMT_VDPAU_WMV3"); break;
	case AV_PIX_FMT_VDPAU_VC1: dbg_info("format AV_PIX_FMT_VDPAU_VC1"); break; 
#endif
	case AV_PIX_FMT_RGB48BE: dbg_info("format AV_PIX_FMT_RGB48BE"); break;   
	case AV_PIX_FMT_RGB48LE: dbg_info("format AV_PIX_FMT_RGB48LE"); break;   
	case AV_PIX_FMT_RGB565BE: dbg_info("format AV_PIX_FMT_RGB565BE"); break;  
	case AV_PIX_FMT_RGB565LE: dbg_info("format AV_PIX_FMT_RGB565LE"); break;  
	case AV_PIX_FMT_RGB555BE: dbg_info("format AV_PIX_FMT_RGB555BE"); break;  
	case AV_PIX_FMT_RGB555LE: dbg_info("format AV_PIX_FMT_RGB555LE"); break;  
	case AV_PIX_FMT_BGR565BE: dbg_info("format AV_PIX_FMT_BGR565BE"); break;  
	case AV_PIX_FMT_BGR565LE: dbg_info("format AV_PIX_FMT_BGR565LE"); break;  
	case AV_PIX_FMT_BGR555BE: dbg_info("format AV_PIX_FMT_BGR555BE"); break;  
	case AV_PIX_FMT_BGR555LE: dbg_info("format AV_PIX_FMT_BGR555LE"); break;  
#if FF_API_VAAPI
	case AV_PIX_FMT_VAAPI_MOCO: dbg_info("format AV_PIX_FMT_VAAPI_MOCO"); break; 
	case AV_PIX_FMT_VAAPI_IDCT: dbg_info("format AV_PIX_FMT_VAAPI_IDCT"); break; 
	case AV_PIX_FMT_VAAPI_VLD: dbg_info("format AV_PIX_FMT_VAAPI_VLD"); break;  
	//case AV_PIX_FMT_VAAPI: dbg_info("format AV_PIX_FMT_VAAPI"); break;
	//case AV_PIX_FMT_VAAPI_VLD: dbg_info("format AV_PIX_FMT_VAAPI_VLD"); break;
#else
	case AV_PIX_FMT_VAAPI: dbg_info("format AV_PIX_FMT_VAAPI"); break;
#endif
	case AV_PIX_FMT_YUV420P16LE: dbg_info("format AV_PIX_FMT_YUV420P16LE"); break;  
	case AV_PIX_FMT_YUV420P16BE: dbg_info("format AV_PIX_FMT_YUV420P16BE"); break;  
	case AV_PIX_FMT_YUV422P16LE: dbg_info("format AV_PIX_FMT_YUV422P16LE"); break;  
	case AV_PIX_FMT_YUV422P16BE: dbg_info("format AV_PIX_FMT_YUV422P16BE"); break;  
	case AV_PIX_FMT_YUV444P16LE: dbg_info("format AV_PIX_FMT_YUV444P16LE"); break;  
	case AV_PIX_FMT_YUV444P16BE: dbg_info("format AV_PIX_FMT_YUV444P16BE"); break;  
#if FF_API_VDPAU
	case AV_PIX_FMT_VDPAU_MPEG4: dbg_info("format AV_PIX_FMT_VDPAU_MPEG4"); break;  
#endif
	case AV_PIX_FMT_DXVA2_VLD: dbg_info("format AV_PIX_FMT_DXVA2_VLD"); break;    
	case AV_PIX_FMT_RGB444LE: dbg_info("format AV_PIX_FMT_RGB444LE"); break;  
	case AV_PIX_FMT_RGB444BE: dbg_info("format AV_PIX_FMT_RGB444BE"); break;  
	case AV_PIX_FMT_BGR444LE: dbg_info("format AV_PIX_FMT_BGR444LE"); break;  
	case AV_PIX_FMT_BGR444BE: dbg_info("format AV_PIX_FMT_BGR444BE"); break;  
	case AV_PIX_FMT_YA8: dbg_info("format AV_PIX_FMT_YA8"); break;       
	//case AV_PIX_FMT_Y400A: dbg_info("format AV_PIX_FMT_Y400A"); break;
	//case AV_PIX_FMT_YA8: dbg_info("format AV_PIX_FMT_YA8"); break; 
	//case AV_PIX_FMT_GRAY8A
	//case AV_PIX_FMT_YA8: dbg_info("format AV_PIX_FMT_YA8"); break; 
	case AV_PIX_FMT_BGR48BE: dbg_info("format AV_PIX_FMT_BGR48BE"); break;   
	case AV_PIX_FMT_BGR48LE: dbg_info("format AV_PIX_FMT_BGR48LE"); break;   
	case AV_PIX_FMT_YUV420P9BE: dbg_info("format AV_PIX_FMT_YUV420P9BE"); break; 
	case AV_PIX_FMT_YUV420P9LE: dbg_info("format AV_PIX_FMT_YUV420P9LE"); break; 
	case AV_PIX_FMT_YUV420P10BE: dbg_info("format AV_PIX_FMT_YUV420P10BE"); break;
	case AV_PIX_FMT_YUV420P10LE: dbg_info("format AV_PIX_FMT_YUV420P10LE"); break;
	case AV_PIX_FMT_YUV422P10BE: dbg_info("format AV_PIX_FMT_YUV422P10BE"); break;
	case AV_PIX_FMT_YUV422P10LE: dbg_info("format AV_PIX_FMT_YUV422P10LE"); break;
	case AV_PIX_FMT_YUV444P9BE: dbg_info("format AV_PIX_FMT_YUV444P9BE"); break; 
	case AV_PIX_FMT_YUV444P9LE: dbg_info("format AV_PIX_FMT_YUV444P9LE"); break; 
	case AV_PIX_FMT_YUV444P10BE: dbg_info("format AV_PIX_FMT_YUV444P10BE"); break;
	case AV_PIX_FMT_YUV444P10LE: dbg_info("format AV_PIX_FMT_YUV444P10LE"); break;
	case AV_PIX_FMT_YUV422P9BE: dbg_info("format AV_PIX_FMT_YUV422P9BE"); break; 
	case AV_PIX_FMT_YUV422P9LE: dbg_info("format AV_PIX_FMT_YUV422P9LE"); break; 
	//case AV_PIX_FMT_VDA_VLD: dbg_info("format AV_PIX_FMT_VDA_VLD"); break;    
	case AV_PIX_FMT_GBRP: dbg_info("format AV_PIX_FMT_GBRP"); break;      
	//case AV_PIX_FMT_GBR24P: dbg_info("format AV_PIX_FMT_GBR24P"); break;
	//case AV_PIX_FMT_GBRP: dbg_info("format AV_PIX_FMT_GBRP"); break;
	case AV_PIX_FMT_GBRP9BE: dbg_info("format AV_PIX_FMT_GBRP9BE"); break;   
	case AV_PIX_FMT_GBRP9LE: dbg_info("format AV_PIX_FMT_GBRP9LE"); break;   
	case AV_PIX_FMT_GBRP10BE: dbg_info("format AV_PIX_FMT_GBRP10BE"); break;  
	case AV_PIX_FMT_GBRP10LE: dbg_info("format AV_PIX_FMT_GBRP10LE"); break;  
	case AV_PIX_FMT_GBRP16BE: dbg_info("format AV_PIX_FMT_GBRP16BE"); break;  
	case AV_PIX_FMT_GBRP16LE: dbg_info("format AV_PIX_FMT_GBRP16LE"); break;  
	case AV_PIX_FMT_YUVA422P: dbg_info("format AV_PIX_FMT_YUVA422P"); break;  
	case AV_PIX_FMT_YUVA444P: dbg_info("format AV_PIX_FMT_YUVA444P"); break;  
	case AV_PIX_FMT_YUVA420P9BE: dbg_info("format AV_PIX_FMT_YUVA420P9BE"); break;  
	case AV_PIX_FMT_YUVA420P9LE: dbg_info("format AV_PIX_FMT_YUVA420P9LE"); break;  
	case AV_PIX_FMT_YUVA422P9BE: dbg_info("format AV_PIX_FMT_YUVA422P9BE"); break;  
	case AV_PIX_FMT_YUVA422P9LE: dbg_info("format AV_PIX_FMT_YUVA422P9LE"); break;  
	case AV_PIX_FMT_YUVA444P9BE: dbg_info("format AV_PIX_FMT_YUVA444P9BE"); break;  
	case AV_PIX_FMT_YUVA444P9LE: dbg_info("format AV_PIX_FMT_YUVA444P9LE"); break;  
	case AV_PIX_FMT_YUVA420P10BE: dbg_info("format AV_PIX_FMT_YUVA420P10BE"); break; 
	case AV_PIX_FMT_YUVA420P10LE: dbg_info("format AV_PIX_FMT_YUVA420P10LE"); break; 
	case AV_PIX_FMT_YUVA422P10BE: dbg_info("format AV_PIX_FMT_YUVA422P10BE"); break; 
	case AV_PIX_FMT_YUVA422P10LE: dbg_info("format AV_PIX_FMT_YUVA422P10LE"); break; 
	case AV_PIX_FMT_YUVA444P10BE: dbg_info("format AV_PIX_FMT_YUVA444P10BE"); break; 
	case AV_PIX_FMT_YUVA444P10LE: dbg_info("format AV_PIX_FMT_YUVA444P10LE"); break; 
	case AV_PIX_FMT_YUVA420P16BE: dbg_info("format AV_PIX_FMT_YUVA420P16BE"); break; 
	case AV_PIX_FMT_YUVA420P16LE: dbg_info("format AV_PIX_FMT_YUVA420P16LE"); break; 
	case AV_PIX_FMT_YUVA422P16BE: dbg_info("format AV_PIX_FMT_YUVA422P16BE"); break; 
	case AV_PIX_FMT_YUVA422P16LE: dbg_info("format AV_PIX_FMT_YUVA422P16LE"); break; 
	case AV_PIX_FMT_YUVA444P16BE: dbg_info("format AV_PIX_FMT_YUVA444P16BE"); break; 
	case AV_PIX_FMT_YUVA444P16LE: dbg_info("format AV_PIX_FMT_YUVA444P16LE"); break; 
	case AV_PIX_FMT_VDPAU: dbg_info("format AV_PIX_FMT_VDPAU"); break;     
	case AV_PIX_FMT_XYZ12LE: dbg_info("format AV_PIX_FMT_XYZ12LE"); break;      
	case AV_PIX_FMT_XYZ12BE: dbg_info("format AV_PIX_FMT_XYZ12BE"); break;      
	case AV_PIX_FMT_NV16: dbg_info("format AV_PIX_FMT_NV16"); break;         
	case AV_PIX_FMT_NV20LE: dbg_info("format AV_PIX_FMT_NV20LE"); break;       
	case AV_PIX_FMT_NV20BE: dbg_info("format AV_PIX_FMT_NV20BE"); break;       
	case AV_PIX_FMT_RGBA64BE: dbg_info("format AV_PIX_FMT_RGBA64BE"); break;     
	case AV_PIX_FMT_RGBA64LE: dbg_info("format AV_PIX_FMT_RGBA64LE"); break;     
	case AV_PIX_FMT_BGRA64BE: dbg_info("format AV_PIX_FMT_BGRA64BE"); break;     
	case AV_PIX_FMT_BGRA64LE: dbg_info("format AV_PIX_FMT_BGRA64LE"); break;     
	case AV_PIX_FMT_YVYU422: dbg_info("format AV_PIX_FMT_YVYU422"); break;   
	//case AV_PIX_FMT_VDA: dbg_info("format AV_PIX_FMT_VDA"); break;          
	case AV_PIX_FMT_YA16BE: dbg_info("format AV_PIX_FMT_YA16BE"); break;       
	case AV_PIX_FMT_YA16LE: dbg_info("format AV_PIX_FMT_YA16LE"); break;       
	case AV_PIX_FMT_GBRAP: dbg_info("format AV_PIX_FMT_GBRAP"); break;        
	case AV_PIX_FMT_GBRAP16BE: dbg_info("format AV_PIX_FMT_GBRAP16BE"); break;    
	case AV_PIX_FMT_GBRAP16LE: dbg_info("format AV_PIX_FMT_GBRAP16LE"); break;    
	case AV_PIX_FMT_QSV: dbg_info("format AV_PIX_FMT_QSV"); break;
	case AV_PIX_FMT_MMAL: dbg_info("format AV_PIX_FMT_MMAL"); break;
	case AV_PIX_FMT_D3D11VA_VLD: dbg_info("format AV_PIX_FMT_D3D11VA_VLD"); break;  
	case AV_PIX_FMT_CUDA: dbg_info("format AV_PIX_FMT_CUDA"); break;
	case AV_PIX_FMT_0RGB: dbg_info("format AV_PIX_FMT_0RGB"); break;
	case AV_PIX_FMT_RGB0: dbg_info("format AV_PIX_FMT_RGB0"); break;        
	case AV_PIX_FMT_0BGR: dbg_info("format AV_PIX_FMT_0BGR"); break;        
	case AV_PIX_FMT_BGR0: dbg_info("format AV_PIX_FMT_BGR0"); break;        
	case AV_PIX_FMT_YUV420P12BE: dbg_info("format AV_PIX_FMT_YUV420P12BE"); break; 
	case AV_PIX_FMT_YUV420P12LE: dbg_info("format AV_PIX_FMT_YUV420P12LE"); break; 
	case AV_PIX_FMT_YUV420P14BE: dbg_info("format AV_PIX_FMT_YUV420P14BE"); break; 
	case AV_PIX_FMT_YUV420P14LE: dbg_info("format AV_PIX_FMT_YUV420P14LE"); break; 
	case AV_PIX_FMT_YUV422P12BE: dbg_info("format AV_PIX_FMT_YUV422P12BE"); break; 
	case AV_PIX_FMT_YUV422P12LE: dbg_info("format AV_PIX_FMT_YUV422P12LE"); break; 
	case AV_PIX_FMT_YUV422P14BE: dbg_info("format AV_PIX_FMT_YUV422P14BE"); break; 
	case AV_PIX_FMT_YUV422P14LE: dbg_info("format AV_PIX_FMT_YUV422P14LE"); break; 
	case AV_PIX_FMT_YUV444P12BE: dbg_info("format AV_PIX_FMT_YUV444P12BE"); break; 
	case AV_PIX_FMT_YUV444P12LE: dbg_info("format AV_PIX_FMT_YUV444P12LE"); break; 
	case AV_PIX_FMT_YUV444P14BE: dbg_info("format AV_PIX_FMT_YUV444P14BE"); break; 
	case AV_PIX_FMT_YUV444P14LE: dbg_info("format AV_PIX_FMT_YUV444P14LE"); break; 
	case AV_PIX_FMT_GBRP12BE: dbg_info("format AV_PIX_FMT_GBRP12BE"); break;    
	case AV_PIX_FMT_GBRP12LE: dbg_info("format AV_PIX_FMT_GBRP12LE"); break;    
	case AV_PIX_FMT_GBRP14BE: dbg_info("format AV_PIX_FMT_GBRP14BE"); break;    
	case AV_PIX_FMT_GBRP14LE: dbg_info("format AV_PIX_FMT_GBRP14LE"); break;    
	case AV_PIX_FMT_YUVJ411P: dbg_info("format AV_PIX_FMT_YUVJ411P"); break;    
	case AV_PIX_FMT_BAYER_BGGR8: dbg_info("format AV_PIX_FMT_BAYER_BGGR8"); break;    
	case AV_PIX_FMT_BAYER_RGGB8: dbg_info("format AV_PIX_FMT_BAYER_RGGB8"); break;    
	case AV_PIX_FMT_BAYER_GBRG8: dbg_info("format AV_PIX_FMT_BAYER_GBRG8"); break;    
	case AV_PIX_FMT_BAYER_GRBG8: dbg_info("format AV_PIX_FMT_BAYER_GRBG8"); break;    
	case AV_PIX_FMT_BAYER_BGGR16LE: dbg_info("format AV_PIX_FMT_BAYER_BGGR16LE"); break; 
	case AV_PIX_FMT_BAYER_BGGR16BE: dbg_info("format AV_PIX_FMT_BAYER_BGGR16BE"); break; 
	case AV_PIX_FMT_BAYER_RGGB16LE: dbg_info("format AV_PIX_FMT_BAYER_RGGB16LE"); break; 
	case AV_PIX_FMT_BAYER_RGGB16BE: dbg_info("format AV_PIX_FMT_BAYER_RGGB16BE"); break; 
	case AV_PIX_FMT_BAYER_GBRG16LE: dbg_info("format AV_PIX_FMT_BAYER_GBRG16LE"); break; 
	case AV_PIX_FMT_BAYER_GBRG16BE: dbg_info("format AV_PIX_FMT_BAYER_GBRG16BE"); break; 
	case AV_PIX_FMT_BAYER_GRBG16LE: dbg_info("format AV_PIX_FMT_BAYER_GRBG16LE"); break; 
	case AV_PIX_FMT_BAYER_GRBG16BE: dbg_info("format AV_PIX_FMT_BAYER_GRBG16BE"); break; 
#if !FF_API_XVMC
	case AV_PIX_FMT_XVMC: dbg_info("format AV_PIX_FMT_XVMC"); break;
#endif /* !FF_API_XVMC */
	case AV_PIX_FMT_YUV440P10LE: dbg_info("format AV_PIX_FMT_YUV440P10LE"); break; 
	case AV_PIX_FMT_YUV440P10BE: dbg_info("format AV_PIX_FMT_YUV440P10BE"); break; 
	case AV_PIX_FMT_YUV440P12LE: dbg_info("format AV_PIX_FMT_YUV440P12LE"); break; 
	case AV_PIX_FMT_YUV440P12BE: dbg_info("format AV_PIX_FMT_YUV440P12BE"); break; 
	case AV_PIX_FMT_AYUV64LE: dbg_info("format AV_PIX_FMT_AYUV64LE"); break;    
	case AV_PIX_FMT_AYUV64BE: dbg_info("format AV_PIX_FMT_AYUV64BE"); break;    
	case AV_PIX_FMT_VIDEOTOOLBOX: dbg_info("format AV_PIX_FMT_VIDEOTOOLBOX"); break; 
	case AV_PIX_FMT_P010LE: dbg_info("format AV_PIX_FMT_P010LE"); break; 
	case AV_PIX_FMT_P010BE: dbg_info("format AV_PIX_FMT_P010BE"); break; 
	case AV_PIX_FMT_GBRAP12BE: dbg_info("format AV_PIX_FMT_GBRAP12BE"); break;  
	case AV_PIX_FMT_GBRAP12LE: dbg_info("format AV_PIX_FMT_GBRAP12LE"); break;  
	case AV_PIX_FMT_GBRAP10BE: dbg_info("format AV_PIX_FMT_GBRAP10BE"); break;  
	case AV_PIX_FMT_GBRAP10LE: dbg_info("format AV_PIX_FMT_GBRAP10LE"); break;  
	case AV_PIX_FMT_MEDIACODEC: dbg_info("format AV_PIX_FMT_MEDIACODEC"); break; 
	default: case AV_PIX_FMT_NB: dbg_info("format AV_PIX_FMT_NB"); break;

	}
}
#else 
	#define dbg_format(F)
#endif

__private int decode_packet(media_s* media){
	if( media->pacstate < 0 ){
		dbg_info("send decode");
		media->pacstate = avcodec_send_packet( media->avcodecctx, media->avpacket);
		if( media->pacstate < 0 ){
			dbg_error("send packet");
			err_push("while sending a packet to the decoder: %s", av_err2str(media->pacstate));
			return -1;
		}
	}

	dbg_info("get packet");
	media->pacstate = avcodec_receive_frame(media->avcodecctx, media->avframe);
	if( media->pacstate == AVERROR(EAGAIN) || media-> pacstate== AVERROR_EOF ){
		dbg_info("eagain, averror_eof");
		return 0;
	}
	else if( media->pacstate < 0 ){
		dbg_error("frame decoded");
		err_push("while receiving a frame from the decoder: %s", av_err2str(media->pacstate));
		return -1;
	}
	else if( media->pacstate >= 0 ){
		dbg_info("convert");
		media->lastPTS = media->pts;
		media->pts = media->avframe->pts;		
		dbg_info("format:%d", media->avframe->format);	
		dbg_format(media->avframe->format);
		if( media->seek ){
			--media->seek;
			return 0;
		}
		if( media->frameScaled ){
			frame_resize_to(media->frameScaled, media->avframe);
		}
		else{
			iassert((unsigned)media->avframe->width == media->frame->w);
			iassert((unsigned)media->avframe->height == media->frame->h);
			frame_yuv_to_rgb(media->frame, media->avframe);	
		}
		return 1;
	}
	
	return 0;
}

void media_redraw(media_s* media){
	if( media->frameScaled ){
		frame_resize_to(media->frameScaled, media->avframe);
	}
	else{
		iassert((unsigned)media->avframe->width == media->frame->w);
		iassert((unsigned)media->avframe->height == media->frame->h);
		frame_yuv_to_rgb(media->frame, media->avframe);	
	}
}

int media_decode(media_s* media){
	media->synctime = time_us();

	if( media->pacstate >= 0 ){
		dbg_info("continue previous packet");
		int ret = decode_packet(media);
		if( ret != 0 ) return ret;
	}
	
	if( (media->revcstate = av_read_frame(media->avfctx, media->avpacket)) >=0 ){
		dbg_info("data is video");
		if( media->avpacket->stream_index == media->videoindex ){
			int ret = decode_packet(media);
			av_packet_unref(media->avpacket);
			return ret;
		}
		av_packet_unref(media->avpacket);
	}
	
	return media->revcstate >= 0 ? 0 : -1;
}

g2dImage_s* media_frame_get(media_s* media){
	return media->frameScaled ? media->frameScaled : media->frame;
}

long media_delay_get(media_s* media){
	if( media->pts != AV_NOPTS_VALUE && media->lastPTS != AV_NOPTS_VALUE ){
		long delay = av_rescale_q( media->pts - media->lastPTS, media->timebase, AV_TIME_BASE_Q);
		dbg_info("DELAY: %luus", delay);
        if (delay > 0 && delay < 1000000) return delay;
	}
	return 0;
}

void media_sleep(media_s* media){
	long delay = media_delay_get(media);
	long te = time_us() - media->synctime;
	if( delay > te ){
		usleep(delay - te);
	}
}

unsigned media_width(media_s* media){
	iassert(media);
	return media->width;
}

unsigned media_height(media_s* media){
	iassert(media);
	return media->height;
}

double media_duration(media_s* media){
	iassert(media);
	return media->durate;
}

double media_fps(media_s* media){
	iassert(media);
	return media->fps;
}

void media_seek(media_s* media, double s){
	size_t frame = s * media->fps;
	if( frame < media->currentframe ){
		av_seek_frame(media->avfctx, media->videoindex, 0, AVSEEK_FLAG_BACKWARD);
		media->seek = frame;
	}
	else if( frame > media->currentframe ){
		media->seek = frame - media->currentframe; 
	}	
}

double media_time(media_s* media){
	return media->currentframe / media->fps;
}
