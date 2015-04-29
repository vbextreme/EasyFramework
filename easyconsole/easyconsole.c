#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>
#include <signal.h>
#include "easyconsole.h"

#define   RD_EOF   -1
#define   RD_EIO   -2

static struct termios initial_settings, new_settings;
static BOOL asyncmode = FALSE;
static UINT32 exkbhit = 0;
static CHAR onresize = 0;

static PKFNC _pkmap[256];
static BOOL _PKINIT = FALSE;


INT32 _iskey(CHAR* k)
{
	INT32 l = strlen(k);
	return (strncmp(&k[l-3],"kbd",3)) ? 0 : 1;
}

VOID con_async(INT32 enable)
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
        exkbhit = 0;
    }
    else
    {
        tcsetattr(0, TCSANOW, &initial_settings);
        asyncmode = FALSE;
        exkbhit = 0;
    }
}

VOID _sig_resize(INT32 sig)
{
	signal(SIGWINCH, SIG_IGN);
		onresize = 1;
	signal(SIGWINCH, _sig_resize);
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
	BOOL restore = FALSE;
	
	if ( asyncmode )
	{
		restore = TRUE;
		con_async(0);
	}
	
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
    
    if ( restore )
    {
		con_async(1);
	}
}

inline void con_cls()
{
    printf("\033[H\033[J");
}

inline VOID con_clsline(CHAR* mode)
{
    printf("\033[%s",mode);
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

INT32 _kbhit()
{	
	INT32 toread;
	ioctl(0, FIONREAD, &toread);
    return toread;
}

INT32 con_kbhit()
{	
	INT32 toread;
	ioctl(0, FIONREAD, &toread);
    return toread + exkbhit;
}

VOID con_sigsize()
{
	signal(SIGWINCH, SIG_IGN);
	signal(SIGWINCH, _sig_resize);
}

BOOL con_haveresize()
{
	if (onresize) {onresize = 0; return TRUE;}
	return FALSE;
}

INT32 con_getch()
{
    char ch;
	if ( read(0,&ch,1) < 1 ) return EOF;
    return ch;
}

VOID _clpp(UINT32* c, UINT32 sz)
{
	++(*c);
	if ( *c >= sz ) *c = 0;
}


INT32 _chex(INT32* buf, UINT32 sz, UINT32* r, UINT32 w)
{
	static const INT32 map79[8][5] = { {49,59,53,81,-1}, 
							           {49,59,53,82,-1}, 
							 	       {49,59,53,83,-1},
							  	       {72,-1,-1,-1,-1},
								       {70,-1,-1,-1,-1},
								       {81,-1,-1,-1,-1},
								       {82,-1,-1,-1,-1},
								       {83,-1,-1,-1,-1}
							         };
							   
	static const INT32 rem79[8] = { CON_KEY_F2 | CON_KEY_CTRL,
				       		 	    CON_KEY_F3 | CON_KEY_CTRL,
							        CON_KEY_F4 | CON_KEY_CTRL,
							        CON_KEY_HOME,
							        CON_KEY_FINE,
							        CON_KEY_F2,
							        CON_KEY_F3,
							        CON_KEY_F4
						          };
						    
    static const INT32 map91[33][6] = {
		                         { 49, 53,126, -1, -1, -1}, 
								 { 49, 55,126, -1, -1, -1}, 
								 { 49, 56,126, -1, -1, -1}, 
								 { 49, 57,126, -1, -1, -1}, 
								 { 49, 53, 59, 53,126, -1},
								 { 49, 55, 59, 53,126, -1}, 
								 { 49, 56, 59, 53,126, -1}, 
								 { 49, 57, 59, 53,126, -1},
								 { 49, 59, 50, 67, -1, -1},
								 { 49, 59, 50, 68, -1, -1},
								 { 49, 59, 51, 67, -1, -1},
								 { 49, 59, 51, 68, -1, -1},
								 { 49, 59, 51, 65, -1, -1},
								 { 49, 59, 51, 66, -1, -1},
								 { 49, 59, 53, 65, -1, -1},
								 { 49, 59, 53, 66, -1, -1},
								 { 49, 59, 53, 67, -1, -1},
								 { 49, 59, 53, 68, -1, -1},
								 { 50,126, -1, -1, -1, -1},
								 { 50, 48,126, -1, -1, -1},
								 { 50, 52,126, -1, -1, -1},
								 { 50, 48, 59, 53,126, -1},
								 { 50, 49, 59, 53,126, -1},
								 { 50, 51, 59, 53,126, -1},
								 { 50, 52, 59, 53,126, -1},
								 { 51,126, -1, -1, -1, -1},
								 { 53,126, -1, -1, -1, -1},
								 { 54,126, -1, -1, -1, -1},
								 { 65, -1, -1, -1, -1, -1},
								 { 66, -1, -1, -1, -1, -1},
								 { 67, -1, -1, -1, -1, -1},
								 { 68, -1, -1, -1, -1, -1}
							    };
							    
	static const INT32 rem91[33] = { CON_KEY_F5,
									 CON_KEY_F6,
									 CON_KEY_F7,
									 CON_KEY_F8,
									 CON_KEY_F5 | CON_KEY_CTRL,
									 CON_KEY_F6 | CON_KEY_CTRL,
									 CON_KEY_F7 | CON_KEY_CTRL,
									 CON_KEY_F8 | CON_KEY_CTRL,
									 CON_KEY_DX | CON_KEY_SHIFT,
									 CON_KEY_SX | CON_KEY_SHIFT,
									 CON_KEY_DX | CON_KEY_ALT,
									 CON_KEY_SX | CON_KEY_ALT,
									 CON_KEY_UP | CON_KEY_ALT,
									 CON_KEY_DOWN | CON_KEY_ALT,
									 CON_KEY_UP | CON_KEY_CTRL,
									 CON_KEY_DOWN | CON_KEY_CTRL,
									 CON_KEY_DX | CON_KEY_CTRL,
									 CON_KEY_SX | CON_KEY_CTRL,
									 CON_KEY_INS,
									 CON_KEY_F9,
									 CON_KEY_F12,
									 CON_KEY_F9  | CON_KEY_CTRL,
									 CON_KEY_F10 | CON_KEY_CTRL,
									 CON_KEY_F11 | CON_KEY_CTRL,
									 CON_KEY_F12 | CON_KEY_CTRL,
									 CON_KEY_CANC,
									 CON_KEY_PGUP,
									 CON_KEY_PGDW,
									 CON_KEY_UP,
									 CON_KEY_DOWN,
									 CON_KEY_DX,
									 CON_KEY_SX
								   };
	
	static const INT32 map1[19] = { 1, 2, 3, 4, 5, 6, 7, 14,15,16,18,20,21,22,23,24,25,26,31};
	static const INT32 rem1[19] = { 'a' | CON_KEY_CTRL, 'b' | CON_KEY_CTRL, 'c' | CON_KEY_CTRL,
									'd' | CON_KEY_CTRL, 'e' | CON_KEY_CTRL, 'f' | CON_KEY_CTRL,
									'g' | CON_KEY_CTRL, 'n' | CON_KEY_CTRL, 'p' | CON_KEY_CTRL,
									'r' | CON_KEY_CTRL, 't' | CON_KEY_CTRL,	'u' | CON_KEY_CTRL,
									'v' | CON_KEY_CTRL, 'w' | CON_KEY_CTRL, 'x' | CON_KEY_CTRL,
									'y' | CON_KEY_CTRL, 'z' | CON_KEY_CTRL,	'-' | CON_KEY_CTRL
								  };
	
	if ( *r == w ) 
	{
		//printf("r(%d) w(%d) start r==w",*r,w);
		return 0;
	}
	
	INT32 i;
	INT32 c = buf[*r];
	_clpp(r,sz);
	--exkbhit;
	
	if ( c != CON_KEY_ESC ) 
	{
		//printf("r(%d) w(%d) c(%d) != CON_KEY_ESC",*r,w,c);
		for ( i = 0; i < 19; ++i)
			if ( map1[i] == c ) {/*printf(" INMAP ");*/return rem1[i];}
		//printf(" NOMAP ");
		return c; 
	}
	else if ( *r == w )
	{
		//printf("r(%d) w(%d) r == w",*r,w);
		return CON_KEY_ESC;
	}
	
	UINT32 t = *r;
	c = buf[t];
	_clpp(&t,sz);
	
	if ( t == w )
	{
		*r = t;
		--exkbhit;
		//printf("r(%d) w(%d) t == w",*r,w);
		return c | CON_KEY_ALT;
	}
	
	if ( c == 79 )
	{
		INT32 vt[5] = {-1,-1,-1,-1,-1};
		INT32 k,n;
		
		for (i = 0; i < 5 && t == w; ++i, _clpp(&t,sz))
			vt[i] = buf[t];
		
		for (i = 0; i < 8; ++i)
		{
			for ( k = 0, n = 0; k < 5; ++k,++n )
			{
				if ( map79[i][k] == -1 || vt[k] == -1 ) break;
				if ( vt[k] != map79[i][k] ) break;
			}
			
			if ( map79[i][k] == -1 )
			{
				*r += n + 1;
				exkbhit -= n + 1;
				return rem79[i];
			}
		}
	}
	else if ( c == 91 )
	{
		INT32 vt[6] = {-1,-1,-1,-1,-1,-1};
		INT32 k,n;
		
		for (i = 0; i < 6 ; ++i)
		{
			vt[i] = buf[t];
			_clpp(&t,sz);
			if ( t == w ) break;
		}
		
		for (i = 0; i < 33; ++i)
		{
			for ( k = 0, n = 0; k < 6; ++k,++n )
			{
				if ( map91[i][k] == -1 || vt[k] == -1 ) break;
				if ( vt[k] != map91[i][k] ) break;
			}
			
			if ( map91[i][k] == -1 )
			{
				*r += n + 1;
				if (*r >= sz)
				{
					*r = sz - *r;
				}
				exkbhit -= n + 1;
				return rem91[i];
			}
		}
	}
	
	return CON_KEY_ESC;
} 


INT32 con_getchex()
{
	#define MAXBUFFER 256
	
	static INT32 unbuffer[MAXBUFFER];
	static UINT32 cw = 0;
	static UINT32 cr = 0;
									
	INT32 c;
	UINT32 test;
	
	test = cw;
	_clpp(&test,MAXBUFFER);
	while ( (_kbhit() && test != cr) || cw == cr )
	{
		c = con_getch();
		unbuffer[cw] = c;
		_clpp(&cw,MAXBUFFER);
		++exkbhit;
		test = cw;
		_clpp(&test,MAXBUFFER);
		/*
		printf("ONGETCH\n");
		INT32 d;
		for ( d = 0; d < MAXBUFFER; ++d)
		{
			printf("[");
			
			if ( cw == d )
			{
				con_setcolor(CON_COLOR_BK_LRED,0);
			}
			
			if ( cr == d )
			{
				con_setcolor(CON_COLOR_BK_LGREEN,0);
			}
			
			if (cw == cr && cw == d)
			{
				con_setcolor(CON_COLOR_BK_LBLUE,0);
			}
			
			
			printf("%2d",unbuffer[d]);
			con_setcolor(0,0);
			printf("]");
		}
		putchar('\n');
		*/
	}
	
	INT32 ret = _chex(unbuffer,MAXBUFFER,&cr,cw);
	/*
	printf("AFTERCHEX\n");
	
	INT32 d;
	for ( d = 0; d < MAXBUFFER; ++d)
	{
		printf("[");
		
		if ( cw == d )
		{
			con_setcolor(CON_COLOR_BK_LRED,0);
		}
			
		if ( cr == d )
		{
			con_setcolor(CON_COLOR_BK_LGREEN,0);
		}
			
		if (cw == cr && cr == d)
		{
			con_setcolor(CON_COLOR_BK_LBLUE,0);
		}
		
		printf("%2d",unbuffer[d]);
		con_setcolor(0,0);
		printf("]");
	}
	putchar('\n');
	printf("r(%d) w(%d)\n",cr,cw);
	*/
	return ret;
}

void con_getpassword(char* psw,int sz,char mask)
{
    char* s = psw;
    con_async(1);
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
    con_async(0);
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

static const CHAR* _getinfoch(CONPK* pk,const CHAR* f, BOOL fi)
{	
	while(1)
	{
		switch ( *f )
		{
			case '-':	pk->minus = TRUE; break;
			case '+':	pk->plus = TRUE; break;
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
	
	CONPK pk;
	
	va_list ap;
    va_start(ap,format);
	const CHAR* stc;
	
	CHAR *en;
	while(1)
	{
		while ( *format && *format != '%' ) putchar(*format++);
			if ( !*format ) return 0;
		
		++format;
		if ( *format == '%' ) {putchar(*format++);continue;}
		
		stc = format - 1;
		
		pk.prec = -1;pk.width = -1;
		pk.minus = FALSE; pk.plus = FALSE; pk.sharp = FALSE;
		pk.precarg = FALSE;	pk.widtharg = FALSE;
		pk.sh = FALSE; pk.lg = FALSE;
		pk.k = '\0';
		format = _getinfoch(&pk,format,TRUE);
		if ( !pk.widtharg )
		{
			pk.width = strtol(format,&en,10);
			format = en;
		}		
		if ( *format == '.' )
		{
			++format;
			format = _getinfoch(&pk,format,FALSE);
			if ( !pk.precarg )
			{
				pk.prec = strtol(format,&en,10);
				format = en;
			}
		}
		format = _getinfoch(&pk,format,FALSE);
		pk.k = *format++;
		en = pk.conv;
		while ( stc < format ) *en++ = *stc++;
		*en = '\0';
		_pkmap[(BYTE)pk.k](&pk,&ap);
	}
	
	return 0;
}


VOID con_msg(CONMSG* m, CHAR* msg, INT32 status)
{
	UINT32 ox=0,oy=0;
	if ( msg )
	{
		con_getmaxrc(&oy,&ox);
		con_getrc(&m->y,&m->x);
		con_printfk("[%0.97k..%0.0k]%s",msg);
		UINT32 l = strlen(msg) + 4;
		
		if ( l > ox )
		{
			l /= ox;
			if ( oy == m->y ) 
				m->y -= l;
			else if ( m->y + l > oy )
				m->y += (m->y + l) - oy; 
		}
		
		con_getrc(&m->ey,&m->ex);
		con_flush();
		return;
	}
	
	con_gotorc(m->y,m->x+1);
	
	if ( status > 99 ) 
		con_printfk("%0.32kOK%0.0k");
	else if ( status < 0 )  
		con_printfk("%0.31kEE%0.0k");
	else
		con_printfk("%0.97k%2d%0.0k",status);
	
	con_gotorc(m->ey,m->ex);	
	if ( status < 0 || status > 99 ) putchar('\n');
	con_flush();
}

VOID con_carret_up(UINT32 n)
{
	printf("\033[%dA",n);
}

VOID con_carret_down(UINT32 n)
{
	printf("\033[%dB",n);
}

VOID con_carret_next(UINT32 n)
{
	printf("\033[%dC",n);
}

VOID con_carret_prev(UINT32 n)
{
	printf("\033[%dD",n);
}

VOID con_carret_home()
{
	printf("\033[H");
}

VOID con_carret_end()
{
	printf("\033[H");
}

VOID con_carret_save()
{
	printf("\033[s");
}

VOID con_carret_restore()
{
	printf("\033[8");
}

VOID con_scrool_up()
{
	printf("\033[M");
}

VOID con_scrool_down()
{
	printf("\033[D");
}

VOID con_carret_delete(UINT32 n)
{
	printf("\033[%dP",n);
}


VOID con_mode_ins(BOOL enable)
{
	printf("\033[4%c",(enable)?'h':'l');
}

VOID con_linewrap(BOOL enable)
{
	printf("\033[?7%c",(enable)?'h':'l');
	
}

VOID con_vt100_reset()
{
	printf("\033[c");
}

CHAR* _ci_redraw(CHAR* buf, CHAR* cbuf, UINT32* sty, UINT32 stx, UINT32 scrh, UINT32 scrw, INT32 mode)
{
	UINT32 l = strlen(buf);
	UINT32 o = ( ( (scrh - *sty ) + 1 ) * scrw ) - stx;
	UINT32 cb = cbuf - buf;
	UINT32 cx ,cy;
	
	switch (mode)
	{
		case 1:
			con_clsline(CON_CLLS_RIGHT);
		case 2:
			if ( mode != 1 ) con_clsline(CON_CLLS_DOWN);
		case 9:
			con_gotorc(*sty,stx);
			printf("%s ",buf);
		break;
	}
	
	if ( l > o )
	{
		if ( *sty == 1 )
		{
			con_async(0);
			printf("nothing for now l=%d o=%d\n",l,o);
			exit(0);
		}
		else
		{
			--(*sty);
		}
	}
	
	cy = *sty + ( cb + stx -1) / scrw;
	
	
	if ( cy == *sty ) 
		cx =  (cb + stx) % scrw;
	else
		cx = (cb - (scrw - stx)) % scrw;
		
	if (!cx ) cx = scrw;
	con_gotorc(cy,cx);
	
	con_flush();
	
	return buf;
}


CHAR* con_input(CHAR* inp, INPEX fprew, BOOL allpreview, INPEX finp, UINT32 max)
{
	BOOL ins = TRUE;
	
	con_flush();
	
	UINT32 stx,sty,scrw,scrh;
	
	con_sigsize();
	
	con_getmaxrc(&scrh,&scrw);
	con_getrc(&sty,&stx);
	
	UINT32 bsz = scrw * 2;
	UINT32 blen = 0;
	CHAR* buf = malloc(sizeof(CHAR) * bsz);
	CHAR* cbuf = buf;
	*cbuf = '\0';
	
	if ( inp )
	{
		strcpy(buf,inp);
		blen = strlen(inp);
		cbuf += blen;
		_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,9);
	}
	
	con_async(1);
    
	INT32 c;
	INT32 ret = 0;
	
	while ( (c = con_getchex()) != CON_KEY_ENTER )
	{
		if ( con_haveresize() )
			con_getmaxrc(&scrh,&scrw);
		
		if ( allpreview && fprew)
		{
			ret = finp(&bsz,&buf,&cbuf,&c,&sty,stx,scrh,scrw);
			if ( ret == CON_INPEX_EOF ) break;
			if ( ret & CON_INPEX_DRAW ) 
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,ret & 0x0F);
			if ( ret & CON_INPEX_DISCARGE ) continue;
		}
			
		if ( (c & CON_KEY_CTRL) || (c & CON_KEY_ALT) || (c & CON_KEY_SHIFT) )
		{
			if ( !allpreview && finp )
			{
				ret = finp(&bsz,&buf,&cbuf,&c,&sty,stx,scrh,scrw);
				if ( ret == CON_INPEX_EOF ) break;
				if ( ret & CON_INPEX_DRAW ) 
					_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,ret & 0x0F);
				if ( ret & CON_INPEX_DISCARGE ) continue;
			}
			
			if ( (c & CON_KEY_CTRL) && ( c & 'c') ) break;
		}
		
		switch ( c )
		{
			default:
				if ( max > 0 && cbuf-buf >= max ) continue; 
			
				if ( blen >= bsz - 1 )
				{
					UINT32 coff = cbuf - buf;
					bsz += scrw * 2;
					CHAR* tmp = malloc(sizeof(CHAR) * bsz);
					memcpy(tmp,buf,blen + 1);
					free(buf);
					buf = tmp;
					cbuf = tmp + coff;
				}
				
				if ( ins )
				{
					if ( max > 0 && blen >= max ) continue; 
					memmove(cbuf+1,cbuf,(blen + 1) - (cbuf - buf) );
					++blen;
				}
				
				*cbuf = (CHAR) c;
				++cbuf;
				
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,9);
			break;
			
			case CON_KEY_CANC:
				if ( !blen ) break;
				memmove(cbuf,cbuf + 1,(blen + 1) - (cbuf - buf) );
				--blen;
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,9);
			break;
			
			case CON_KEY_BACK:
				if ( buf == cbuf ) break;
				--cbuf;
				memmove(cbuf,cbuf + 1,(blen + 1) - (cbuf - buf) );
				--blen;
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,9);
			break;
			
			case CON_KEY_SX:
				if ( buf == cbuf ) break;
				--cbuf;
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,0);
			break;
			
			case CON_KEY_DX:
				if ( !*cbuf ) break;
				++cbuf;
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,0);
			break;
			
			case CON_KEY_FINE:
				cbuf = buf + strlen(buf);
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,0);
			break;
			
			case CON_KEY_HOME:
				cbuf = buf;
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,0);
			break;
			case CON_KEY_INS:
				ins = !ins;
			break;
			
			case CON_KEY_ESC:
			case CON_KEY_CARR:
			case CON_KEY_F1:
			case CON_KEY_F2:
			case CON_KEY_F3:
			case CON_KEY_F4:
			case CON_KEY_F5:
			case CON_KEY_F6:
			case CON_KEY_F7:
			case CON_KEY_F8:
			case CON_KEY_F9:
			case CON_KEY_F10:
			case CON_KEY_F11:
			case CON_KEY_F12:
			case CON_KEY_UP:
			case CON_KEY_DOWN:
			case CON_KEY_PGUP:
			case CON_KEY_PGDW:
				if ( !allpreview && finp )
				{
					ret = fprew(&bsz,&buf,&cbuf,&c,&sty,stx,scrh,scrw);
					if ( ret == CON_INPEX_EOF ) goto TOEXIT;
					if ( ret & CON_INPEX_DRAW ) 
						_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,ret & 0x0F);
					if ( ret & CON_INPEX_DISCARGE ) continue;
				}
			break;
		}
		
		if ( finp )
		{
			ret = finp(&bsz,&buf,&cbuf,&c,&sty,stx,scrh,scrw);
			if ( ret == CON_INPEX_EOF ) goto TOEXIT;
			if ( ret & CON_INPEX_DRAW ) 
				_ci_redraw(buf,cbuf,&sty,stx,scrh,scrw,ret & 0x0F);
			if ( ret & CON_INPEX_DISCARGE ) continue;
		}
	}
	
	TOEXIT:
		con_async(0);
    
	return buf;
}

















