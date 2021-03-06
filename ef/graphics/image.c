#include <ef/image.h>
#include <ef/memory.h>
#include <ef/vectorization.h>
#include <ef/mth.h>
#include <ef/vector.h>
#include <ef/err.h>
//#include <ef/stack.h>
//#include <ef/file.h>
//#include <png.h>
//#include <setjmp.h>

#define YR 0.299
#define YG 0.587
#define YB 0.114

#define CHANNEL 2000
//#define mm_alpha(N) ((CHANNEL)/((N)+1))
//#define mm_ahpla(A) ((CHANNEL/2)-A)
//#define mm_next(AL,LA,NW,OL) (((AL)*(NW)+(LA)*(OL))/(CHANNEL/2))

/************************************************************************************/
/*********************************  GENERIC FORMULAS ********************************/
/************************************************************************************/

void img_rgb_to_yuv8(unsigned char* y, unsigned char* u, unsigned char* v, unsigned char r ,unsigned char g, unsigned char b){
	*y = ((66  * r + 129* g + 25  * b + 128) >> 8) + 16;
	*u = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
	*v = ((112 * r - 94 * g - 18  * b + 128) >> 8) + 128;
}

void img_yuv8_to_rgb(unsigned char* r, unsigned char* g, unsigned char* b, unsigned char y ,unsigned char u, unsigned char v){
	unsigned const c = y - 16;
	unsigned const d = u - 128;
	unsigned const e = v - 128;
	unsigned R = (298 * c           + 409 * e + 128) >> 8;
	unsigned G = (298 * c - 100 * d - 208 * e + 128) >> 8;
	unsigned B = (298 * c + 516 * d           + 128) >> 8;
	*r = img_clip_h(R);
	*g = img_clip_h(G);
	*b = img_clip_h(B);
}

unsigned char img_rgb_to_gray(unsigned char r, unsigned char g, unsigned char b){
	return (7 * r + 38 * g + 19 * b + 32 ) >> 6;
}

int img_color_h(int R, int G, int B){
	double r = (double)R / 255.0;
	double g = (double)G / 255.0;
	double b = (double)B / 255.0;
	double m = MTH_3MIN(r,g,b);
	double M = MTH_3MAX(r,g,b);
	double H = 0.0;
UNSAFE_BEGIN("-Wfloat-equal")
	iassert(M-m != 0.0);
UNSAFE_END
	if( (long)r == (long)M ) H=(g-r)/(M-m);
	else if ( (long)g == (long)M ) H=2.0+(b-r)/(M-m);
	else if ( (long)b == (long)M ) H=4.0+(r-g)/(M-m);
	else iassert( 0 );
	H*=60;
	if(H<0) H+=360;
	return isnan(H)?0:round(H);
}

//from stackoverflow https://stackoverflow.com/questions/2792443/finding-the-centroid-of-a-polygon
g2dPoint_s g2d_centroid(g2dPoint_s* points){
	double centroidX = 0.0;
	double centroidY = 0.0;
	double signedArea = 0.0;
	double x0 = 0.0;
	double y0 = 0.0;
	double x1 = 0.0;
	double y1 = 0.0;
	double a = 0.0;

    vector_foreach(points, i){
        x0 = points[i].x;
        y0 = points[i].y;
        x1 = points[(i+1) % vector_count(points)].x;
        y1 = points[(i+1) % vector_count(points)].y;
        a = x0*y1 - x1*y0;
        signedArea += a;
        centroidX += (x0 + x1)*a;
        centroidY += (y0 + y1)*a;
    }

    signedArea *= 0.5;
    centroidX /= (6.0*signedArea);
    centroidY /= (6.0*signedArea);

	g2dPoint_s cent = {.x = centroidX, .y = centroidY };
    return cent;
}

	

/*******************************************************************************/
/************************************** RAW IMAGE ******************************/
/*******************************************************************************/

g2dImage_s* g2d_clone(unsigned w, unsigned h, g2dMode_e mode, unsigned char* pixels){
	g2dImage_s* img = mem_new(g2dImage_s);
	if( !img ) err_fail("malloc");
	img->h = h;
	img->w = w;
	img->p = w * 4;
	img->pixel = pixels;
	img->mode = mode;
	switch( mode ){
		default: case G2D_MODE_ARGB:
			img->sa = 24;
			img->sr = 16;
			img->sg = 8;
			img->sb = 0;
		break;
		case G2D_MODE_RGBA:
			img->sa = 0;
			img->sr = 24;
			img->sg = 16;
			img->sb = 8;
		break;
		case G2D_MODE_BGRA:
			img->sa = 0;
			img->sr = 8;
			img->sg = 16;
			img->sb = 24;
		break;
		case G2D_MODE_ABGR:
			img->sa = 24;
			img->sr = 0;
			img->sg = 8;
			img->sb = 16;
		break;
	}
	img->ma = 0xFF << img->sa;
	img->mr = 0xFF << img->sr;
	img->mg = 0xFF << img->sg;
	img->mb = 0xFF << img->sb;
	dbg_info("image: %u*%u", img->w, img->h);
	return img;
}

g2dImage_s* g2d_new(unsigned w, unsigned h, g2dMode_e mode){
	size_t size = h*(w*4);
	unsigned char* pixels = mem_many_aligned(unsigned char, &size, 16);
	if( !pixels ) err_fail("malloc");
	return g2d_clone(w, h, mode, pixels);
}

void g2d_free(g2dImage_s* img){
	if( img->pixel ) free( img->pixel );
	free(img);
}

void g2d_autofree(g2dImage_s** img){
	if( *img ) g2d_free(*img);
}

void g2d_ratio(int modeAWH, unsigned sw, unsigned sh, unsigned* w, unsigned* h){
	double scalingX = (double)sw / (double)*w;
	double scalingY = (double)sh / (double)*h;

	switch( modeAWH ){
		case 0:{
			double scaling = scalingX > scalingY ? scalingX : scalingY;
			*w = sw / scaling;
			*h = sh / scaling;
		}
		break;
		case 1:
			*w = sw / scalingX;
			*h = sh / scalingX;
		break;
		case 2:
			*w = sw / scalingY;
			*h = sh / scalingY;
		break;
	}
}

g2dColor_t g2d_color_gen(g2dMode_e mode, unsigned a, unsigned r, unsigned g, unsigned b){
	switch( mode ){
		case G2D_MODE_ARGB: return (a << 24) | (r << 16) | (g << 8 ) | (b << 0 );
		case G2D_MODE_RGBA: return (a << 0 ) | (r << 24) | (g << 16) | (b << 8 );
		case G2D_MODE_BGRA: return (a << 0 ) | (r << 8 ) | (g << 16) | (b << 24);
		case G2D_MODE_ABGR: return (a << 24) | (r << 0 ) | (g << 8 ) | (b << 16);
	}
	dbg_error("mode %d not support", mode);
	return 0;
}

__target_default 
__private void g2d_copy_default(g2dImage_s* dst, g2dImage_s* src){
	memcpy(dst->pixel, src->pixel, src->p * src->h);
}

__target_vectorization 
__private void g2d_copy_vectorize(g2dImage_s* dst, g2dImage_s* src){
	unsigned const h = src->h;
	unsigned const w = src->w;
	
	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, y);
		unsigned const drow = g2d_row(dst, y);
		vectorize_pair_loop(uint4_v, unsigned, &src->pixel[srow], 0, w, &dst->pixel[drow], 0, w,
			{
				*Bscalar++ = *Ascalar++;
			},
			{
				*Bvector++ = *Avector++;
			}
		);
	}
/*
	unsigned ali = (w * h) / 4;
	unsigned post = (w * h) % 4;
	unsigned i = w*h - post;
	uint4_v* vs = __is_aligned(src->pixel, sizeof(uint4_v));
	uint4_v* vd = __is_aligned(dst->pixel, sizeof(uint4_v));
	while( ali-->0 )
		*vd++ = *vs++;
	while( post-->0 ){
		vd[i] = vs[i];
		++i;
	}
*/
}

g2dImage_s* g2d_copy(g2dImage_s* src){
	g2dImage_s* img = g2d_new(src->w, src->h, src->mode);

	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
		g2d_copy_vectorize(img, src);
	}
	else{
		dbg_warning("not vectorized");
		g2d_copy_default(img, src);
	}
	return img;
}

__private err_t g2d_bitblt_validate(g2dImage_s* dst, g2dCoord_s* cod, g2dCoord_s* cos){
	if( cod->w + cod->x > dst->w ){
		if( cod->x > dst->w ){
			dbg_error("out of image dst x(%u) > dst w(%u)", cod->x, dst->w);	
			return -1;
		}
		cos->w = cod->w = dst->w - cos->x;
	}
	if( cod->y + cod->h > dst->h ){
		if( cod->y > dst->h ){
			dbg_error("out of image dst y(%u) > dst h(%u)", cod->y, dst->h);
			return -1;	
		}
		cos->h = cod->h = dst->h - cos->y;
	}
	if( cod->w != cos->w || cod->h != cos->h ){
		dbg_error("can't blt different size");
		return -1;
	}

	return 0;
}

__target_default 
__private void g2d_bitblt_default(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}

	unsigned const h = cod->h;
	unsigned const w = cod->w;
	unsigned const dx = cod->x;
	unsigned const sx = cos->x;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		g2dColor_t* cd = g2d_color(dst, drow, dx);
		g2dColor_t* sd = g2d_color(src, srow, sx);
		for( unsigned x = 0; x < w; ++x ){
			*cd++ = *sd++;
		}
	}
}

__target_vectorization
__private void g2d_bitblt_vectorize(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}

	unsigned const h = cod->h;
	unsigned const sw = cos->x + cos->w;
	unsigned const dw = cod->x + cod->w;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		vectorize_pair_loop(uint4_v, unsigned, &src->pixel[srow], cos->x, sw, &dst->pixel[drow], cod->x, dw,
			{
				*Bscalar++ = *Ascalar++;
			},
			{
				*Bvector++ = *Avector++;
			}
		);
	}
}

void g2d_bitblt(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorize");
		g2d_bitblt_vectorize(dst, cod, src, cos);
	}
	else{
		dbg_warning("not vectorized");
		g2d_bitblt_default(dst, cod, src, cos);
	}
}

__target_default 
__private void g2d_bitblt_xor_default(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}

	unsigned const h = cod->h;
	unsigned const w = cod->w;
	unsigned const dx = cod->x;
	unsigned const sx = cos->x;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		g2dColor_t* cd = g2d_color(dst, drow, dx);
		g2dColor_t* sd = g2d_color(src, srow, sx);
		for( unsigned x = 0; x < w; ++x ){
			*cd++ ^= *sd++;
		}
	}
}

__target_vectorization
__private void g2d_bitblt_xor_vectorize(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}

	unsigned const h = cod->h;
	unsigned const sw = cos->x + cos->w;
	unsigned const dw = cod->x + cod->w;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		vectorize_pair_loop(uint4_v, unsigned, &src->pixel[srow], cos->x, sw, &dst->pixel[drow], cod->x, dw,
			{
				*Bscalar++ ^= *Ascalar++;
			},
			{
				*Bvector++ ^= *Avector++;
			}
		);
	}
}

void g2d_bitblt_xor(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
//	g2d_bitblt_xor_default(dst, cod, src, cos);

		g2d_bitblt_xor_vectorize(dst, cod, src, cos);
	}
	else{
		dbg_warning("not vectorized");
		g2d_bitblt_xor_default(dst, cod, src, cos);
	}
}

__target_default
__private void g2d_bitblt_alpha_default(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}

	unsigned const h = cod->h;
	unsigned const w = cod->w;
	unsigned const dx = cod->x;
	unsigned const sx = cos->x;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		g2dColor_t* cd = g2d_color(dst, drow, dx);
		g2dColor_t* sd = g2d_color(src, srow, sx);
		for( unsigned x = 0; x < w; ++x){
			unsigned char Aalpha = g2d_color_alpha(src,*sd);
			unsigned char Ared = g2d_color_red(src,*sd);
			unsigned char Agreen = g2d_color_green(src,*sd);
			unsigned char Ablue = g2d_color_blue(src,*sd);
			unsigned char Bred = g2d_color_red(src,*cd);
			unsigned char Bgreen = g2d_color_green(src,*cd);
			unsigned char Bblue = g2d_color_blue(src,*cd);
			Bred = g2d_alpha_part(Aalpha, Ared, Bred);
			Bgreen = g2d_alpha_part(Aalpha, Agreen, Bgreen);
			Bblue = g2d_alpha_part(Aalpha, Ablue, Bblue);
			*cd = g2d_color_make(dst, 255, Bred, Bgreen, Bblue);
			++sd;
			++cd;
		}
	}
}

__target_vectorization
__private void g2d_bitblt_alpha_vectorize(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}

	unsigned const h = cod->h;
	unsigned const sw = cos->x + cos->w;
	unsigned const dw = cod->x + cod->w;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		vectorize_pair_loop(uint4_v, unsigned, &src->pixel[srow], cos->x, sw, &dst->pixel[drow], cod->x, dw,
			{
				unsigned char Aalpha = g2d_color_alpha(src,*Ascalar);
				unsigned char Ared = g2d_color_red(src,*Ascalar);
				unsigned char Agreen = g2d_color_green(src,*Ascalar);
				unsigned char Ablue = g2d_color_blue(src,*Ascalar);
				unsigned char Bred = g2d_color_red(src,*Bscalar);
				unsigned char Bgreen = g2d_color_green(src,*Bscalar);
				unsigned char Bblue = g2d_color_blue(src,*Bscalar);
				Bred = g2d_alpha_part(Aalpha, Ared, Bred);
				Bgreen = g2d_alpha_part(Aalpha, Agreen, Bgreen);
				Bblue = g2d_alpha_part(Aalpha, Ablue, Bblue);
				*Bscalar = g2d_color_make(dst, 255, Bred, Bgreen, Bblue);
				Ascalar++;
				Bscalar++;
			},
			{
				uint4_v vsa = (*Avector >> src->sa) & 0xFF;
				uint4_v vs = (*Avector >> src->sr) & 0xFF;
				uint4_v vd = (*Bvector >> dst->sr) & 0xFF;
				uint4_v vcr = g2d_alpha_part(vsa, vs, vd);
				vs = (*Avector >> src->sg) & 0xFF;
				vd = (*Bvector >> dst->sg) & 0xFF;
				uint4_v vcg = g2d_alpha_part(vsa, vs, vd);
				vs = (*Avector >> src->sb) & 0xFF;
				vd = (*Bvector >> dst->sb) & 0xFF;
				uint4_v vcb = g2d_alpha_part(vsa, vs, vd);
				*Bvector = (255<<dst->sa) | (vcr << dst->sr) | (vcg << dst->sg) | (vcb << dst->sb);

				Bvector++;
				Avector++;
			}
		);
	}
}

void g2d_bitblt_alpha(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
		g2d_bitblt_alpha_vectorize(dst, cod, src, cos);
	}
	else{
		dbg_warning("not vectorized");
		g2d_bitblt_alpha_default(dst, cod, src, cos);
	}
}

__target_default
__private void g2d_bitblt_channel_default(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos, unsigned int mask){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}

	unsigned const h = cod->h;
	unsigned const w = cod->w;
	unsigned const dx = cod->x;
	unsigned const sx = cos->x;

    __parallef
	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		g2dColor_t* cd = g2d_color(dst, drow, dx);
		g2dColor_t* sd = g2d_color(src, srow, sx);
		for( unsigned x = 0; x < w; ++x){
			*cd = (*cd & (~mask)) | (*sd & mask);
			++sd;
			++cd;
		}
	}
}

__target_vectorization
__private void g2d_bitblt_channel_vectorize(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos, unsigned int mask){
	if( g2d_bitblt_validate(dst, cod, cos) ){
		return;
	}
 
	uint4_v vmask = vector4_set_all(mask);

	unsigned const h = cod->h;
	unsigned const sw = cos->x + cos->w;
	unsigned const dw = cod->x + cod->w;

	__parallef
	for( unsigned y = 0; y < h; ++y ){
		unsigned const srow = g2d_row(src, cos->y + y);
		unsigned const drow = g2d_row(dst, cod->y + y);
		vectorize_pair_loop(uint4_v, unsigned, &src->pixel[srow], cos->x, sw, &dst->pixel[drow], cod->x, dw,
			{
				*Bscalar = (*Bscalar & (~mask)) | (*Ascalar & mask);
				++Bscalar;
				++Ascalar;
			},
			{
				*Bvector = (*Bvector & (~vmask)) | (*Avector & vmask);
				++Bvector;
				++Avector;
			}
		);
	}
}

void g2d_bitblt_channel(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos, unsigned int mask){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
		g2d_bitblt_channel_vectorize(dst, cod, src, cos, mask);
	}
	else{
		dbg_warning("not vectorized");
		g2d_bitblt_channel_default(dst, cod, src, cos, mask);
	}
}

__target_default
__private void g2d_clear_default(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord){
	unsigned const h = coord->h + coord->y;
	unsigned const w = coord->w;
	iassert( h <= img->h );
	iassert( coord->x + coord->w <= img->w );

	__parallef 	
	for( unsigned y = coord->y; y < h; ++y){
		unsigned const row = g2d_row(img, y);
		g2dColor_t* cd = g2d_color(img, row, coord->x);
		for( unsigned x = 0; x < w; ++x ){
			*cd++ = color;
		}
	}
}

__target_vectorization
__private void g2d_clear_vectorize(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord){
	unsigned const h = coord->h + coord->y;
	unsigned const w = coord->w + coord->x;
	uint4_v vcol = vector4_set_all(color);

	__parallef
	for( unsigned y = coord->y; y < h; ++y){
		unsigned const row = g2d_row(img, y);
		vectorize_loop(uint4_v, unsigned, &img->pixel[row], coord->x, w,
			{
				*scalar++ = color;
			},
			{			
				*vector++ = vcol;
			}
		);
	}
}

void g2d_clear(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
		g2d_clear_vectorize(img, color, coord);
	}
	else{
		dbg_warning("not vectorized");
		g2d_clear_default(img, color, coord);
	}
}

__target_default
__private void g2d_channel_set_dafault(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord, unsigned mask){
	unsigned const h = coord->h + coord->y;
	unsigned const w = coord->w + coord->x;

	__parallef for( unsigned y = coord->y; y < h; ++y){
		unsigned const row = g2d_row(img, y);
		g2dColor_t* scalar = g2d_color(img, row, coord->x);
		for( unsigned x = 0; x < w; ++x ){
			*scalar = (*scalar & (~mask)) | (color & mask);
			scalar++;
		}
	}
}

__target_vectorization
__private void g2d_channel_set_vectorize(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord, unsigned mask){
	unsigned const h = coord->h + coord->y;
	unsigned const w = coord->w + coord->x;
	uint4_v vcol = vector4_set_all(color);
	uint4_v vmask = vector4_set_all(mask);

	__parallef for( unsigned y = coord->y; y < h; ++y){
		unsigned const row = g2d_row(img, y);
		vectorize_loop(uint4_v, unsigned, &img->pixel[row], coord->x, w,
			{
				*scalar = (*scalar & (~mask)) | (color & mask);
				scalar++;
			},
			{			
				*vector = (*vector & (~vmask)) | (vcol & vmask);
				vector++;
			}
		);
	}
}

void g2d_channel_set(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord, unsigned mask){
	if( __cpu_supports_vectorization() ){
		g2d_channel_set_vectorize(img, color, coord, mask);
	}
	else{
		g2d_channel_set_dafault(img, color, coord, mask);
	}
}

__target_default
__private void g2d_luminance_default(g2dImage_s* img){
	unsigned const h = img->h;
	unsigned const w = img->w;
	
	__parallef for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(img, y);
		g2dColor_t* scalar = g2d_color(img, row, 0);
		for( unsigned x = 0; x < w; ++x ){
			unsigned const gray =(7 * g2d_color_red(img, *scalar) + 38 * g2d_color_green(img, *scalar) + 19 * g2d_color_blue(img, *scalar) + 32 ) >> 6;
			*scalar++ = g2d_color_make(img, 255, gray, gray, gray);
		}
	}
}

__target_vectorization
__private void g2d_luminance_vectorize(g2dImage_s* img){
	unsigned const h = img->h;
	unsigned const w = img->w;

	uint4_v vpr,vpg,vpb;
	
	__parallef for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(img, y);
		vectorize_loop(uint4_v, unsigned, &img->pixel[row], 0, w,
			{
				unsigned const gray =(7 * g2d_color_red(img, *scalar) + 38 * g2d_color_green(img, *scalar) + 19 * g2d_color_blue(img, *scalar) + 32 ) >> 6;
				*scalar++ = g2d_color_make(img, 255, gray, gray, gray);
			},
			{
				vpr = (*vector & img->mr) >> img->sr;
				vpg = (*vector & img->mg) >> img->sg;
				vpb = (*vector & img->mb) >> img->sb;
				vpr = ((vpr * 7 + vpg * 38 + vpb * 19) + 32) >> 6;
				*vector++ = (vpr << img->sr) | (vpr << img->sg) | (vpr << img->sb);
			}
		);
	}
}

void g2d_luminance(g2dImage_s* img){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
		g2d_luminance_vectorize(img);
	}
	else{
		dbg_warning("not vectorized");
		g2d_luminance_default(img);
	}
}

__private unsigned char g2d_black_white_distance(g2dImage_s* gray, g2dCoord_s* coord){
	unsigned char min = 255;
	unsigned char max = 0;
	unsigned const h = (coord->y + coord->h) > gray->h ? gray->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > gray->w ? gray->w - coord->x : coord->w;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(gray, coord->y + y);
		unsigned* ic = g2d_color(gray, row, coord->x);
		for( unsigned x = 0; x < w; ++x ){
			unsigned char red = g2d_color_red(gray,ic[x]);
			if( red < min ) min = red;
			if( red > max ) max = red;	
		}
	}

	return ((max-min)>>1)+min;
}

__target_default
__private void g2d_black_white_default(g2dImage_s* gray, g2dCoord_s* coord){
	unsigned const h = (coord->y + coord->h) > gray->h ? gray->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > gray->w ? gray->w - coord->x : coord->w;
	unsigned char d = g2d_black_white_distance(gray, coord);
	
	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(gray, coord->y + y);
		g2dColor_t* scalar = g2d_color(gray, row, coord->x);
		for( unsigned x = 0; x < w; ++x){
			*scalar = g2d_color_red(gray, *scalar) < d ? 0 : 0xFFFFFFFF;
			++scalar;
		}	
	}
}

__target_vectorization
__private void g2d_black_white_vectorize(g2dImage_s* gray, g2dCoord_s* coord){
	unsigned const h = (coord->y + coord->h) > gray->h ? gray->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > gray->w ? gray->w - coord->x : coord->w;
	unsigned char d = g2d_black_white_distance(gray, coord);
	uchar16_v vdis = vector16_set_all( d );
	
	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(gray, coord->y + y);
		vectorize_loop(uchar16_v, unsigned, &gray->pixel[row], coord->x, coord->x + w,
			{
				*scalar = g2d_color_red(gray, *scalar) < d ? 0 : 0xFFFFFFFF;
				++scalar;
			},
			{
				uchar16_v mask = *vector < vdis;
				*vector = ~mask;
				++vector;
			}
		);
	} 	
}

void g2d_black_white(g2dImage_s* gray, g2dCoord_s* coord){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
		g2d_black_white_vectorize(gray, coord);
	}
	else{
		dbg_warning("not vectorized");
		g2d_black_white_default(gray, coord);
	}
}

__target_default
__private void g2d_black_white_set_default(g2dImage_s* bw, g2dCoord_s* coord, g2dColor_t* colorAB){
	unsigned const h = (coord->y + coord->h) > bw->h ? bw->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > bw->w ? bw->w - coord->x : coord->w;

	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(bw, coord->y + y);
		g2dColor_t* scalar = g2d_color(bw, row, coord->x);
		for( unsigned x = 0; x < w; ++x){
			unsigned const id = g2d_color_red(bw,*scalar) == 0 ? 1 : 0;
			*scalar = colorAB[id];
			++scalar;
		}	
	}
}

__target_vectorization
__private void g2d_black_white_set_vectorize(g2dImage_s* bw, g2dCoord_s* coord, g2dColor_t* colorAB){
	unsigned const h = (coord->y + coord->h) > bw->h ? bw->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > bw->w ? bw->w - coord->x : coord->w;

	uint4_v vcola = vector4_set_all( colorAB[0] );
	uint4_v vcolb = vector4_set_all( colorAB[1] );

	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(bw, coord->y + y);
		vectorize_loop(uint4_v, unsigned, &bw->pixel[row], coord->x, coord->x + w,
			{
				unsigned const id = g2d_color_red(bw,*scalar) == 0 ? 1 : 0;
				*scalar = colorAB[id];
				++scalar;
			},
			{
				uint4_v vpa = (~*vector & vcola);
				uint4_v vpb = ( *vector & vcolb);
				*vector = vpa | vpb;
				++vector;
			}
		);
	}
}

void g2d_black_white_set(g2dImage_s* bw, g2dCoord_s* coord, g2dColor_t* colorAB){
	if( __cpu_supports_vectorization() ){
		dbg_info("vectorized");
		g2d_black_white_set_vectorize(bw, coord, colorAB);
	}
	else{
		dbg_warning("not vectorized");
		g2d_black_white_set_default(bw, coord, colorAB);
	}
}

void g2d_black_white_dominant(g2dColor_t* outAB, g2dImage_s* src, g2dImage_s* bw, g2dCoord_s* coord){
	unsigned const A = MM_ALPHA(CHANNEL, coord->w/2);
	unsigned const a = MM_AHPLA(CHANNEL, A);
	unsigned const h = (coord->y + coord->h) > src->h ? src->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > src->w ? src->w - coord->x : coord->w;
	iassert( src->w == bw->w && src->h == bw->h );
	
	outAB[0] = 0;
	outAB[1] = 0;
	int part[3][2] = {0};

	unsigned count[2] = {0,0};
	
	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(bw, coord->y + y);
		for( unsigned x = 0; x < w; ++x ){
			g2dColor_t* icbw = g2d_color(bw, row, coord->x + x);
			g2dColor_t* icsr = g2d_color(src, row, coord->x + x);
			unsigned id = g2d_color_red(bw, *icbw) == 0 ? 0 : 1;
			part[0][id] = MM_NEXT(CHANNEL, A, a, g2d_color_red(src, *icsr), part[0][id]);
			part[1][id] = MM_NEXT(CHANNEL, A, a, g2d_color_green(src, *icsr), part[1][id]);
			part[2][id] = MM_NEXT(CHANNEL, A, a, g2d_color_blue(src, *icsr), part[2][id]);
			count[id]++;
		}
	}
	outAB[0] = g2d_color_make(src, 255, part[0][0], part[1][0], part[2][0]);
	outAB[1] = g2d_color_make(src, 255, part[0][1], part[1][1], part[2][1]);

	if( count[0] == 0 ){
		outAB[0] = outAB[1];
	}
	else if( count[1] == 0 ){
		outAB[1] = outAB[0];
	}
}

__target_default
__private unsigned g2d_compare_similar_default(g2dImage_s* a, g2dCoord_s* ca, g2dImage_s* b, g2dCoord_s* cb){
	unsigned const ha = (ca->y + ca->h) > a->h ? a->h - ca->y : ca->h;
	unsigned const wa = (ca->x + ca->w) > a->w ? a->w - ca->x : ca->w;
	unsigned const hb = (cb->y + cb->h) > b->h ? b->h - cb->y : cb->h;
	unsigned const wb = (cb->x + cb->w) > b->w ? b->w - cb->x : cb->w;
	unsigned const w  = MTH_MIN(wa,wb);
	unsigned const h  = MTH_MIN(ha,hb);

	unsigned bits = 0;
	for( unsigned y = 0; y < h; ++y ){
		unsigned const ra = g2d_row(a, ca->y + y);
		unsigned const rb = g2d_row(b, cb->y + y);
		g2dColor_t* ica = g2d_color(a, ra, ca->x);
		g2dColor_t* icb = g2d_color(b, rb, cb->x);
		for( unsigned x = 0; x < w; ++x ){
			bits += FAST_BIT_COUNT( ((~ica[x]) & (~icb[x]))&0xFFFFFFFF );
		}
	}
	return bits;
}

__target_popcount
__private unsigned g2d_compare_similar_popcount(g2dImage_s* a, g2dCoord_s* ca, g2dImage_s* b, g2dCoord_s* cb){
	unsigned const ha = (ca->y + ca->h) > a->h ? a->h - ca->y : ca->h;
	unsigned const wa = (ca->x + ca->w) > a->w ? a->w - ca->x : ca->w;
	unsigned const hb = (cb->y + cb->h) > b->h ? b->h - cb->y : cb->h;
	unsigned const wb = (cb->x + cb->w) > b->w ? b->w - cb->x : cb->w;
	unsigned const w  = MTH_MIN(wa,wb);
	unsigned const h  = MTH_MIN(ha,hb);

	unsigned bits = 0;
	for( unsigned y = 0; y < h; ++y ){
		unsigned const ra = g2d_row(a, ca->y + y);
		unsigned const rb = g2d_row(b, cb->y + y);
		g2dColor_t* ica = g2d_color(a, ra, ca->x);
		g2dColor_t* icb = g2d_color(b, rb, cb->x);
		for( unsigned x = 0; x < w; ++x ){
			bits += FAST_BIT_COUNT( ((~ica[x]) & (~icb[x]))&0xFFFFFFFF );
		}
	}
	return bits;
}

unsigned g2d_compare_similar(g2dImage_s* a, g2dCoord_s* ca, g2dImage_s* b, g2dCoord_s* cb){
	if( __cpu_supports_popcount() ){
		return g2d_compare_similar_popcount(a, ca, b, cb);
	}
	else{
		return g2d_compare_similar_default(a, ca, b, cb);
	}
}

__target_default
__private unsigned g2d_bitcount_default(g2dImage_s* img, g2dCoord_s* coord){
	unsigned const h = (coord->y + coord->h) > img->h ? img->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > img->w ? img->w - coord->x : coord->w;

	unsigned bits = 0;
	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(img, coord->y + y);
		g2dColor_t* ic = g2d_color(img, row, coord->x);
		for( unsigned x = 0; x < w; ++x ){
			bits += FAST_BIT_COUNT( (~ic[x]) & 0xFFFFFF00 );
		}
	}
	return bits;
}

__target_popcount
__private unsigned g2d_bitcount_popcount(g2dImage_s* img, g2dCoord_s* coord){
	unsigned const h = (coord->y + coord->h) > img->h ? img->h - coord->y : coord->h;
	unsigned const w = (coord->x + coord->w) > img->w ? img->w - coord->x : coord->w;

	unsigned bits = 0;
	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(img, coord->y + y);
		g2dColor_t* ic = g2d_color(img, row, coord->x);
		for( unsigned x = 0; x < w; ++x ){
			bits += FAST_BIT_COUNT( (~ic[x]) & 0xFFFFFF00 );
		}
	}
	return bits;
}

unsigned g2d_bitcount(g2dImage_s* img, g2dCoord_s* coord){
	if( __cpu_supports_popcount() ){
		return g2d_bitcount_default(img, coord);
	}
	else{
		return g2d_bitcount_popcount(img, coord);
	}
}

__const inline __private float cubic_hermite(const float A, const float B, const float C, const float D, const float t){
	const float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
	const float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
	const float c = -A / 2.0f + C / 2.0f;
	return a*t*t*t + b*t*t + c*t + B;
}

__private g2dColor_t sample_bicubic(g2dImage_s* img, float u, float v){
	float x = (u * img->w)-0.5;
	int xint = (int)x;
	float xfract = x-floor(x);

	float y = (v * img->h) - 0.5;
	int yint = (int)y;
	float yfract = y - floor(y);
	
	if( xint == 0 ){
		++xint;
	}
	else if( (unsigned)xint >= img->w - 1 ){
		xint = img->w - 3;
	}
	else if( (unsigned)xint >= img->w - 2 ){
		xint = img->w - 3;
	}
	
	if( yint == 0 ){
		++yint;
	}
	else if( (unsigned)yint >= img->h - 1 ){
		yint = img->h - 3;
	}
	else if( (unsigned)yint >= img->h - 2 ){
		yint = img->h - 3;
	}
	
	g2dColor_t* p[4];
	unsigned row[4];
	float col[4][4];

	row[0] = g2d_row(img, yint-1);
	row[1] = g2d_row(img, yint);
	row[2] = g2d_row(img, yint+1);
	row[3] = g2d_row(img, yint+2);

	unsigned char alpha = g2d_color_alpha(img, *g2d_color(img, row[1], xint));
	//1266*668
	//1366*768
	//dbg_info("xint %d->%d yint %d->%d", xint, xint+2, yint,yint+2);

	for( unsigned y = 0; y < 4; ++y){
		p[0] = g2d_color(img, row[y], xint-1);
		p[1] = g2d_color(img, row[y], xint);
		p[2] = g2d_color(img, row[y], xint+1);
		p[3] = g2d_color(img, row[y], xint+2);
		col[y][0] = cubic_hermite( g2d_color_red(img, *p[0]), g2d_color_red(img, *p[1]), g2d_color_red(img, *p[2]), g2d_color_red(img, *p[3]), xfract);
		col[y][1] = cubic_hermite( g2d_color_green(img, *p[0]), g2d_color_green(img, *p[1]), g2d_color_green(img, *p[2]), g2d_color_green(img, *p[3]), xfract);
		col[y][2] = cubic_hermite( g2d_color_blue(img, *p[0]), g2d_color_blue(img, *p[1]), g2d_color_blue(img, *p[2]), g2d_color_blue(img, *p[3]), xfract);
	}

	float value = cubic_hermite(col[0][0], col[1][0], col[2][0], col[3][0], yfract);
	unsigned char red = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;
	value = cubic_hermite(col[0][1], col[1][1], col[2][1], col[3][1], yfract);
	unsigned char green = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;
	value = cubic_hermite(col[0][2], col[1][2], col[2][2], col[3][2], yfract);
	unsigned char blue = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;

	return g2d_color_make(img, alpha, red, green, blue);
} 
 
void g2d_resize_to(g2dImage_s* dst, g2dImage_s* src){
	dbg_info("resize %u*%u -> %u*%u", src->w, src->h, dst->w, dst->h);
	const unsigned w = dst->w;
	const unsigned h = dst->h;
	__parallef
	for( unsigned y = 0; y < h; ++y ){
		const float v = (const float)y / (const float)(h - 1);
		unsigned const row = g2d_row(dst, y);
		g2dColor_t* dcol = g2d_color(dst, row, 0);
		for( unsigned x = 0; x < w; ++x ){
            const float u = (const float)x / (const float)(w - 1);
            dcol[x] = sample_bicubic(src, u, v);
        }
    }
}      

g2dImage_s* g2d_resize(g2dImage_s* src, unsigned w, unsigned h){
	g2dImage_s* img = g2d_new(w, h, src->mode);
	g2d_resize_to(img, src);
	return img;
}

__private g2dColor_t sample_bicubic_alpha(g2dImage_s* img, float u, float v){
	float x = (u * img->w)-0.5;
	int xint = (int)x;
	float xfract = x-floor(x);

	float y = (v * img->h) - 0.5;
	int yint = (int)y;
	float yfract = y - floor(y);
	
	if( xint == 0 ){
		++xint;
	}
	else if( (unsigned)xint >= img->w - 1 ){
		xint = img->w - 3;
	}
	else if( (unsigned)xint >= img->w - 2 ){
		xint = img->w - 3;
	}
	
	if( yint == 0 ){
		++yint;
	}
	else if( (unsigned)yint >= img->h - 1 ){
		yint = img->h - 3;
	}
	else if( (unsigned)yint >= img->h - 2 ){
		yint = img->h - 3;
	}
	
	g2dColor_t* p[4];
	unsigned row[4];
	float col[4][4];

	row[0] = g2d_row(img, yint-1);
	row[1] = g2d_row(img, yint);
	row[2] = g2d_row(img, yint+1);
	row[3] = g2d_row(img, yint+2);

	for( unsigned y = 0; y < 4; ++y){
		p[0] = g2d_color(img, row[y], xint-1);
		p[1] = g2d_color(img, row[y], xint);
		p[2] = g2d_color(img, row[y], xint+1);
		p[3] = g2d_color(img, row[y], xint+2);
		col[y][0] = cubic_hermite( g2d_color_red(img, *p[0]), g2d_color_red(img, *p[1]), g2d_color_red(img, *p[2]), g2d_color_red(img, *p[3]), xfract);
		col[y][1] = cubic_hermite( g2d_color_green(img, *p[0]), g2d_color_green(img, *p[1]), g2d_color_green(img, *p[2]), g2d_color_green(img, *p[3]), xfract);
		col[y][2] = cubic_hermite( g2d_color_blue(img, *p[0]), g2d_color_blue(img, *p[1]), g2d_color_blue(img, *p[2]), g2d_color_blue(img, *p[3]), xfract);
		col[y][3] = cubic_hermite( g2d_color_alpha(img, *p[0]), g2d_color_alpha(img, *p[1]), g2d_color_alpha(img, *p[2]), g2d_color_alpha(img, *p[3]), xfract);
	}

	float value = cubic_hermite(col[0][0], col[1][0], col[2][0], col[3][0], yfract);
	unsigned char red = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;
	value = cubic_hermite(col[0][1], col[1][1], col[2][1], col[3][1], yfract);
	unsigned char green = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;
	value = cubic_hermite(col[0][2], col[1][2], col[2][2], col[3][2], yfract);
	unsigned char blue = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;
	value = cubic_hermite(col[0][3], col[1][3], col[2][3], col[3][3], yfract);
	unsigned char al = ( value < 0 ) ? 0 : value > 255 ? 255 : (int)value;

	return g2d_color_make(img, al, red, green, blue);
} 
 
void g2d_resize_to_alpha(g2dImage_s* dst, g2dImage_s* src){
	dbg_info("resize %u*%u -> %u*%u", src->w, src->h, dst->w, dst->h);
	const unsigned w = dst->w;
	const unsigned h = dst->h;
	__parallef
	for( unsigned y = 0; y < h; ++y ){
		const float v = (const float)y / (const float)(h - 1);
		unsigned const row = g2d_row(dst, y);
		g2dColor_t* dcol = g2d_color(dst, row, 0);
		for( unsigned x = 0; x < w; ++x ){
            const float u = (const float)x / (const float)(w - 1);
            dcol[x] = sample_bicubic_alpha(src, u, v);
        }
    }
}      

g2dImage_s* g2d_resize_alpha(g2dImage_s* src, unsigned w, unsigned h){
	g2dImage_s* img = g2d_new(w, h, src->mode);
	g2d_resize_to_alpha(img, src);
	return img;
}

g2dImage_s* g2d_rotate(g2dImage_s* src, unsigned cx, unsigned cy, float grad){
	float const rads = (grad * 3.14159265)/180.0;
	float const cosi = cos(rads);
	float const sine = sin(rads);

	float const p1x = (-(int)src->h * sine);
    float const p1y = (src->h * cosi);
    float const p2x = (src->w * cosi - src->h * sine);
    float const p2y = (src->h * cosi + src->w * sine);
    float const p3x = (src->w * cosi);
    float const p3y = (src->w * sine);

    float const minx = MTH_MIN(0,MTH_MIN(p1x,MTH_MIN(p2x,p3x)));
    float const miny = MTH_MIN(0,MTH_MIN(p1y,MTH_MIN(p2y,p3y)));
    float const maxx = MTH_MAX(0,MTH_MAX(p1x,MTH_MAX(p2x,p3x)));
    float const maxy = MTH_MAX(0,MTH_MAX(p1y,MTH_MAX(p2y,p3y)));

    unsigned const wi = (unsigned)ceil(maxx-minx);
    unsigned const he = (unsigned)ceil(maxy-miny);

    cx -= src->w / 2;
    cy -= src->h / 2;
	
    g2dImage_s* dst = g2d_new(wi, he, src->mode);
	g2dColor_t bk = g2d_color_make(dst, 255, 0, 0, 0);
	g2dCoord_s co = {.x = 0, .y = 0, .w = wi, .h = he };
	g2d_clear(dst, bk, &co);

    for ( unsigned y = 0; y < he; y++){
		int const ny = y - cy + miny;
		unsigned const drow = g2d_row(dst, y);
		g2dColor_t* dcol = g2d_color(dst, drow, 0);
        for ( unsigned x = 0; x < wi; x++){
		    int const nx = x - cx;
            unsigned const y0 = (unsigned)(ny * cosi - ( nx + minx) * sine) + cy;
			unsigned const x0 = (unsigned)((nx + minx) * cosi + ny * sine) + cx;
            if ( y0 < src->h && x0 < src->w ){
				unsigned const row = g2d_row(src, y0);
				g2dColor_t* scol = g2d_color(src, row, x0);
				dcol[x] = *scol;
			}
        }
    }
	return dst;
}

__private void supersampling_to(g2dImage_s* img, g2dImage_s* sam){
	//__g2d_free g2dImage_s* sam = g2d_resize(img, img->w * 2, img->h * 2);
	const unsigned h = sam->h;
	const unsigned w = sam->w;

	for( unsigned y = 0; y < h; y += 2){
		const unsigned rows[2] = {
			g2d_row(sam, y),
			g2d_row(sam, y+1)
		};
		g2dColor_t* pixels[2] = {
			g2d_color(sam, rows[0], 0),
			g2d_color(sam, rows[1], 0)
		};
		const unsigned row = g2d_row(img, y>>1);
		g2dColor_t* pixel = g2d_color(img, row, 0);
		for( unsigned x = 0; x < w; x += 2 ){
			unsigned r = g2d_color_red(sam, pixels[0][x]) + g2d_color_red(sam, pixels[0][x+1]) + 
						 g2d_color_red(sam, pixels[1][x]) + g2d_color_red(sam, pixels[1][x+1]);
			unsigned g = g2d_color_green(sam, pixels[0][x]) + g2d_color_green(sam, pixels[0][x+1]) + 
						 g2d_color_green(sam, pixels[1][x]) + g2d_color_green(sam, pixels[1][x+1]);
			unsigned b = g2d_color_blue(sam, pixels[0][x]) + g2d_color_blue(sam, pixels[0][x+1]) + 
						 g2d_color_blue(sam, pixels[1][x]) + g2d_color_blue(sam, pixels[1][x+1]);

			r >>= 2;
			g >>= 2;
			b >>= 2;
			pixel[x>>1] = g2d_color_make(img, 255, r, g, b);
		}
	}
}

void g2d_supersampling_to(g2dImage_s* img, unsigned mul){
	const unsigned mw = 1 << mul;
	g2dImage_s* sam = g2d_resize(img, img->w * mw, img->h * mw);
	for( unsigned n = 1; n < mul; ++n){
		g2dImage_s* tmp = g2d_new(sam->w >> 1, sam->h >> 1, img->mode);
		supersampling_to(tmp, sam);
		g2d_free(sam);
		sam = tmp;
	}
	supersampling_to(img, sam);
	free(sam);
}

__private void supersampling_alpha_to(g2dImage_s* img, g2dImage_s* sam){
	const unsigned h = sam->h;
	const unsigned w = sam->w;

	for( unsigned y = 0; y < h; y += 2){
		const unsigned rows[2] = {
			g2d_row(sam, y),
			g2d_row(sam, y+1)
		};
		g2dColor_t* pixels[2] = {
			g2d_color(sam, rows[0], 0),
			g2d_color(sam, rows[1], 0)
		};
		const unsigned row = g2d_row(img, y>>1);
		g2dColor_t* pixel = g2d_color(img, row, 0);
		for( unsigned x = 0; x < w; x += 2 ){
			unsigned a = g2d_color_alpha(sam, pixels[0][x]) + g2d_color_alpha(sam, pixels[0][x+1]) + 
						 g2d_color_alpha(sam, pixels[1][x]) + g2d_color_alpha(sam, pixels[1][x+1]);
			unsigned r = g2d_color_red(sam, pixels[0][x]) + g2d_color_red(sam, pixels[0][x+1]) + 
						 g2d_color_red(sam, pixels[1][x]) + g2d_color_red(sam, pixels[1][x+1]);
			unsigned g = g2d_color_green(sam, pixels[0][x]) + g2d_color_green(sam, pixels[0][x+1]) + 
						 g2d_color_green(sam, pixels[1][x]) + g2d_color_green(sam, pixels[1][x+1]);
			unsigned b = g2d_color_blue(sam, pixels[0][x]) + g2d_color_blue(sam, pixels[0][x+1]) + 
						 g2d_color_blue(sam, pixels[1][x]) + g2d_color_blue(sam, pixels[1][x+1]);

			a >>= 2;
			r >>= 2;
			g >>= 2;
			b >>= 2;
			pixel[x>>1] = g2d_color_make(img, a, r, g, b);
		}
	}
}

void g2d_supersampling_alpha_to(g2dImage_s* img, unsigned mul){
	const unsigned mw = 1 << mul;
	g2dImage_s* sam = g2d_resize_alpha(img, img->w * mw, img->h * mw);
	for( unsigned n = 1; n < mul; ++n){
		g2dImage_s* tmp = g2d_new(sam->w >> 1, sam->h >> 1, img->mode);
		supersampling_alpha_to(tmp, sam);
		g2d_free(sam);
		sam = tmp;
	}
	supersampling_alpha_to(img, sam);
	free(sam);
}

void g2d_char(g2dImage_s* dst, g2dCoord_s* coord, g2dImage_s* ch, g2dColor_t col){
	unsigned const h = (coord->y + ch->h) > dst->h ? dst->h - coord->y : ch->h;
	unsigned const w = (coord->x + ch->w) > dst->w ? dst->w - coord->x : ch->w;
	unsigned char Ared = g2d_color_red(ch, col);
	unsigned char Agreen = g2d_color_green(ch, col);
	unsigned char Ablue = g2d_color_blue(ch, col);

	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(dst, coord->y + y);
		unsigned const chrow = g2d_row(ch, y);
		g2dColor_t* scalarB = g2d_color(dst, row, coord->x);
		g2dColor_t* scalarA = g2d_color(ch, chrow, 0);
		for( unsigned x = 0; x < w; ++x){
			if( !g2d_color_red(ch, *scalarA) ){
				unsigned char Aalpha = g2d_color_alpha(ch, *scalarA);
				unsigned char Bred = g2d_color_red(dst, *scalarB);
				unsigned char Bgreen = g2d_color_green(dst, *scalarB);
				unsigned char Bblue = g2d_color_blue(dst, *scalarB);
				Bred = g2d_alpha_part(Aalpha, Ared, Bred);
				Bgreen = g2d_alpha_part(Aalpha, Agreen, Bgreen);
				Bblue = g2d_alpha_part(Aalpha, Ablue, Bblue);
				*scalarB = g2d_color_make(dst, 255, Bred, Bgreen, Bblue);
			}
			++scalarB;
			++scalarA;
		}	
	}
}

void g2d_char_indirect(g2dImage_s* dst, g2dCoord_s* coord, g2dImage_s* ch, g2dColor_t col){
	unsigned const h = (coord->y + ch->h) > dst->h ? dst->h - coord->y : ch->h;
	unsigned const w = (coord->x + ch->w) > dst->w ? dst->w - coord->x : ch->w;
	unsigned char Ared = g2d_color_red(ch, col);
	unsigned char Agreen = g2d_color_green(ch, col);
	unsigned char Ablue = g2d_color_blue(ch, col);

	for( unsigned y = 0; y < h; ++y ){
		unsigned const row = g2d_row(dst, coord->y + y);
		unsigned const chrow = g2d_row(ch, y);
		g2dColor_t* scalarB = g2d_color(dst, row, coord->x);
		g2dColor_t* scalarA = g2d_color(ch, chrow, 0);
		for( unsigned x = 0; x < w; ++x){	
			if( !g2d_color_red(ch, *scalarA) ){
				unsigned char Aalpha = g2d_color_alpha(ch, *scalarA);
				*scalarB = g2d_color_make(dst, Aalpha, Ared, Agreen, Ablue);
			}
			++scalarB;
			++scalarA;
		}	
	}
}

/*********************************************************************************/
/*************************************** PRIMITIVE *******************************/
/*********************************************************************************/

void g2d_point_rotate(unsigned* y, unsigned* x, unsigned cy, unsigned cx, double grad){
    float rads = (grad * 3.14159265)/180.0;
    float cosi = cos(rads);
    float sine = sin(rads);

    int ny = *y - cy;
    int nx = *x - cx;
    *y = (int)(ny * cosi - nx * sine) + cy;
    *x = (int)(nx * cosi + ny * sine) + cx;
}

#define _distance(A,B) (A>B?A-B:B-A)

#define _point(IMG, X, Y, C) do{\
		unsigned const __row__ = g2d_row(IMG, (Y));\
		g2dColor_t* __col__ = g2d_color(IMG, __row__, X);\
		*__col__ = (C);\
	}while(0)

#define _point_alpha(IMG, X, Y, C) do{\
		unsigned const __row__ = g2d_row(IMG, (Y));\
		g2dColor_t* __col__ = g2d_color(IMG, __row__, (X));\
		unsigned char __red__ = g2d_alpha_part(g2d_color_alpha(IMG,(C)), g2d_color_red(IMG,(C)), g2d_color_red(IMG,*__col__));\
		unsigned char __green__ = g2d_alpha_part(g2d_color_alpha(IMG,(C)), g2d_color_green(IMG,(C)), g2d_color_green(IMG,*__col__));\
		unsigned char __blue__ = g2d_alpha_part(g2d_color_alpha(IMG,(C)), g2d_color_blue(IMG,(C)), g2d_color_blue(IMG,*__col__));\
		*__col__ = g2d_color_make(IMG, g2d_color_alpha(IMG,*__col__), __red__, __green__, __blue__);\
	}while(0)

#define _point_inside(IMG, X, Y, C) do{\
		if( (X) < (IMG)->w && (Y) < (IMG)->h ){\
			_point(IMG, (X), (Y), (C));\
		}\
	}while(0)

#define _point_alpha_inside(IMG, X, Y, C) do{\
		if( (X) < (IMG)->w && (Y) < (IMG)->h ){\
			_point_alpha(IMG, (X), (Y), (C));\
		}\
	}while(0)

void g2d_points(g2dImage_s* img, g2dPoint_s* points, g2dColor_t* colors, size_t count){
	for( size_t i = 0; i < count; ++i){
		_point(img, points[i].y, points[i].x, colors[i]);
	}
}

void g2d_hline(g2dImage_s* img, g2dPoint_s* st, unsigned x1, g2dColor_t col){
    unsigned sx,ex;
	if( st->y >= img->h ) return;
    if ( st->x > x1 ){
        sx = x1;
        ex = st->x;
    }
    else{
        sx = st->x;
        ex = x1;
    }
 
	if( sx >= img->w ) return;   
    if( ex >= img->w ) ex = img->w - 1;

	unsigned const row = g2d_row(img, st->y);
	g2dColor_t* xc = g2d_color(img, row, sx);
	
	for( unsigned ix = sx; ix <= ex; ++ix){
		*xc = col;
		++xc;
	}
}

void g2d_vline(g2dImage_s* img, g2dPoint_s* st, unsigned y1, g2dColor_t col){
    unsigned sy,ey;
    
    if ( st->y > y1 ){
        sy = y1;
        ey = st->y;
    }
    else{
        sy = st->y;
        ey = y1;
    }
    
    if( ey >= img->h ) ey = img->h - 1;
    
	for( unsigned iy = sy; iy <= ey; ++iy){
		unsigned const row = g2d_row(img, iy);
		g2dColor_t* xc = g2d_color(img, row, st->x);
		*xc = col;
	}
}

void g2d_bline(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col){
    int i, deltax, deltay, numpixels; 
    int d, dinc1, dinc2; 
    int x, xinc1, xinc2; 
    int y, yinc1, yinc2; 
    
    int x0 = st->x;
    int y0 = st->y;
    int x1 = en->x;
	int y1 = en->y;

    deltax = (x1 > x0 ) ? x1 - x0 : x0 - x1;
    deltay = (y1 > y0 ) ? y1 - y0 : y0 - y1;
 
    if (deltax >= deltay) { 
        numpixels = deltax + 1;
        d = (2 * deltay) - deltax;
        dinc1 = deltay * 2;
        dinc2 = (deltay - deltax) * 2;
        xinc1 = 1; 
        xinc2 = 1; 
        yinc1 = 0; 
        yinc2 = 1; 
    } 
    else{ 
        numpixels = deltay + 1;
        d = (2 * deltax) - deltay;
        dinc1 = deltax * 2;
        dinc2 = (deltax - deltay) * 2;
        xinc1 = 0;
        xinc2 = 1;
        yinc1 = 1;
        yinc2 = 1;
    }
 
    if (x0 > x1) {
        xinc1 = -xinc1;
        xinc2 = -xinc2;
    }
    if (y0 > y1) {
        yinc1 = -yinc1;
        yinc2 = -yinc2;
    }
    
    x = x0;
    y = y0;
       
    for(i = 0; i < numpixels; ++i){
        _point(img, x, y, col);
        if( d < 0 ){
            d += dinc1;
            x += xinc1;
            y += yinc1;
        }
		else{
            d += dinc2;
            x += xinc2;
            y += yinc2;
        }
    }
}

void g2d_bline_antialiased(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col){
	int x0 = st->x;
	int x1 = en->x;
	int y0 = st->y;
	int y1 = en->y;	
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = dx-dy, e2, x2;                       /* error value e_xy */
	int ed = dx+dy == 0 ? 1 : sqrt((float)dx*dx+(float)dy*dy);

	for(;;){
		col = g2d_color_alpha_set(img, col,  255 - (255 * abs(err-dx+dy)/ed));
		_point_alpha(img, x0, y0, col);
		e2 = err; x2 = x0;
		if (2*e2 >= -dx) {
			if (x0 == x1) break;
			if (e2+dy < ed){
				col = g2d_color_alpha_set(img, col, 255- (255*(e2+dy)/ed));
				_point_alpha(img, x0, y0+sy, col);
			}
			err -= dy; x0 += sx;
		}
		if (2*e2 <= dy) {
			if (y0 == y1) break;
			if (dx-e2 < ed){
				col = g2d_color_alpha_set(img, col, 255-(255*(dx-e2)/ed));
				_point_alpha(img, x2+sx, y0, col);
			}
			err += dx; y0 += sy;
		}
	}
}

void g2d_line(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col, int antialiased){
	if( st->x == en->x ){
		g2d_vline(img, st, en->y, col);
	}
	else if( st->y == en->y ){
		g2d_hline(img, st, en->x, col);
	}
	else if( antialiased ){
		g2d_bline_antialiased(img, st, en, col);
	}
	else{
		g2d_bline(img, st, en, col);
	}
}

void g2d_qubezier_antialiased(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t color){
	int x0 = p0->x;
	int y0 = p0->y;
	int x1 = p1->x;
	int y1 = p1->y;
	int x2 = p2->x;
	int y2 = p2->y;
	int sx = x2-x1, sy = y2-y1;
	long xx = x0-x1, yy = y0-y1, xy;
	double dx, dy, err, ed, cur = xx*sy-yy*sx;
	iassert(xx*sx <= 0 && yy*sy <= 0);
	if( sx*(long)sx+sy*(long)sy > xx*xx+yy*yy ){
		x2 = x0; x0 = sx+x1; y2 = y0; y0 = sy+y1; cur = -cur;
	}
	if( (int)cur != 0 ){
		xx += sx; xx *= sx = x0 < x2 ? 1 : -1;
		yy += sy; yy *= sy = y0 < y2 ? 1 : -1;
		xy = 2*xx*yy; xx *= xx; yy *= yy;
		if( cur*sx*sy < 0 ){
			xx = -xx; yy = -yy; xy = -xy; cur = -cur;
		}
		dx = 4.0*sy*(x1-x0)*cur+xx-xy;
		dy = 4.0*sx*(y0-y1)*cur+yy-xy;
		xx += xx; yy += yy; err = dx+dy+xy;
		do{
			cur = MTH_MIN(dx+xy,-xy-dy);
			ed = MTH_MAX(dx+xy,-xy-dy);
			ed = 255/(ed+2*ed*cur*cur/(4*ed*ed+cur*cur));
			int alpha = 255 - ed*fabs(err-dx-dy-xy);
			if( alpha > 255 ) alpha = 255;
			if( alpha < 0 ) alpha = 0;
			color = g2d_color_alpha_set(img, color, alpha);
			_point_alpha(img, x0, y0, color);
			if( x0 == x2 && y0 == y2 ) return;
			x1 = x0; cur = dx-err; y1 = 2*err+dy < 0;
			if( 2*err+dx > 0 ){
				if( err-dy < ed ){
					int alpha = 255 - ed*fabs(err-dy);
					if( alpha > 255 ) alpha = 255;
					if( alpha < 0 ) alpha = 0;
					color = g2d_color_alpha_set(img, color, alpha);
					_point_alpha(img, x0, y0+sy, color);
				}
				x0 += sx; dx -= xy; err += dy += yy;
			}
			if( y1 ){
				if( cur < ed ){
					int alpha = 255 - ed*fabs(cur);
					if( alpha > 255 ) alpha = 255;
					if( alpha < 0 ) alpha = 0;
					color = g2d_color_alpha_set(img, color, alpha);
					_point_alpha(img, x1+sx, y0, color);
				}
				y0 += sy; dy -= xy; err += dx += xx;
			}
		}while( dy < dx );
	}
	g2dPoint_s PS = { .x = x0, .y = y0};
	g2dPoint_s PE = { .x = x2, .y = y2};
	g2d_line(img, &PS, &PE, color, 1);
}

void g2d_cubezier_normal(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dPoint_s* p3, g2dColor_t col){
 	float dt;
 	int	ii=0;
    int np;
    int x0 = p0->x;
    int y0 = p0->y;
    int rx0 = p1->x;
    int ry0 = p1->y;
    int rx1 = p2->x;
    int ry1 = p2->y;
    int x1 = p3->x;
    int y1 = p3->y;
    
    if ( (unsigned)x0 < img->w && (unsigned)x0 < img->h )
        _point(img, x0, y0, col);
    
    if ( (unsigned)x1 < img->w && (unsigned)y1 < img->h )
        _point(img, x1, y1, col);
    
    np  = _distance(x0,x1);
    np += _distance(x0,rx0);
    np += _distance(rx0,rx1);
    np += _distance(rx1,x1);
    np += _distance(y0,y1);
    np += _distance(y0,ry0);
    np += _distance(ry0,ry1);
    np += _distance(ry1,y1);

 	dt = 1.0 / ( np - 1 );

    int rx,ry;
    float ax,bx,cx;
 	float ay,by,cy;
 	float tSquared, tCubed;
    float t;

 	cx = 3.0 * (rx0 - x0);
 	bx = 3.0 * (rx1 - rx0) - cx;
 	ax = x1 - x0 - cx - bx;

 	cy = 3.0 * (ry0 - y0);
 	by = 3.0 * (ry1 - ry0) - cy;
 	ay = y1 - y0 - cy - by;

    //t = ii * dt;
    //tSquared = t * t;
    //tCubed = tSquared * t;

    for( ii = 0; ii < np; ii++ ){
        t = ii * dt;
        tSquared = t * t;
        tCubed = tSquared * t;

        rx = (ax * tCubed) + (bx * tSquared) + (cx * t) + x0;
        ry = (ay * tCubed) + (by * tSquared) + (cy * t) + y0;

        if( (unsigned)rx < img->w && (unsigned)ry < img->h ){
            _point( img, rx, ry, col);
        }
 	}
}

__private void g2d_cubezier_segment_antialiased(g2dImage_s* img, int x0, int y0, float x1, float y1, float x2, float y2, int x3, int y3, g2dColor_t color){
	int f, fx, fy, leg = 1;
	int sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
	float xc = -fabs(x0+x1-x2-x3), xa = xc-4*sx*(x1-x2), xb = sx*(x0-x1-x2+x3);
	float yc = -fabs(y0+y1-y2-y3), ya = yc-4*sy*(y1-y2), yb = sy*(y0-y1-y2+y3);
	double ab, ac, bc, ba, xx, xy, yy, dx, dy, ex, px, py, ed, ip, EP = 0.01;

	iassert((x1-x0)*(x2-x3) < EP && ((x3-x0)*(x1-x2) < EP || xb*xb < xa*xc+EP));
	iassert((y1-y0)*(y2-y3) < EP && ((y3-y0)*(y1-y2) < EP || yb*yb < ya*yc+EP));
	if( (int)xa == 0 && (int)ya == 0 ){
		sx = floor((3*x1-x0+1)/2); sy = floor((3*y1-y0+1)/2);
		g2dPoint_s P0 = {.x = x0, .y = y0};
		g2dPoint_s P1 = {.x = sx, .y = sy};
		g2dPoint_s P2 = {.x = x3, .y = y3};
		g2d_qubezier_antialiased(img, &P0, &P1, &P2, color);
		return;
	}
	x1 = (x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+1;
	x2 = (x2-x3)*(x2-x3)+(y2-y3)*(y2-y3)+1;
	do{
		ab = xa*yb-xb*ya; ac = xa*yc-xc*ya; bc = xb*yc-xc*yb;
		ip = 4*ab*bc-ac*ac;
		ex = ab*(ab+ac-3*bc)+ac*ac;
		f = ex > 0 ? 1 : sqrt(1+1024/x1);
		ab *= f; ac *= f; bc *= f; ex *= f*f;
		xy = 9*(ab+ac+bc)/8; ba = 8*(xa-ya);
		dx = 27*(8*ab*(yb*yb-ya*yc)+ex*(ya+2*yb+yc))/64-ya*ya*(xy-ya);
		dy = 27*(8*ab*(xb*xb-xa*xc)-ex*(xa+2*xb+xc))/64-xa*xa*(xy+xa);
		xx = 3*(3*ab*(3*yb*yb-ya*ya-2*ya*yc)-ya*(3*ac*(ya+yb)+ya*ba))/4;
		yy = 3*(3*ab*(3*xb*xb-xa*xa-2*xa*xc)-xa*(3*ac*(xa+xb)+xa*ba))/4;
		xy = xa*ya*(6*ab+6*ac-3*bc+ba); ac = ya*ya; ba = xa*xa;
		xy = 3*(xy+9*f*(ba*yb*yc-xb*xc*ac)-18*xb*yb*ab)/8;
		if( ex < 0 ){
			dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; ba = -ba;
		}
		ab = 6*ya*ac; ac = -6*xa*ac; bc = 6*ya*ba; ba = -6*xa*ba;
		dx += xy; ex = dx+dy; dy += xy;
		for( fx = fy = f; x0 != x3 && y0 != y3; ){
			y1 = MTH_MIN(xy-dx,dy-xy);
			ed = MTH_MAX(xy-dx,dy-xy);
			ed = f*(ed+2*ed*y1*y1/(4*ed*ed+y1*y1)); 
			y1 = 255*fabs(ex-(f-fx+1)*dx-(f-fy+1)*dy+f*xy)/ed;
			if( y1 < 256 ){
				int alpha = 255 - y1;
				if( alpha > 255 ) alpha = 255;
				if( alpha < 0 ) alpha = 0;
				color = g2d_color_alpha_set(img, color, alpha);
				_point_alpha(img, x0, y0, color);
			}
			px = fabs(ex-(f-fx+1)*dx+(fy-1)*dy);
			py = fabs(ex+(fx-1)*dx-(f-fy+1)*dy);
			y2 = y0;
			do{
				if( ip >= -EP )
					if( dx+xx > xy || dy+yy < xy ) goto exit;
				y1 = 2*ex+dx;
				if( 2*ex+dy > 0 ){
					fx--; ex += dx += xx; dy += xy += ac; yy += bc; xx += ab;
				}
				if( y1 <= 0 ){
					fy--; ex += dy += yy; dx += xy += bc; xx += ac; yy += ba;
				}
			}while( fx > 0 && fy > 0 );
			if( 2*fy <= f ){
				if( py < ed ){
					int alpha = 255 - 255*py/ed;
					if( alpha > 255 ) alpha = 255;
					if( alpha < 0 ) alpha = 0;
					color = g2d_color_alpha_set(img, color, alpha);
					_point_alpha(img, x0 + sx, y0, color);
				}
				y0 += sy; fy += f;
			}
			if( 2*fx <= f ){
				if( px < ed ){
					int alpha = 255 - 255*px/ed;
					if( alpha > 255 ) alpha = 255;
					if( alpha < 0 ) alpha = 0;
					color = g2d_color_alpha_set(img, color, alpha);
					_point_alpha(img, x0, y2 + sy, color);
				}
				x0 += sx; fx += f;
			}
		}
		break;
	exit:
		if (2*ex < dy && 2*fy <= f+2) {
			if( py < ed ){
				int alpha = 255 - 255*py/ed;
				if( alpha > 255 ) alpha = 255;
				if( alpha < 0 ) alpha = 0;
				color = g2d_color_alpha_set(img, color, alpha);
				_point_alpha(img, x0 + sx, y0, color);
			}
			y0 += sy;
		}
		if( 2*ex > dx && 2*fx <= f+2 ){
			if( px < ed ){
				int alpha = 255 - 255*px/ed;
				if( alpha > 255 ) alpha = 255;
				if( alpha < 0 ) alpha = 0;
				color = g2d_color_alpha_set(img, color, alpha);
				_point_alpha(img, x0, y2 + sy, color);
			}
			x0 += sx;
		}
		xx = x0; x0 = x3; x3 = xx; sx = -sx; xb = -xb;
		yy = y0; y0 = y3; y3 = yy; sy = -sy; yb = -yb; x1 = x2;
	}while( leg-- );
	
	g2dPoint_s PS = { .x = x0, .y = y0};
	g2dPoint_s PE = { .x = x3, .y = y3};
	g2d_line(img, &PS, &PE, color, 1);
}

void g2d_cubezier_antialiased(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dPoint_s* p3, g2dColor_t col){
	int x0 = p0->x;
	int y0 = p0->y;
	int x1 = p1->x;
	int y1 = p1->y;
	int x2 = p2->x;
	int y2 = p2->y;
	int x3 = p3->x;
	int y3 = p3->y;
	int n = 0, i = 0;
	long xc = x0+x1-x2-x3, xa = xc-4*(x1-x2);
	long xb = x0-x1-x2+x3, xd = xb+4*(x1+x2);
	long yc = y0+y1-y2-y3, ya = yc-4*(y1-y2);
	long yb = y0-y1-y2+y3, yd = yb+4*(y1+y2);
	float fx0 = x0, fx1, fx2, fx3, fy0 = y0, fy1, fy2, fy3;
	double t1 = xb*xb-xa*xc, t2, t[5];
	if( xa == 0 ){
		if( abs((int)xc) < 2*abs((int)xb) ) t[n++] = xc/(2.0*xb);
	}
	else if( t1 > 0.0 ){
		t2 = sqrt(t1);
		t1 = (xb-t2)/xa; 
		if (fabs(t1) < 1.0) t[n++] = t1;
		t1 = (xb+t2)/xa; 
		if (fabs(t1) < 1.0) t[n++] = t1;
	}
	t1 = yb*yb-ya*yc;
	if( ya == 0 ){
		if( abs((int)yc) < 2*abs((int)yb) ) t[n++] = yc/(2.0*yb);
	}
	else if( t1 > 0.0 ){
		t2 = sqrt(t1);
		t1	= (yb-t2)/ya;
		if( fabs(t1) < 1.0 ) t[n++] = t1;
		t1 = (yb+t2)/ya;
		if( fabs(t1) < 1.0 ) t[n++] = t1;
	}
	for (i = 1; i < n; i++)
		if ((t1 = t[i-1]) > t[i]) { t[i-1] = t[i]; t[i] = t1; i = 0; }
	t1 = -1.0; t[n] = 1.0;
	for (i = 0; i <= n; i++){
		t2 = t[i];
		fx1 = (t1*(t1*xb-2*xc)-t2*(t1*(t1*xa-2*xb)+xc)+xd)/8-fx0;
		fy1 = (t1*(t1*yb-2*yc)-t2*(t1*(t1*ya-2*yb)+yc)+yd)/8-fy0;
		fx2 = (t2*(t2*xb-2*xc)-t1*(t2*(t2*xa-2*xb)+xc)+xd)/8-fx0;
		fy2 = (t2*(t2*yb-2*yc)-t1*(t2*(t2*ya-2*yb)+yc)+yd)/8-fy0;
		fx0 -= fx3 = (t2*(t2*(3*xb-t2*xa)-3*xc)+xd)/8;
		fy0 -= fy3 = (t2*(t2*(3*yb-t2*ya)-3*yc)+yd)/8;
		x3 = floor(fx3+0.5); y3 = floor(fy3+0.5);
		if( (int)fx0 != 0 ) { fx1 *= fx0 = (x0-x3)/fx0; fx2 *= fx0; }
		if( (int)fy0 != 0 ) { fy1 *= fy0 = (y0-y3)/fy0; fy2 *= fy0; }
		if( x0 != x3 || y0 != y3){
			g2d_cubezier_segment_antialiased(img, x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3, col);
		}
		x0 = x3; y0 = y3; fx0 = fx3; fy0 = fy3; t1 = t2;
	}
}

void g2d_cubezier(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dPoint_s* p3, g2dColor_t col, int antialiased){
	if( antialiased ) g2d_cubezier_antialiased(img, p0, p1, p2, p3, col);
	else g2d_cubezier_normal(img, p0, p1, p2, p3, col);
}

void g2d_triangle_fill(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t col){
	unsigned t1x, t2x, y, minx, maxx, t1xp, t2xp;
	uint8_t changed1 = 0;
	uint8_t changed2 = 0;
	int e1,e2,i,signx1,signx2,dx1,dy1,dx2,dy2;
    unsigned x0 = p0->x, y0 = p0->y;
    unsigned x1 = p1->x, y1 = p1->y;
	unsigned x2 = p2->x, y2 = p2->y;
    
	if ( y0 > y1 ) { SWAP(y0,y1); SWAP(x0,x1); }
	if ( y0 > y2 ) { SWAP(y0,y2); SWAP(x0,x2); }
	if ( y1 > y2 ) { SWAP(y1,y2); SWAP(x1,x2); }

	t1x = t2x = x0;
    y = y0;

	dx1 = x1 - x0; 
    if( dx1 < 0 ) {dx1 = -dx1; signx1 = -1;} else signx1=1;
	dy1 = y1 - y0;
 
	dx2 = x2 - x0;
    if(dx2 < 0) {dx2 = -dx2; signx2 = -1;} else signx2=1;
	dy2 = y2 - y0;
	
	if (dy1 > dx1) {
        SWAP(dx1,dy1);
		changed1 = 1;
	}
	if (dy2 > dx2){
        SWAP(dy2,dx2);
		changed2 = 1;
	}
	
	e2 = dx2 >> 1;
    if(y0 == y1) goto next;
    e1 = dx1 >> 1;
	
	for ( i = 0; i < dx1;){
		t1xp = 0; t2xp = 0;
		if(t1x<t2x) {
            minx = t1x;
            maxx = t2x;
        }
		else{
            minx = t2x;
            maxx = t1x;
        }
        
		while( i < dx1 ){
			++i;			
			e1 += dy1;
	   	   	while (e1 >= dx1) {
				e1 -= dx1;
                if (changed1) t1xp = signx1;
				else          goto next1;
			}
			if (changed1) break;
			else t1x += signx1;
		}
	next1:
		while (1) {
			e2 += dy2;		
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp=signx2;
				else          goto next2;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next2:
		if(minx > t1x) minx = t1x; 
        if(minx > t2x) minx = t2x;
		if(maxx < t1x) maxx = t1x; 
        if(maxx < t2x) maxx = t2x;
        g2dPoint_s p = { .x = minx, .y = y };
		g2d_hline(img, &p, maxx, col);
		if( !changed1 ) t1x += signx1;
		t1x += t1xp;
		if( !changed2 ) t2x += signx2;
		t2x += t2xp;
    	y += 1;
		if(y == y1) break;
	}
	next:
	dx1 = (x2 - x1); if(dx1 < 0) { dx1 = -dx1; signx1 = -1; } else signx1=1;
	dy1 = (y2 - y1);
	t1x=x1;
 
	if (dy1 > dx1) {  
        SWAP(dy1,dx1);
		changed1 = 1;
	} 
    else{ 
        changed1=0;
	}

	e1 = dx1 >> 1;
	
	for ( i = 0; i <= dx1; ++i){
		t1xp = 0; t2xp = 0;
		if(t1x < t2x) {
            minx = t1x; 
            maxx = t2x;
        }
		else{
            minx = t2x;
            maxx = t1x;
        }
	    
		while(i < dx1){
    		e1 += dy1;
	   	   	while (e1 >= dx1) {
				e1 -= dx1;
   	   	   	   	if (changed1) { t1xp = signx1; break; }
				else          goto next3;
			}
			if (changed1) break;
			else   	   	  t1x += signx1;
			if(i<dx1) ++i;
		}
	next3:
		while (t2x != x2) {
			e2 += dy2;
	   	   	while (e2 >= dx2) {
				e2 -= dx2;
				if(changed2) t2xp=signx2;
				else          goto next4;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}	   	   
	next4:
		if ( minx > t1x ) minx = t1x; 
        if ( minx > t2x ) minx = t2x;
		if ( maxx < t1x ) maxx = t1x; 
        if ( maxx < t2x ) maxx = t2x;
		g2dPoint_s p = { .x = minx, .y = y };
		g2d_hline(img, &p, maxx, col);
		if ( !changed1 ) t1x += signx1;
		t1x += t1xp;
		if ( !changed2 ) t2x += signx2;
		t2x += t2xp;
    	y += 1;
		if( y>y2 ) return;
	}
}


void g2d_triangle(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t col, int antialiased){
    g2d_line(img, p0, p1, col, antialiased);
    g2d_line(img, p0, p2, col, antialiased);
	g2d_line(img, p1, p2, col, antialiased); 
}

void g2d_rect(g2dImage_s* img, g2dCoord_s* rc, g2dColor_t col){
    g2dPoint_s p0;
	p0.x = rc->x;
	p0.y = rc->y;
	g2d_hline(img, &p0, rc->x + rc->w, col);
	g2d_vline(img, &p0, rc->y + rc->h, col);
	p0.y += rc->h;
	p0.x += rc->w;
	g2d_hline(img, &p0, rc->x, col);
	g2d_hline(img, &p0, rc->y, col);
}

void g2d_rect_fill(g2dImage_s* img, g2dCoord_s* rc, g2dColor_t col){
    g2d_clear(img, col, rc);
}

void g2d_circle_normal(g2dImage_s* img, g2dPoint_s* cx, unsigned r, g2dColor_t col){
	iassert(r);
    int x,y,xc,yc,re;

    x = r;
    y = 0;
    xc = 1 - 2 * r;
    yc = 1;
    re = 0;
   
    while( x >=y ){
		_point_inside(img, cx->x + x, cx->y + y, col);
        _point_inside(img, cx->x - x, cx->y + y, col);
        _point_inside(img, cx->x + x, cx->y - y, col);
        _point_inside(img, cx->x - x, cx->y - y, col);
        _point_inside(img, cx->x + y, cx->y + x, col);
        _point_inside(img, cx->x - y, cx->y + x, col);
        _point_inside(img, cx->x + y, cx->y - x, col);
        _point_inside(img, cx->x - y, cx->y - x, col);
        ++y;
        re += yc;
        yc += 2;
        if ( 2 * re + xc > 0){
            --x;
            re += xc;
            xc += 2;
        }	
    }	
}

/*Bresenham*/
void g2d_circle_antialiased(g2dImage_s* img, g2dPoint_s* cx, int r, g2dColor_t col){
	int x = r, y = 0;
	int i, x2, e2, err = 2-2*r;
	r = 1-err;
	for(;;){
		i = 255*abs(err+2*(x+y)-2)/r;
		if( i < 0 ) i = 0;
		if( i > 255 ) i = 255;
		col = g2d_color_alpha_set(img, col,  255-i);
		_point_alpha_inside(img, cx->x + x, cx->y - y, col);
		_point_alpha_inside(img, cx->x + y, cx->y + x, col);
		_point_alpha_inside(img, cx->x - x, cx->y + y, col);
		_point_alpha_inside(img, cx->x - y, cx->y - x, col);
		if (x == 0) break;
		e2 = err; x2 = x;
		if (err > y){
			i = 255*(err+2*x-1)/r;
			if (i < 255) {
				col = g2d_color_alpha_set(img, col, 255- i);
				_point_alpha_inside(img, cx->x + x    , cx->y - y + 1, col);
				_point_alpha_inside(img, cx->x + y - 1, cx->y + x    , col);
				_point_alpha_inside(img, cx->x - x    , cx->y + y - 1, col);
				_point_alpha_inside(img, cx->x - y + 1, cx->y - x    , col);
			}
			err -= --x*2-1;
		}
		if (e2 <= x2--) { 
			i = 255*(1-2*y-e2)/r;
			if (i < 255) {
				col = g2d_color_alpha_set(img, col,  255-i);
				_point_alpha_inside(img, cx->x + x2, cx->y - y , col);
				_point_alpha_inside(img, cx->x + y , cx->y + x2, col);
				_point_alpha_inside(img, cx->x - x2, cx->y + y , col);
				_point_alpha_inside(img, cx->x - y , cx->y - x2, col);
			}
			err -= --y*2-1;
		}
	}
}

void g2d_circle(g2dImage_s* img, g2dPoint_s* cx, int r, g2dColor_t col, int antialiased){
	if( antialiased ) g2d_circle_antialiased(img, cx, r, col);
	else g2d_circle_normal(img, cx, r, col);
}

void g2d_circle_fill_antialiased(g2dImage_s* img, g2dPoint_s* cx, int r, g2dColor_t col){
	int x = r, y = 0;
	int i, x2, e2, err = 2-2*r;
	r = 1-err;
	g2dColor_t color = col;

	for(;;){
		i = 255*abs(err+2*(x+y)-2)/r;
		if( i < 0 ) i = 0;
		if( i > 255 ) i = 255;

		g2dPoint_s st;
		unsigned ex1;
		st.x = (int)cx->x - x > 0 ? cx->x - x : 0;
		st.y = (int)cx->y + y > 0 ? cx->y + y : 0;
		ex1 = (int)cx->x + x > 0 ? cx->x + x : 0;
		g2d_hline(img, &st, ex1, color);
		st.x = (int)cx->x + y > 0 ? cx->x + y : 0;
		st.y = (int)cx->y + x > 0 ? cx->y + x : 0;
		ex1 = (int)cx->x - y > 0 ? cx->x - y : 0;
		g2d_hline(img, &st, ex1, color);
	
		col = g2d_color_alpha_set(img, col,  255-i);
		_point_alpha_inside(img, cx->x + x, cx->y - y, col);
		_point_alpha_inside(img, cx->x + y, cx->y + x, col);
		_point_alpha_inside(img, cx->x - x, cx->y + y, col);
		_point_alpha_inside(img, cx->x - y, cx->y - x, col);
		if (x == 0) break;
		e2 = err; x2 = x;
		if (err > y){
			i = 255*(err+2*x-1)/r;
			if (i < 255) {
				col = g2d_color_alpha_set(img, col, 255- i);
				_point_alpha_inside(img, cx->x + x    , cx->y - y + 1, col);
				_point_alpha_inside(img, cx->x + y - 1, cx->y + x    , col);
				_point_alpha_inside(img, cx->x - x    , cx->y + y - 1, col);
				_point_alpha_inside(img, cx->x - y + 1, cx->y - x    , col);
			}
			err -= --x*2-1;
		}
		if (e2 <= x2--) { 
			i = 255*(1-2*y-e2)/r;
			if (i < 255) {
				col = g2d_color_alpha_set(img, col,  255-i);
				_point_alpha_inside(img, cx->x + x2, cx->y - y , col);
				_point_alpha_inside(img, cx->x + y , cx->y + x2, col);
				_point_alpha_inside(img, cx->x - x2, cx->y + y , col);
				_point_alpha_inside(img, cx->x - y , cx->y - x2, col);
			}
			err -= --y*2-1;
		}
	}
}

void g2d_circle_fill(g2dImage_s* img, g2dPoint_s* cx, unsigned r, g2dColor_t col){
	iassert(r);
    int x,y,xc,yc,re;

    x = r;
    y = 0;
    xc = 1 - 2 * r;
    yc = 1;
    re = 0;

    while( x >=y ){
		g2dPoint_s p0;
		p0.x = cx->x - x;
		p0.y = cx->y + y;
        g2d_hline(img, &p0, cx->x + x, col);
		p0.x = cx->x - x;
		p0.y = cx->y - y;
        g2d_hline(img, &p0, cx->x + x, col);
		p0.x = cx->x - y;
		p0.y = cx->y + x;
        g2d_hline(img, &p0, cx->x + y, col);
		p0.x = cx->x - y;
		p0.y = cx->y - x;
        g2d_hline(img, &p0, cx->x + y, col);
        ++y;
        re += yc;
        yc += 2;
        if ( 2 * re + xc > 0){
            --x;
            re += xc;
            xc += 2;
        }
    }
}

void g2d_ellipse_normal(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col){
    iassert( rx && ry );
    int x, y;
    int Xchange, Ychange;
    int EllipseError;
    int TwoASquare, TwoBSquare;
    int StoppingX, StoppingY;

    TwoASquare = 2 * rx * rx;
    TwoBSquare = 2 * ry * ry;

    x = rx - 1;
    y = 0;

    Xchange = ry * ry * ( 1 - 2 * rx);
    Ychange = rx * rx;

    EllipseError = 0;

    StoppingX = TwoBSquare * rx;
    StoppingY = 0;
  
    while( StoppingX > StoppingY ){
        _point_inside(img, cx->x + x, cx->y + y, col);
		_point_inside(img, cx->x - x, cx->y + y, col);
		_point_inside(img, cx->x + x, cx->y - y, col);
        _point_inside(img, cx->x - x, cx->y - y, col);

        ++y;
        StoppingY    += TwoASquare;
        EllipseError += Ychange;
        Ychange      += TwoASquare;
        if( (2 * EllipseError + Xchange) > 0 ){
            --x;
            StoppingX    -= TwoBSquare;
            EllipseError += Xchange;
            Xchange      += TwoBSquare;
        }
    }

    x = 0;
    y = ry - 1;
    Xchange = ry * ry;
    Ychange = rx * rx * (1 - 2 * ry);
    EllipseError = 0;
    StoppingX = 0;
    StoppingY = TwoASquare * ry;

    while( StoppingX < StoppingY ){
        _point_inside(img, cx->x + x, cx->y + y, col);
		_point_inside(img, cx->x - x, cx->y + y, col);
		_point_inside(img, cx->x + x, cx->y - y, col);
        _point_inside(img, cx->x - x, cx->y - y, col);

        ++x;
        StoppingX    += TwoBSquare;
        EllipseError += Xchange;
        Xchange      += TwoBSquare;
        if( (2 * EllipseError + Ychange) > 0 ){
            --y;
            StoppingY    -= TwoASquare;
            EllipseError += Ychange;
            Ychange      += TwoASquare;
        }
    }
}

void g2d_ellipse_antialiased(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col){
    iassert( rx && ry );

	double rx2 = rx * rx;
	double ry2 = ry * ry;
	const unsigned maxTransparency = 255;

	double quarter = round(rx2 / sqrt(rx2 + ry2));
	for( double x = 0; x <= quarter; ++x){
		double y = ry * sqrt(1.0-x*x/rx2);
		double error = (double)y - floor(y);
		unsigned transparency = round(error * maxTransparency);
		unsigned transparency2 = maxTransparency - transparency;
		if( transparency > 255 ) transparency = 255;
		if( transparency2 > 255 ) transparency2 = 255;

		col = g2d_color_alpha_set(img, col,  transparency);
		_point_alpha(img, cx->x + (unsigned)x, cx->y + (unsigned)floor(y), col);
		_point_alpha(img, cx->x - (unsigned)x, cx->y + (unsigned)floor(y), col);
		_point_alpha(img, cx->x + (unsigned)x, cx->y - (unsigned)floor(y), col);
		_point_alpha(img, cx->x - (unsigned)x, cx->y - (unsigned)floor(y), col);
		col = g2d_color_alpha_set(img, col,  transparency2);
		_point_alpha(img, cx->x + (unsigned)x, cx->y + (unsigned)floor(y)-1, col);
		_point_alpha(img, cx->x - (unsigned)x, cx->y + (unsigned)floor(y)-1, col);
		_point_alpha(img, cx->x + (unsigned)x, cx->y - (unsigned)floor(y)+1, col);
		_point_alpha(img, cx->x - (unsigned)x, cx->y - (unsigned)floor(y)+1, col);
	}

	quarter = round(ry2 / sqrt(rx2 + ry2));
	for(double y = 0; y <= quarter; ++y){
		double x = rx * sqrt(1.0-y*y/ry2);
		double error = x - floor(x);
		unsigned transparency = round(error * maxTransparency);
		unsigned transparency2 = maxTransparency - transparency;

		col = g2d_color_alpha_set(img, col,  transparency);
		_point_alpha(img, cx->x + (unsigned)floor(x), cx->y + (unsigned)y, col);
		_point_alpha(img, cx->x - (unsigned)floor(x), cx->y + (unsigned)y, col);
		_point_alpha(img, cx->x + (unsigned)floor(x), cx->y - (unsigned)y, col);
		_point_alpha(img, cx->x - (unsigned)floor(x), cx->y - (unsigned)y, col);
		col = g2d_color_alpha_set(img, col,  transparency2);
		_point_alpha(img, cx->x + (unsigned)floor(x)-1, cx->y + (unsigned)y, col);
		_point_alpha(img, cx->x - (unsigned)floor(x)+1, cx->y + (unsigned)y, col);
		_point_alpha(img, cx->x + (unsigned)floor(x)-1, cx->y - (unsigned)y, col);
		_point_alpha(img, cx->x - (unsigned)floor(x)+1, cx->y - (unsigned)y, col);
	}
}

void g2d_ellipse(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col, int antialiased){
	if( antialiased ) g2d_ellipse_antialiased(img, cx, rx, ry, col);
	else g2d_ellipse_normal(img, cx, rx, ry, col);
}

void g2d_ellipse_fill(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col){
	iassert( rx && ry );
    int x, y;
    int Xchange, Ychange;
    int EllipseError;
    int TwoASquare, TwoBSquare;
    int StoppingX, StoppingY;
    
    TwoASquare = 2 * rx * rx;
    TwoBSquare = 2 * ry * ry;

    x = rx - 1;
    y = 0;

    Xchange = ry * ry * ( 1 - 2 * rx);
    Ychange = rx * rx;

    EllipseError = 0;

    StoppingX = TwoBSquare * rx;
    StoppingY = 0;
  
    while( StoppingX > StoppingY ){
		g2dPoint_s p;
		p.x = cx->x > (unsigned)x ? cx->x - x : 0;
        p.y = cx->y + y;
        g2d_hline(img, &p, cx->x + x, col);
        p.x = cx->x > (unsigned)x ? cx->x - x : 0;
        p.y = cx->y > (unsigned)y ? cx->y - y : 0;
        g2d_hline(img, &p, cx->x + x, col);
        ++y;
        StoppingY    += TwoASquare;
        EllipseError += Ychange;
        Ychange      += TwoASquare;
        if ((2 * EllipseError + Xchange) > 0) {
            --x;
            StoppingX    -= TwoBSquare;
            EllipseError += Xchange;
            Xchange      += TwoBSquare;
        }
    }

    x = 0;
    y = ry - 1;
    Xchange = ry * ry;
    Ychange = rx * rx * (1 - 2 * ry);
    EllipseError = 0;
    StoppingX = 0;
    StoppingY = TwoASquare * ry;

    while( StoppingX < StoppingY ){
		g2dPoint_s p;
		p.x = cx->x > (unsigned)x ? cx->x - x : 0;
        p.y = cx->y + y;
        g2d_hline(img, &p, cx->x + x, col);
        p.x = cx->x > (unsigned)x ? cx->x - x : 0;
        p.y = cx->y > (unsigned)y ? cx->y - y : 0;
        g2d_hline(img, &p, cx->x + x, col);
        ++x;
        StoppingX    += TwoBSquare;
        EllipseError += Xchange;
        Xchange      += TwoBSquare;
        if ( (2 * EllipseError + Ychange) > 0) {
            --y;
            StoppingY    -= TwoASquare;
            EllipseError += Ychange;
            Ychange      += TwoASquare;
        }
    }
}

void g2d_arc_normal(g2dImage_s* img, g2dPoint_s* cx, unsigned r, float startAngle, float endAngle, g2dColor_t color){
	startAngle -= 90.0;
	endAngle -= 90.0;
	float const radStart = (startAngle * 3.14159265)/180.0;
	float const radEnd =   (endAngle   * 3.14159265)/180.0;
	for(float i = radStart + 0.00; i < radEnd; i += 0.01){
		float cosi = cos(i);
		float sini = sin(i);
		unsigned x = cosi * r + cx->x;
		unsigned y = sini * r + cx->y;
		_point_inside(img, x, y, color);
	}
}

void g2d_arc_antialiased(g2dImage_s* img, g2dPoint_s* cx, unsigned r, float startAngle, float endAngle, g2dColor_t col){
	//TODO orrible
	startAngle -= 90.0;
	endAngle -= 90.0;
	float const radStart = (startAngle * 3.14159265)/180.0;
	float const radEnd =   (endAngle   * 3.14159265)/180.0;
	for(float i = radStart + 0.00; i < radEnd; i += 0.01){
		float cosi = cos(i);
		float sini = sin(i);
		double dx = cosi * r + cx->x;
		double dy = sini * r + cx->y;
		unsigned x = dx;
		unsigned y = dy;
		double px0 = 1.0 - floor(x);
		double py0 = 1.0 - floor(y);
		double px1 = 1.0 - px0;
		double py1 = 1.0 - py0;
		double pa0 = 255.0-(255.0 * px0 * py0);
		double pa1 = 255.0-(255.0 * px1 * py0);
		double pa2 = 255.0-(255.0 * px0 * py1);
		double pa3 = 255.0-(255.0 * px1 * py1);

	
		unsigned char a = pa0;
		g2dColor_t c = g2d_color_get(img, x, y);
		if( (c & (~img->ma)) != (col & (~img->ma)) || g2d_color_alpha(img,c) < a ){
			col = g2d_color_alpha_set(img, col, a);
			_point_alpha_inside(img, x  , y  , col);
		}
		a = pa1;
		if( (c & (~img->ma)) != (col & (~img->ma)) || g2d_color_alpha(img,c) < a ){
			col = g2d_color_alpha_set(img, col, a);
			_point_alpha_inside(img, x+1, y  , col);
		}
		a = pa2;
		if( (c & (~img->ma)) != (col & (~img->ma)) || g2d_color_alpha(img,c) < a ){
			col = g2d_color_alpha_set(img, col, a);
			_point_alpha_inside(img, x  , y+1, col);
		}
		a = pa3;
		if( (c & (~img->ma)) != (col & (~img->ma)) || g2d_color_alpha(img,c) < a ){
			col = g2d_color_alpha_set(img, col, a);
			_point_alpha_inside(img, x+1, y+1, col);
		}
	}
}

void g2d_arc(g2dImage_s* img, g2dPoint_s* cx, unsigned r, float startAngle, float endAngle, g2dColor_t color, int antialiased){
	if( antialiased ){
		g2d_arc_antialiased(img, cx, r, startAngle, endAngle, color);
	}
	else{
		g2d_arc_normal(img, cx, r, startAngle, endAngle, color);
	}
}

void g2d_pieslice(g2dImage_s* img, g2dPoint_s* cx, unsigned r, float startAngle, float endAngle, g2dColor_t color){
	startAngle -= 90.0;
	endAngle -= 90.0;
	float const radStart = (startAngle * 3.14159265)/180.0;
	float const radEnd =   (endAngle   * 3.14159265)/180.0;

	g2dPoint_s eline = {
		.x = cos(radStart) * r + cx->x,
		.y = sin(radStart) * r + cx->y
	};
	g2d_line(img, cx, &eline, color, 0);	

	for(float i = radStart + 0.01; i < radEnd - 0.01; i += 0.01){
		float cosi = cos(i);
		float sini = sin(i);
		unsigned x = cosi * r + cx->x;
		unsigned y = sini * r + cx->y;
		_point_inside(img, x, y, color);
	}

	eline.x = cos(radEnd) * r + cx->x;
	eline.y = sin(radEnd) * r + cx->y;
	g2d_line(img, cx, &eline, color, 0);	
}

void g2d_pieslice_fill(g2dImage_s* img, g2dPoint_s* cx, unsigned r, float startAngle, float endAngle, g2dColor_t color){
	g2d_pieslice(img, cx, r, startAngle, endAngle, color);

	r -= 2;
	float midAngle = ((endAngle - startAngle) / 2) + startAngle;
	midAngle -= 90;
	float const rad = (midAngle * 3.14159265)/180.0;

	g2dPoint_s ecenter = {
		.x = cos(rad) * r + cx->x,
		.y = sin(rad) * r + cx->y
	};
	
	const unsigned row = g2d_row(img, ecenter.y);
	g2dColor_t* rep = g2d_color(img, row, ecenter.x);
	if( *rep != color ){
		//_point_inside(img, ecenter.x, ecenter.y, color);
		g2d_repfill(img, &ecenter, *rep, color);
	}
}

void g2d_repfill(g2dImage_s* img, g2dPoint_s* st, g2dColor_t rep, g2dColor_t col){
	g2dPoint_s* stk = vector_new(g2dPoint_s, 4096, NULL);
	vector_push_back(stk, *st);

    int8_t ract,lact;
    unsigned mx,my;
    
    while( vector_count(stk) ){
		g2dPoint_s p = vector_pull_back(stk);
        mx = p.x;
        my = p.y;
        ract = 0;
        lact = 0;
		
		unsigned row = g2d_row(img, p.y);
		g2dColor_t* scol = g2d_color(img, row, p.x);
        
        while ( p.y > 0 && *scol == rep ){
			if ( lact == 0 ){
				if( p.x > 0 && scol[-1] == rep ){
					g2dPoint_s t = { .x = p.x-1, .y = p.y };
					vector_push_back(stk, t);
                    lact = 1;
                }
            }
            else if( p.x > 0 && scol[-1] != rep ){
                lact = 0;
            }

            if ( ract == 0 ){
				if( p.x < img->w && scol[1] == rep ){
					g2dPoint_s t = { .x = p.x+1, .y = p.y };
					vector_push_back(stk, t);
                    ract = 1;
                }
            }
            else if( p.x < img->w && scol[1] != rep ){
                ract = 0;
            }
            
            _point(img, p.x, p.y, col);
            --p.y;
			row = g2d_row(img, p.y);
			scol = g2d_color(img, row, p.x);
        }

        p.x = mx;
        p.y = my + 1;
        ract = 0;
        lact = 0;
    	row = g2d_row(img, p.y);
		scol = g2d_color(img, row, p.x);
        
        while ( p.y < img->h && *scol == rep ){
			if ( lact == 0 ){
				if( p.x > 0 && scol[-1] == rep ){
					g2dPoint_s t = { .x = p.x-1, .y = p.y };
					vector_push_back(stk, t);
                    lact = 1;
                }
            }
            else if( p.x > 0 && scol[-1] != rep ){
                lact = 0;
            }

            if ( ract == 0 ){
				if( p.x < img->w && scol[1] == rep ){
					g2dPoint_s t = { .x = p.x+1, .y = p.y };
					vector_push_back(stk, t);
                    ract = 1;
                }
            }
            else if( p.x < img->w && scol[1] != rep ){
                ract = 0;
            }
            
            _point(img, p.x, p.y, col);
           
            ++p.y;
			row = g2d_row(img, p.y);
			scol = g2d_color(img, row, p.x);
        }
    }
}

