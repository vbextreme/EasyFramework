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
	long durate;
	long lastPTS;
	AVRational timebase;
	long pts;
	long synctime;
	unsigned width;
	unsigned height;
	int revcstate;
	int pacstate;
	g2dImage_s* frame;
	g2dImage_s* frameScaled;
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

	// loop though all the streams and print its main information
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
				media->durate = media->avfctx->streams[i]->duration;
				media->width = lccodecpar->width;
				media->height = lccodecpar->height;
				media->timebase = media->avfctx->streams[i]->time_base;
				dbg_info("find stream:%d %u*%u", media->videoindex, media->width, media->height);
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

__always_inline __private float cubic_hermite(float A, float B, float C, float D, float t){
	float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
	float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
	float c = -A / 2.0f + C / 2.0f;
	float d = B;
	return a*t*t*t + b*t*t + c*t + d;
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
	
	const unsigned char r = Y + 1.402 * (V-128);
	const unsigned char g = Y - 0.344 * (U-128) - 0.714*(V-128);
	const unsigned char b = Y + 1.772*(U-128);	

	return g2d_color_make(dst, 255, r, g, b);
} 

__private void frame_resize_to(g2dImage_s* dst, AVFrame* frame){
	dbg_info("resize %u*%u -> %u*%u", frame->width, frame->height, dst->w, dst->h);
	for( unsigned y = 0; y < dst->h; ++y ){
		float v = (float)y / (float)(dst->h - 1);
		unsigned const row = g2d_row(dst, y);
		g2dColor_t* dcol = g2d_color(dst, row, 0);
        __parallef 
		for( unsigned x = 0; x < dst->w; ++x ){
            float u = (float)x / (float)(dst->w - 1);
            dcol[x] = sample_bicubic(dst, frame, u, v);
        }
    }
}

__private void frame_yuv_to_rgb(g2dImage_s* dst, AVFrame* frame){
	for( size_t y = 0; y < dst->h; ++y){
		const unsigned row = g2d_row(dst, y);
		g2dColor_t* pix = g2d_color(dst, row, 0);
		const unsigned rowYFrame = frame->linesize[0] * y;
		const unsigned rowUFrame = frame->linesize[1] * (y>>1);
		const unsigned rowVFrame = frame->linesize[2] * (y>>1);
		__parallef
		for( size_t x = 0; x < dst->w; ++x ){
			const uint8_t Y = frame->data[0][rowYFrame + x];
			const uint8_t U = frame->data[1][rowUFrame + (x>>1)];
			const uint8_t V = frame->data[2][rowVFrame + (x>>1)];
			const unsigned char r = Y + 1.402 * (V-128);
			const unsigned char g = Y - 0.344 * (U-128) - 0.714*(V-128);
		   	const unsigned char b = Y + 1.772*(U-128);
			pix[x] = g2d_color_make(dst, 255, r, g, b);

		}
	}
}

#include <ef/delay.h>

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
		
		if( media->frameScaled ){
			//TODO use frame_resize_to
			frame_yuv_to_rgb(media->frame, media->avframe);	
			g2d_resize_to(media->frameScaled, media->frame);
			//frame_resize_to(media->frameScaled, media->avframe);
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
		dbg_error("DELAY: %luus", delay);
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


