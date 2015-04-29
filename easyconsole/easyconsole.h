#ifndef EASYCONSOLE_H_INCLUDED
#define EASYCONSOLE_H_INCLUDED

#include <easytype.h>
#include <termios.h>
#include <stdarg.h>

#define CON_KEY_WAIT    0.001

#define CON_KEY_NONE   '\0'
#define CON_KEY_ESC      27
#define CON_KEY_ENTER  '\n'
#define CON_KEY_CARR   '\r'
#define CON_KEY_TAB    '\t'
#define CON_KEY_BACK    127

#define CON_KEY_F1      0x00010000
#define CON_KEY_F2      0x00020000
#define CON_KEY_F3      0x00030000
#define CON_KEY_F4      0x00040000
#define CON_KEY_F5      0x00050000
#define CON_KEY_F6      0x00060000
#define CON_KEY_F7      0x00070000
#define CON_KEY_F8      0x00080000
#define CON_KEY_F9      0x00090000
#define CON_KEY_F10     0x000A0000
#define CON_KEY_F11     0x000B0000
#define CON_KEY_F12     0x000C0000
#define CON_KEY_UP      0x000D0000
#define CON_KEY_DOWN    0x000E0000
#define CON_KEY_SX      0x000F0000
#define CON_KEY_DX      0x00100000
#define CON_KEY_CANC    0x00110000
#define CON_KEY_PGUP    0x00120000
#define CON_KEY_PGDW    0x00130000
#define CON_KEY_FINE    0x00140000
#define CON_KEY_HOME    0x00150000
#define CON_KEY_INS     0x00160000
#define CON_KEY_SUPER_R 0x00170000
#define CON_KEY_CTRL    0x01000000
#define CON_KEY_ALT     0x02000000
#define CON_KEY_SHIFT   0x04000000

#define CON_CLLS_RIGHT "0K"
#define CON_CLLS_LEFT  "1K"
#define CON_CLLS_ALL   "2K"
#define CON_CLLS_DOWN  "J"
#define CON_CLLS_UP    "1J"

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
#define CON_INPEX_NONE     0x00
#define CON_INPEX_EOF      0x10
#define CON_INPEX_DRAW     0x20
#define CON_INPEX_DISCARGE 0x40
typedef INT32(*INPEX)(UINT32* szb, CHAR** buf, CHAR** cbuf, INT32* c, UINT32* sty, UINT32 stx, UINT32 scrh, UINT32 scrw);

VOID con_async(INT32 enable);

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
inline VOID con_clsline(CHAR* mode);
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
INT32 con_kbhit();
VOID con_sigsize();
BOOL con_haveresize();
INT32 con_getch();
INT32 con_getchex();
void con_getpassword(char* psw,int sz,char mask);
VOID con_printfk_reg(BYTE k,PKFNC fnc);
INT32 con_printfk(const CHAR* format,...);
VOID con_msg(CONMSG* m, CHAR* msg, INT32 status);
VOID con_carret_up(UINT32 n);
VOID con_carret_down(UINT32 n);
VOID con_carret_next(UINT32 n);
VOID con_carret_prev(UINT32 n);
VOID con_carret_home();
VOID con_carret_end();
VOID con_carret_save();
VOID con_carret_restore();
VOID con_scrool_up();
VOID con_scrool_down();
VOID con_carret_delete(UINT32 n);
VOID con_mode_ins(BOOL enable);
VOID con_linewrap(BOOL enable);
VOID con_vt100_reset();
CHAR* con_input(CHAR* inp, INPEX fprew, BOOL allpreview, INPEX finp, UINT32 max);

#endif
