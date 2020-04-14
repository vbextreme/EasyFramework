#ifndef __EF_IMAGE_H__
#define __EF_IMAGE_H__

//call os_begin()

#include <ef/type.h>
#include <ef/utf8.h>

typedef enum {G2D_MODE_BGRA, G2D_MODE_ABGR, G2D_MODE_ARGB, G2D_MODE_RGBA} g2dMode_e;

typedef struct g2dImage{
	unsigned char* pixel;
	unsigned w,h,p,sa,sr,sg,sb,ma,mr,mg,mb;
	g2dMode_e mode;
}g2dImage_s;

typedef unsigned int g2dColor_t;

typedef struct g2dCoord{
	unsigned x,y,w,h;
}g2dCoord_s;

typedef struct g2dPoint{
	unsigned x,y;
}g2dPoint_s;


#define img_clip(V) ((V) < 0 ? 0 : (V) > 255 ? 255 : (V))
#define img_clip_h(V) ((V>255)?255:V)

/** convert rgb to yuv8 */
void img_rgb_to_yuv8(unsigned char* y, unsigned char* u, unsigned char* v, unsigned char r ,unsigned char g, unsigned char b);

/** conver yuv8 to rgb*/
void img_yuv8_to_rgb(unsigned char* r, unsigned char* g, unsigned char* b, unsigned char y ,unsigned char u, unsigned char v);

/** convert rgb to gray*/
unsigned char img_rgb_to_gray(unsigned char r, unsigned char g, unsigned char b);

/** convert rgb to hue*/
int img_color_h(int R, int G, int B);

/** find centroid from vector of points*/
g2dPoint_s g2d_centroid(g2dPoint_s* points);

/** create new image with pixel set to pixels
 * @param w width
 * @param h height
 * @param mode color mode, default argb
 * @param pixels copy pointer in to structure
 * @return new image, no error return
 */
g2dImage_s* g2d_clone(unsigned w, unsigned h, g2dMode_e mode, uint8_t* pixels);

/** call g2d_clone with new pixel buffer
 * @param w width
 * @param h height
 * @param mode color mode, default argb
 * @return new image, no error return
 */
g2dImage_s* g2d_new(unsigned w, unsigned h, g2dMode_e mode);

/** free image and pixel buffer*/
void g2d_free(g2dImage_s* img);

/** cleanup */
void g2d_autofree(g2dImage_s** img);

/** cleanup */
#define __g2d_free __cleanup(g2d_autofree)

/** calcolate new w*h scaling ratio
 * @param modeAWH 0 automatic scaling to majour, 1 use sw / *w, 2 use sh / *h
 * @param sw source width
 * @param sh source height
 * @param w destination w
 * @param h destination h
 */
void g2d_ratio(int modeAWH, unsigned sw, unsigned sh, unsigned* w, unsigned* h);

/** create color with on mode*/
g2dColor_t g2d_color_gen(g2dMode_e mode, unsigned a, unsigned r, unsigned g, unsigned b);

/** return copy of img src*/
g2dImage_s* g2d_copy(g2dImage_s* src);

/** copy part of image src to dst cod image, size of block w*h need equal*/
void g2d_bitblt(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos);

/** xor part of image src to dst cod image, size of block w*h need equal*/
void g2d_bitblt_xor(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos);

/** same bitblt but with alpha color*/
void g2d_bitblt_alpha(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos);

/** copy only a channel color */
void g2d_bitblt_channel(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos, unsigned int mask);

/** clear part of image and set to color*/
void g2d_clear(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord);

/** set channel to color img[y][x] = (img[y][x] & (~mask)) | color*/ 
void g2d_channel_set(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord, unsigned mask);

/** get pointer to row*/
#define g2d_row(IMG, Y) (((IMG)->p)*(Y))

/** get pointer to pixel*/
#define g2d_pixel(IMG,ROW) (&(IMG)->pixel[ROW])

/** get color pointer to pixel X*/
#define g2d_color(IMG,ROW,X) (unsigned*)(&(IMG)->pixel[(ROW)+((X)*4)])

/** get channel alpha*/
#define g2d_color_alpha(IMG,ARGB) (((ARGB)>>(IMG)->sa)&0xFF)

/** get channel red*/
#define g2d_color_red(IMG,ARGB)   (((ARGB)>>(IMG)->sr)&0xFF)

/** get channel green */
#define g2d_color_green(IMG,ARGB) (((ARGB)>>(IMG)->sg)&0xFF)

/** get channel blue*/
#define g2d_color_blue(IMG,ARGB)  (((ARGB)>>(IMG)->sb)&0xFF)

/** create color from image*/
#define g2d_color_make(IMG,A,R,G,B) ( ((A)<<(IMG)->sa) | ((R)<<(IMG)->sr) | ((G)<<(IMG)->sg) | ((B)<<(IMG)->sb) )

/** set alpha in color*/
#define g2d_color_alpha_set(IMG,ARGB,A) ( ((ARGB) & (~(IMG)->ma)) | ((A)<<(IMG)->sa) )

/** calcolate alpha*/
#define g2d_alpha_part(ALPHA, PART, BACKGROUND) ( ((PART) * (ALPHA)) / 255  + (BACKGROUND) *  (255 - (ALPHA)) / 255 )

/** convert img to grayscale using luminance*/
void g2d_luminance(g2dImage_s* img);

/** convert gray img to black white*/
void g2d_black_white(g2dImage_s* gray, g2dCoord_s * coord);

/** get two dominant color from block image
 * @param outAB array of two element where store two dominant color of black and white
 * @param src image where calcolated dominat
 * @param bw black and white image for getting position of pixel dominant
 * @param coord block of image
 */ 
void g2d_black_white_dominant(g2dColor_t* outAB, g2dImage_s* src, g2dImage_s* bw, g2dCoord_s* coord);

/** set bw pixel to correspective AB color in block */
void g2d_black_white_set(g2dImage_s* bw, g2dCoord_s* coord, g2dColor_t* colorAB);

/** compare two block of image, return 0 for equal or how many is different*/
unsigned g2d_compare_similar(g2dImage_s* a, g2dCoord_s* ca, g2dImage_s* b, g2dCoord_s* cb);

/** count bit in block*/
unsigned g2d_bitcount(g2dImage_s* img, g2dCoord_s* coord);

/** resize bicubic from src to dst */
void g2d_resize_to(g2dImage_s* dst, g2dImage_s* src);

/** return new image resize, bicubic*/
g2dImage_s* g2d_resize(g2dImage_s* src, unsigned w, unsigned h);

/** return new totated image */
g2dImage_s* g2d_rotate(g2dImage_s* src, unsigned cx, unsigned cy, float grad);

/** convert grayscaled image ch to color and copy to dst*/
void g2d_char(g2dImage_s* dst, g2dCoord_s* coord, g2dImage_s* ch, g2dColor_t col);

/** same g2d_char but not apply alpha*/
void g2d_char_indirect(g2dImage_s* dst, g2dCoord_s* coord, g2dImage_s* ch, g2dColor_t col);

/*primitive*/

/** rotate a point*/
void g2d_point_rotate(unsigned* y, unsigned* x, unsigned cy, unsigned cx, double grad);

/** draw a points*/
void g2d_points(g2dImage_s* img, g2dPoint_s* points, g2dColor_t* colors, size_t count);

/** draw horizzotal line*/
void g2d_hline(g2dImage_s* img, g2dPoint_s* st, unsigned x1, g2dColor_t col);

/** draw vertical line*/
void g2d_vline(g2dImage_s* img, g2dPoint_s* st, unsigned y1, g2dColor_t col);

/** draw line*/
void g2d_bline(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col);

/** draw line antialaised*/
void g2d_bline_antialiased(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col);

/** draw line and select previous function*/
void g2d_line(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col, int antialiased);

/** qubeizer line*/
void g2d_qubezier_antialiased(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t color);

/** cubeizer normal line*/
void g2d_cubezier_normal(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dPoint_s* p3, g2dColor_t col);

/** cubeizer antialiased line*/
void g2d_cubezier_antialiased(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dPoint_s* p3, g2dColor_t color);

/** cubeizer line*/
void g2d_cubezier(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dPoint_s* p3, g2dColor_t col, int antialiased);

/** draw triangle*/
void g2d_triangle(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t col, int antialiased);

/** draw triangle with fill color*/
void g2d_triangle_fill(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t col);

/** draw rect*/
void g2d_rect(g2dImage_s* img, g2dCoord_s* rc, g2dColor_t col);

/** draw rect with fill color*/
void g2d_rect_fill(g2dImage_s* img, g2dCoord_s* rc, g2dColor_t col);

/** draw circle normal*/
void g2d_circle_normal(g2dImage_s* img, g2dPoint_s* cx, unsigned r, g2dColor_t col);

/** draw circle antialiased*/
void g2d_circle_antialiased(g2dImage_s* img, g2dPoint_s* cx, int r, g2dColor_t col);

/** draw circle antialiased */
void g2d_circle(g2dImage_s* img, g2dPoint_s* cx, int r, g2dColor_t col, int antialiased);

/** draw circle with fill color*/
void g2d_circle_fill(g2dImage_s* img, g2dPoint_s* cx, unsigned r, g2dColor_t col);

/** draw ellipse no antialaised*/
void g2d_ellipse_normal(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col);

/** draw ellipse antialaised */
void g2d_ellipse_antialiased(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col);

/** draw ellipse */
void g2d_ellipse(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col, int antialiased);

/** replace color in region with new color*/ 
void g2d_repfill(g2dImage_s* img, g2dPoint_s* st, g2dColor_t rep, g2dColor_t col);

#endif 
