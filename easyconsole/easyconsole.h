#ifndef EASYCONSOLE_H_INCLUDED
#define EASYCONSOLE_H_INCLUDED

#include <easytype.h>
#include <termios.h>
#include <stdarg.h>

#define CON_KEY_SUPER  252
#define CON_KEY_CTRL   253
#define CON_KEY_ALT    254
#define CON_KEY_ALTGR  255
#define CON_KEY_F1     200 //default used
#define CON_KEY_F2     201
#define CON_KEY_F3     202
#define CON_KEY_F4     203
#define CON_KEY_F5     204
#define CON_KEY_F6     205
#define CON_KEY_F7     206
#define CON_KEY_F8     207
#define CON_KEY_F9     208
#define CON_KEY_F10    209 //default used
#define CON_KEY_F11    210 //default used
#define CON_KEY_F12    211
#define CON_KEY_UP     230
#define CON_KEY_DOWN   231
#define CON_KEY_RIGHT  232
#define CON_KEY_LEFT   233
#define CON_KEY_CANC   234 
#define CON_KEY_BACK   127 
#define CON_KEY_PGUP   236 
#define CON_KEY_PGDW   237 
#define CON_KEY_FINE   238
#define CON_KEY_HOME   239
#define CON_KEY_INS    240

#define CON_CLLS_RIGHT 0
#define CON_CLLS_LEFT  1
#define CON_CLLS_ALL   2

#define CON_COLOR_RESET    0
#define CON_COLOR_BK       10

#define CON_COLOR_BLACK    30
#define CON_COLOR_RED      31
#define CON_COLOR_GREEN    32
#define CON_COLOR_YELLOW   33
#define CON_COLOR_BLUE     34
#define CON_COLOR_MAGENTA  35
#define CON_COLOR_CYAN     36
#define CON_COLOR_LGRAY    37
#define CON_COLOR_DGRAY    90
#define CON_COLOR_LRED     91
#define CON_COLOR_LGREEN   92
#define CON_COLOR_LYELLOW  93
#define CON_COLOR_LBLUE    94
#define CON_COLOR_LMAGENTA 95
#define CON_COLOR_LCYAN    96
#define CON_COLOR_WHYTE    97

#define CON_COLOR_BK_BLACK    40
#define CON_COLOR_BK_RED      41
#define CON_COLOR_BK_GREEN    42
#define CON_COLOR_BK_YELLOW   43
#define CON_COLOR_BK_BLUE     44
#define CON_COLOR_BK_MAGENTA  45
#define CON_COLOR_BK_CYAN     46
#define CON_COLOR_BK_LGRAY    47
#define CON_COLOR_BK_DGRAY    100
#define CON_COLOR_BK_LRED     101
#define CON_COLOR_BK_LGREEN   102
#define CON_COLOR_BK_LYELLOW  103
#define CON_COLOR_BK_LBLUE    104
#define CON_COLOR_BK_LMAGENTA 105
#define CON_COLOR_BK_LCYAN    106
#define CON_COLOR_BK_WHYTE    107

#define CON_BRD_OO 'q'
#define CON_BRD_VV 'x'
#define CON_BRD_UL 'l'
#define CON_BRD_UR 'k'
#define CON_BRD_DL 'm'
#define CON_BRD_DR 'j'
#define CON_BRD_CC 'n'
#define CON_BRD_IU 'w'
#define CON_BRD_IL 't'
#define CON_BRD_IR 'u'
#define BCON_RD_ID 'v'

#define DRD_EOF   -1
#define DRD_EIO   -2

#include <easytype.h>

typedef struct _CDIRECTRW
{
    int fd;
    int saved_errno;
    struct termios  saved;
    struct termios  temporary;
}CDIRECTRW;

typedef struct _CONPK
{
	INT32 prec;
	INT32 width;
	BOOL minus;
	BOOL plus;
	BOOL sharp;
	BOOL precarg;
	BOOL widtharg;
	BOOL sh;
	BOOL lg;
	CHAR conv[80];
	CHAR k;
}CONPK;

typedef struct _CONMSG
{
	UINT32 x;
	UINT32 y;
	UINT32 ex;
	UINT32 ey;
	INT32 st;
}CONMSG;

typedef INT32(*PKFNC)(CONPK*, va_list*);

VOID con_async(INT32 enable, CHAR* ofeventk);

inline int con_drd(CDIRECTRW* dc);
inline int con_dwr(CDIRECTRW* dc, const char *const data, const size_t bytes);
inline void con_dsrwhyde(CDIRECTRW* dc);
inline int con_dsetting(CDIRECTRW* dc);
inline int con_drestore(CDIRECTRW* dc);
int con_dopen(CDIRECTRW* dc);

#define con_flush() fflush(stdout)

inline VOID con_gotorc(UINT32 r, UINT32 c);
VOID con_getrc(UINT32* r, UINT32* c);
inline void con_cls();
inline void con_clsline(int mode);
void con_setcolor(unsigned char b,unsigned char f);
void con_setcolor256(unsigned char b,unsigned char f);
inline void con_special(char v);
void con_line(unsigned int r1,unsigned int c1,unsigned int r2,unsigned int c2,char c);
void con_rect(unsigned int r,unsigned int c,unsigned int h,unsigned int w);
void con_fillrect(unsigned int r,unsigned int c,unsigned int h,unsigned int w,char f);
void con_circle(unsigned int r,unsigned int c,unsigned int ra,char ch);
void con_ellipse(unsigned int cr,unsigned int cc,unsigned int sr,unsigned int sc,char ch);
void con_getmaxrc(unsigned int* r,unsigned int* c);
inline void con_resize(unsigned int h,unsigned int w);
CHAR* con_newbuffer(UINT32* sz,UINT32 page);
inline VOID con_setbuf(CHAR* buf,UINT32 sz);
char* con_gets(char* d,int max);
int con_kbhit();
char con_getch();
CHAR con_getchex();
void con_getpassword(char* psw,int sz,char mask);
VOID con_printfk_reg(BYTE k,PKFNC fnc);
INT32 con_printfk(const CHAR* format,...);
VOID con_msg(CONMSG* m, CHAR* msg, INT32 status);

#endif
