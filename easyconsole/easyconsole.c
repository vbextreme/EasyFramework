#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>
//#include <dirent.h>
//#include <sys/stat.h>
#include <easyfile.h>

#include "easyconsole.h"

#define   RD_EOF   -1
#define   RD_EIO   -2

static int _fik = -1;
static struct termios initial_settings, new_settings;
static int peek_character = -1;
static BOOL asyncmode = FALSE;

static PKFNC _pkmap[256];
static BOOL _PKINIT = FALSE;

INT32 _iskey(CHAR* k)
{
	INT32 l = strlen(k);
	return (strncmp(&k[l-3],"kbd",3)) ? 0 : 1;
}

VOID con_async(INT32 enable, CHAR* ofeventk)
{
    if (enable)
    {
		asyncmode = TRUE;
        tcgetattr(0,&initial_settings);
        new_settings = initial_settings;
        new_settings.c_lflag &= ~ICANON;
        new_settings.c_lflag &= ~ECHO;
        new_settings.c_lflag &= ~ISIG;
        new_settings.c_lflag &= ~IXON;
        new_settings.c_cc[VMIN] = 1;
        new_settings.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &new_settings);
        
        CHAR ink[512];
        CHAR nd[512];
        CHAR* ofk = ofeventk;
        
        if ( ofeventk == NULL )
        {
			CHAR* fk = "/dev/input/by-path/";
			if ( dir_list(nd,TRUE,FT_LINK,fk) ) 
			{
				INT32 ret;
				do
				{
					ret = _iskey(nd);
				}while( !ret && dir_list(nd,TRUE,FT_LINK,NULL) != -1 );
				
				if ( !ret ) return;
				
				sprintf(ink,"/dev/input/by-path/%s",nd);
				if ((ret = readlink(ink, nd, 511)) == -1) return;
				nd[ret] = '\0';
				sprintf(ink,"/dev/input/%s",&nd[3]);
				ofk = ink; 
			}
		}
		
		_fik = open(ofk, O_RDONLY | O_NONBLOCK);
    }
    else
    {
		asyncmode = FALSE;
        tcsetattr(0, TCSANOW, &initial_settings);
        if (_fik > 0) close(_fik);
    }
}

inline int con_drd(CDIRECTRW* dc)
{
    unsigned char   buffer[4];
    ssize_t         n;

    while (1) {

        n = read(dc->fd, buffer, 1);
        if (n > (ssize_t)0)
            return buffer[0];

        else
        if (n == (ssize_t)0)
            return DRD_EOF;

        else
        if (n != (ssize_t)-1)
            return DRD_EIO;

        else
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return DRD_EIO;
    }
}

inline int con_dwr(CDIRECTRW* dc, const char *const data, const size_t bytes)
{
    const char       *head = data;
    const char *const tail = data + bytes;
    ssize_t           n;

    while (head < tail) {

        n = write(dc->fd, head, (size_t)(tail - head));
        if (n > (ssize_t)0)
            head += n;

        else
        if (n != (ssize_t)-1)
            return EIO;

        else
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return errno;
    }

    return 0;
}

inline void con_dsrwhyde(CDIRECTRW* dc)
{
    dc->temporary.c_lflag &= ~ICANON;
    dc->temporary.c_lflag &= ~ECHO;
    dc->temporary.c_cflag &= ~CREAD;
}

inline int con_dsetting(CDIRECTRW* dc)
{
    /* Set modified settings. */
    int result;
    do {
        result = tcsetattr(dc->fd, TCSANOW, &dc->temporary);
    } while (result == -1 && errno == EINTR);

    if (result == -1) return errno;

    return 0;
}

inline int con_drestore(CDIRECTRW* dc)
{
    int result;
    do {
        result = tcsetattr(dc->fd, TCSANOW, &dc->saved);
    } while (result == -1 && errno == EINTR);

    if (result == -1) return errno;
    errno = dc->saved_errno;
    return 0;
}

int con_dopen(CDIRECTRW* dc)
{
    const char *dev;
    int result,retval;

    dev = ttyname(STDIN_FILENO);
    if (!dev)
        dev = ttyname(STDOUT_FILENO);
    if (!dev)
        dev = ttyname(STDERR_FILENO);
    if (!dev) {errno = ENOTTY;return -1;}

    do {
        dc->fd = open(dev, O_RDWR | O_NOCTTY);
    } while (dc->fd == -1 && errno == EINTR);
    if (dc->fd == -1)return -1;

    /* Bad tty? */

    dc->saved_errno = errno;

    /* Save current terminal settings. */
    do {
        result = tcgetattr(dc->fd, &dc->saved);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = dc->saved_errno;
        return retval;
    }

    /* Get current terminal settings for basis, too. */
    do {
        result = tcgetattr(dc->fd, &dc->temporary);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = dc->saved_errno;
        return retval;
    }

    return 0;
}

inline VOID con_gotorc(UINT32 r, UINT32 c)
{
    printf("\033[%d;%df",r,c);
}

VOID con_getrc(UINT32* r, UINT32* c)
{
	con_flush();
    CDIRECTRW dc;

    if ( con_dopen(&dc) ) return;

    con_dsrwhyde(&dc);

    int rows,cols,result;

    do {

        if ( con_dsetting(&dc) ) break;

        if ( con_dwr(&dc, "\033[6n", 4) ) break;


        if ( (result = con_drd(&dc)) != 27) break;
        if ( (result = con_drd(&dc)) != '[')  break;

        /* Parse rows. */
        rows = 0;
        result = con_drd(&dc);
        while (result >= '0' && result <= '9') {
            rows = 10 * rows + result - '0';
            result = con_drd(&dc);
        }

        if (result != ';') break;

        /* Parse cols. */
        cols = 0;
        result = con_drd(&dc);
        while (result >= '0' && result <= '9') {
            cols = 10 * cols + result - '0';
            result = con_drd(&dc);
        }

        if (result != 'R') break;
        /* Success! */
        if (r) *r = rows;
        if (c) *c = cols;

    } while (0);

    /* Restore saved terminal settings. */
    con_drestore(&dc);
}

inline void con_cls()
{
    printf("\033[H\033[J");
}

inline void con_clsline(int mode)
{
    printf("\033[%dK",mode);
}


void con_setcolor(unsigned char b,unsigned char f)
{

    if (!b && !f)
    {
        printf("\033[m");
        return;
    }

    if (b)
        printf("\033[%um",b);
    if (f)
        printf("\033[%um",f);
}

void con_setcolor256(unsigned char b,unsigned char f)
{
    if (!b && !f)
    {
        printf("\033[m");
        return;
    }

    if (b)
        printf("\033[48;5;%um",b);
    if (f)
        printf("\033[38;5;%um",f);
}

inline void con_special(char v)
{
    printf("\033(0%c\033(B",v);
}

void con_line(unsigned int r1,unsigned int c1,unsigned int r2,unsigned int c2,char c)
{
    register int dx,dy,stepx,stepy,fraction;
    register int x0 = c1;
    register int x1 = c2;
    register int y0 = r1;
    register int y1 = r2;

    if(x0 != x1){if( x1 < x0){++x0;}else{--x0;}}
    if(y0 != y1){if( y1 < y0){++y0;}else{--y0;}}

    dx = x1 - x0;
    dy = y1 - y0;

    if (dy < 0) {dy=-dy;stepy=-1;}else{stepy=1;}
    if (dx < 0) {dx=-dx;stepx=-1;}else{stepx=1;}

    dx <<= 1;
    dy <<= 1;

    if (dx > dy)
    {
        fraction=dy - (dx >> 1);
        while (x0 != x1)
        {
            x0 += stepx;
            if (fraction >= 0)
                {y0+=stepy;fraction -= dx;}

            fraction += dy;
            con_gotorc(y0,x0);
            putchar(c);
        }
    }
    else
    {
        fraction=dx-(dy >> 1);
        while (y0 != y1)
        {
            y0 += stepy;

            if (fraction >= 0)
                {x0+=stepx;fraction-=dy;}

            fraction += dx;
            con_gotorc(y0,x0);
            putchar(c);
        }
    }
}

void con_rect(unsigned int r,unsigned int c,unsigned int h,unsigned int w)
{
    register int ir,ic;
    con_gotorc(r,c);
    con_special(CON_BRD_UL);
    for (ic = 0; ic < w - 2 ; ic++ )
        con_special(CON_BRD_OO);
    con_special(CON_BRD_UR);


    con_gotorc(r + h - 1,c);
    con_special(CON_BRD_DL);
    for (ic = 0; ic < w - 2 ; ic++ )
        con_special(CON_BRD_OO);
    con_special(CON_BRD_DR);

    for (ir = r + 1; ir < r + h - 1 ; ir++ )
    {
        con_gotorc(ir,c);
        con_special(CON_BRD_VV);
        con_gotorc(ir,c + w - 1);
        con_special(CON_BRD_VV);
    }
}

void con_fillrect(unsigned int r,unsigned int c,unsigned int h,unsigned int w,char f)
{
    register int ir,ic;
    for (ir = r + 1 ; ir <  r + h - 1 ; ir++)
    {
        con_gotorc(ir,c + 1);
        for (ic = c + 1 ; ic < c + w - 1 ; ic++)
            putchar(f);
    }
}

void con_circle(unsigned int r,unsigned int c,unsigned int ra,char ch)
{

    register int x,y,
                 xc,yc,re;

    x=ra;
    y=0;
    xc=1-2*ra;
    yc=1;
    re=0;
    while(x>=y)
    {
        con_gotorc(r+y,c+x);
        putchar(ch);
        con_gotorc(r+y,c-x);
        putchar(ch);
        con_gotorc(r-y,c-x);
        putchar(ch);
        con_gotorc(r-y,c+x);
        putchar(ch);
        con_gotorc(r+x,c+y);
        putchar(ch);
        con_gotorc(r+x,c-y);
        putchar(ch);
        con_gotorc(r-x,c-y);
        putchar(ch);
        con_gotorc(r-x,c+y);
        putchar(ch);
        ++y;
        re+=yc;
        yc+=2;
        if (2*re+xc>0){
            --x;
            re+=xc;
            xc+=2;
        }
    }
}

void con_ellipse(unsigned int cr,unsigned int cc,unsigned int sr,unsigned int sc,char ch)
{
    register int rx_2,ry_2,
                 p,
                 x,y,
                 two_ry_2_x,two_rx_2_y;

    rx_2 = sc * sc;
    ry_2 = sr * sr;
    p = ry_2 - rx_2 * sr + (ry_2>>2);
    x = 0;
    y = sr;
    two_ry_2_x = 0;
    two_rx_2_y = (rx_2<<1)*y;

    con_gotorc(cr+y,cc+x);
    putchar(ch);
    con_gotorc(cr+y,cc-x);
    putchar(ch);
    con_gotorc(cr-y,cc-x);
    putchar(ch);
    con_gotorc(cr-y,cc+x);
    putchar(ch);

    while(two_rx_2_y >= two_ry_2_x)
    {
        ++x;
        two_ry_2_x += (ry_2<<1);
        p +=  two_ry_2_x + ry_2;

        if(p >= 0)
        {
            --y;
            two_rx_2_y -= (rx_2<<1);
            p -= two_rx_2_y ;
        }

        con_gotorc(cr+y,cc+x);
        putchar(ch);
        con_gotorc(cr+y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc+x);
        putchar(ch);
    }

    p = (int)(ry_2*(x+1/2.0)*(x+1/2.0) + rx_2*(y-1)*(y-1) - rx_2*ry_2);

    while (y>=0)
    {
        p += rx_2;
        --y;
        two_rx_2_y -= (rx_2<<1);
        p -= two_rx_2_y;

        if(p <= 0)
        {
             ++x;
             two_ry_2_x += (ry_2<<1);
             p += two_ry_2_x;
        }

        con_gotorc(cr+y,cc+x);
        putchar(ch);
        con_gotorc(cr+y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc+x);
        putchar(ch);
    }
}

void con_getmaxrc(unsigned int* r,unsigned int* c)
{
    struct winsize ws;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws);
    *r = ws.ws_row;
    *c = ws.ws_col;
}

inline void con_resize(unsigned int h,unsigned int w)
{
    printf("\033[8;%u;%ut",h,w);
}

CHAR* con_newbuffer(UINT32* sz,UINT32 page)
{
	UINT32 r,c;
	con_getmaxrc(&r,&c);
	
	*sz = r * c * page;
	
	CHAR* b = malloc(*sz);
	memset(b,0,*sz);
	
	return b;
}

inline VOID con_setbuf(CHAR* buf,UINT32 sz)
{
	setvbuf(stdout,buf,_IOFBF,sz);
}

char* con_gets(char* de,int max)
{
    register char* d = de;
    register int c;
    while ( (c = getchar()) != EOF && c != '\n' && --max > 0 ) *d++ = c;
    *d = '\0';
    return de;
}

int con_kbhit()
{
    unsigned char ch;
    int nread;

    if (peek_character != -1) return 1;

    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
		nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
	tcsetattr(0, TCSANOW, &new_settings);
		
    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    
    if ( _fik <= 0 ) return 0;
    
    struct input_event ev;

    if ( read(_fik, &ev, sizeof(struct input_event)) <= 0 ) return 0;
    
    if ( ev.type == 1 && ev.value == 1 )
    {
		switch ( ev.code )
		{
			case 29:
				peek_character = CON_KEY_CTRL;
			return 1;
			
			case 56:
				peek_character = CON_KEY_ALT;
			return 1;
			
			case 100:
				peek_character = CON_KEY_ALTGR;
			return 1;
			
			case 125:
				peek_character = CON_KEY_SUPER;
			return 1;
		}
	}
	
    return 0;
}

char con_getch()
{
    char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    if ( read(0,&ch,1) <= 0 ) return 0;
    return ch;
}

CHAR con_getchex()
{
	register CHAR ch,r;
	
	ch = con_getch();
	
	if ( ch == 27 && con_kbhit() )
	{
		ch = con_getch();
		if ( ch == 79 && con_kbhit() )
		{
			ch = con_getch();
			switch ( ch )
			{
				case 70: return CON_KEY_FINE;
				case 72: return CON_KEY_HOME;
				case 80: return CON_KEY_F1;
				case 81: return CON_KEY_F2;
				case 82: return CON_KEY_F3;
				case 83: return CON_KEY_F4;
				default: return 0;
			}
		}
		else if ( ch == 91 && con_kbhit() )
		{
			ch = con_getch();
			
			switch ( ch )
			{	
				case 51: ch = CON_KEY_CANC; if ( con_kbhit() )	r = con_getch();  return ch;
				case 53: ch = CON_KEY_PGUP; if ( con_kbhit() )	r = con_getch();  return ch;
				case 54: ch = CON_KEY_PGDW; if ( con_kbhit() )	r = con_getch();  return ch;
				case 65: return CON_KEY_UP;
				case 66: return CON_KEY_DOWN;
				case 67: return CON_KEY_RIGHT;
				case 68: return CON_KEY_LEFT;
			}
			
			if ( ch == 49 && con_kbhit() )
			{
				ch = con_getch();
				switch ( ch )
				{
					case 53: ch = CON_KEY_F5; break;
					case 55: ch = CON_KEY_F6; break;
					case 56: ch = CON_KEY_F7; break;
					case 57: ch = CON_KEY_F8; break;
					default: ch = 0;
				}
				if ( ch && con_kbhit() )
					r = con_getch();//126
				return ch;
			}
			else if ( ch == 50 && con_kbhit() )
			{
				ch = con_getch();
				switch ( ch )
				{
					case 48: ch = CON_KEY_F9; break;
					case 50: ch = CON_KEY_F10; break;
					case 51: ch = CON_KEY_F11; break;
					case 52: ch = CON_KEY_F12; break;
					case 126: return CON_KEY_INS;
					default: ch = 0;
				}
				if ( ch && con_kbhit() )
					r = con_getch();//126
				return ch;
			}
			else 
				return 0;
		}
		else
			return 0;
	}
	
    return ch;
}

void con_getpassword(char* psw,int sz,char mask)
{
    char* s = psw;
    con_async(1,NULL);
    while(sz--)
    {
        while (!con_kbhit());
        *psw = con_getch();

        if (*psw == '\n')
            break;
        else if (*psw == 127 )
        {
            if (psw > s)
            {
                putchar('\b');
                putchar(' ');
                putchar('\b');
                --psw;
            }
        }
        else
        {
            ++psw;
            putchar(mask);
        }
        fflush(stdout);
    }
    *psw = '\0';
    putchar('\n');
    con_async(0,NULL);
}

static INT32 _pkdef(CONPK* pk, va_list* ap)
{
	INT32 vap;
	INT32* pvap;
	CHAR* pcap;
	FLOAT64 dap;
	
	switch ( pk->k )
	{
		case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':  case 'c':
			vap = va_arg(*ap,INT32);
			printf(pk->conv,vap);
		break;
		case 's':
            pcap = va_arg(*ap,CHAR*);
            printf(pk->conv,pcap);
        break;        
        case 'e': case 'f': case 'g':
            dap = va_arg(*ap,FLOAT64);
            printf(pk->conv,dap);
        break;
        case 'p': case 'n':
            pvap = va_arg(*ap,INT32*);
            printf(pk->conv,pvap);
        break;
        default:
			return -1;
	}
	return 0;
}

static INT32 _pkdefk(CONPK* pk, va_list* ap)
{
	INT32 bk,fc;
	bk = ( pk->widtharg ) ? va_arg(*ap,INT32) : pk->width;
	fc = ( pk->precarg ) ? va_arg(*ap,INT32) : pk->prec;
	con_setcolor(bk,fc);
	return 0;
}

static INT32 _pkdefK(CONPK* pk, va_list* ap)
{
	INT32 bk,fc;
	bk = ( pk->widtharg ) ? va_arg(*ap,INT32) : pk->width;
	fc = ( pk->precarg ) ? va_arg(*ap,INT32) : pk->prec;
	con_setcolor256(bk,fc);
	return 0;
}

static INT32 _pkdeft(CONPK* pk, va_list* ap)
{
	UINT32 r,c;
	r = ( pk->widtharg ) ? va_arg(*ap,INT32) : pk->width;
	c = ( pk->precarg ) ? va_arg(*ap,INT32) : pk->prec;
	con_gotorc(r,c);
	return 0;
}

static INT32 _pkdefb(CONPK* pk, va_list* ap)
{
	register INT32 sz;
	sz = ( pk->widtharg ) ? va_arg(*ap,INT32) : pk->width;
	if (sz <= 0) sz = 8;
	
	register UINT32 a = va_arg(*ap,UINT32);
	
	for ( --sz; sz >= 0; --sz)
		putchar(((a >> sz) & 0x1) + '0');
	
	return 0;
}

static INT32 _pkdefflush(CONPK* pk, va_list* ap)
{
	con_flush();
	return 0;
}

static INT32 _pkdefcls(CONPK* pk, va_list* ap)
{
	con_cls();
	return 0;
}

static VOID _pkinit()
{
	_PKINIT = TRUE;
	register INT32 i;
	for ( i = 0; i < 256; ++i )
		_pkmap[i] = _pkdef;
	
	_pkmap[(BYTE)'b'] = _pkdefb;
	_pkmap[(BYTE)'k'] = _pkdefk;
	_pkmap[(BYTE)'K'] = _pkdefK;
	_pkmap[(BYTE)'t'] = _pkdeft;
	_pkmap[(BYTE)'@'] = _pkdefflush;
	_pkmap[(BYTE)'/'] = _pkdefcls;
	
}

static const CHAR* _getprec(CONPK* pk,const CHAR* f)
{
	CHAR buf[1024];
	register CHAR* b = buf;
	for (; *f && *f >= '0' && *f <= '9'; *b++ = *f++);
	*b = '\0';
	pk->prec = ( buf[0] == '\0' ) ? -1 : atoi(buf);
	return f;
}
static const CHAR* _getwidth(CONPK* pk,const CHAR* f)
{
	CHAR buf[1024];
	CHAR* b = buf;
	for (; *f && *f >= '0' && *f <= '9'; *b++ = *f++);
	*b = '\0';
	pk->width = ( buf[0] == '\0' ) ? -1 : atoi(buf);
	return f;
}

static const CHAR* _getinfoch(CONPK* pk,const CHAR* f, BOOL fi)
{	
	while(1)
	{
		switch ( *f )
		{
			case '-':	pk->minus = TRUE; break;
			case '+':	pk->minus = TRUE; break;
			case '*':	if ( fi ) pk->widtharg = TRUE; else pk->precarg = TRUE; break;
			case '#':	pk->sharp = TRUE; break;
			case 'h': case 'H': pk->sh = TRUE; break;
			case 'l': case 'L': pk->lg = TRUE; break;
			default: return f;
		}
		++f;
	}
	return NULL;
}

VOID con_printfk_reg(BYTE k,PKFNC fnc)
{
	if ( !_PKINIT) _pkinit();
	_pkmap[k] = fnc;
}

INT32 con_printfk(const CHAR* format,...)
{
	if ( !_PKINIT) _pkinit();

    va_list ap;
    va_start(ap,format);
    
    
    CHAR buf[1024];
    register CHAR* b;
    CONPK pk;
    
    const CHAR* stf;
    
    while ( TRUE )
    {
		pk.prec = -1;
		pk.width = -1;
		pk.minus = FALSE;
		pk.plus = FALSE;
		pk.sharp = FALSE;
		pk.precarg = FALSE;
		pk.widtharg = FALSE;
		pk.sh = FALSE;
		pk.lg = FALSE;
		pk.conv[0] = '\0';
		pk.k = '\0';
		
		for (b = buf; *format && *format != '%'; *b++ = *format++);
		*b = '\0';
		if ( buf[0] != '\0' ) printf("%s",buf);
		if ( *format == '\0' ) break;
		stf = format;
		++format;
		if ( *format == '%' ) { putchar('%'); ++format; continue;}
		format = _getinfoch(&pk,format,TRUE);
		format = _getwidth(&pk,format);
		
		if ( *format == '.' )
		{
			++format;
			format = _getinfoch(&pk,format,TRUE);
			format = _getprec(&pk,format);
		}
		format = _getinfoch(&pk,format,TRUE);
		pk.k = *format;
		++format;
		
		strncpy(pk.conv,stf,format-stf);
		pk.conv[format-stf] = '\0';
		_pkmap[(BYTE)pk.k](&pk,&ap);
		//errorblock( !_pkmap[(BYTE)pk.k](&pk,&ap) );
	}
    
    va_end(ap);
    
    return 0;
}

VOID con_msg(CONMSG* m, CHAR* msg, INT32 status)
{	
	if ( msg )
	{
		con_getrc(&m->y,&m->x);
		con_printfk("[%0.97k..%0.0k]%s",msg);
		
		con_flush();
		return;
	}
	
	UINT32 ox,oy;
	con_getrc(&oy,&ox);
	con_gotorc(m->y,m->x+1);
	
	if ( status > 99 ) 
		con_printfk("%0.32kOK%0.0k");
	else if ( status < 0 )  
		con_printfk("%0.31kEE%0.0k");
	else
		con_printfk("%0.97k%2d%0.0k",status);
	
	con_gotorc(oy,ox);	
	con_flush();
}






