#include "easyserial.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct __HSRL
{
	INT32 f;
	SRLMODE m;
	struct termios oldsetting;
	struct termios newsetting;
}_HSRL;

HSRL srl_open(CHAR* p, SRLMODE m, UINT32 baund, UINT32 bit, UINT32 parity, UINT32 bstop, UINT32 timeout, UINT32 nchar)
{
	_HSRL* h = malloc(sizeof(_HSRL));
		if ( !h ) return NULL;
	
	h->m = m;
	
	if ( m == SRL_ACANONICAL || m == SRL_ANONCANONICAL )
		h->f = open(p, O_RDWR | O_NOCTTY | O_NONBLOCK );
	else
		h->f = open(p, O_RDWR | O_NOCTTY );
		
		if ( h->f < 0) {free(h); return NULL;}
	
	tcgetattr(h->f,&h->oldsetting);
	bzero(&h->newsetting, sizeof(struct termios));
	
	switch (bit)
    {
		case 8: default: bit = CS8; break;
        case 7: bit = CS7; break;
        case 6: bit = CS6; break;
        case 5: bit = CS5; break;
    }  
    
    switch (bstop)
    {
		case 1: default: bstop = 0; break;
        case 2: bstop = CSTOPB; break;
    }
    
    switch (parity)
    {
		case 0: default: parity = 0; break;
        case 1: parity = PARENB | PARODD; break;
        case 2: parity = PARENB; break;
    }
	
	switch (baund)
	{
		case 4800: baund = B4800; break;
		case 9600: baund = B9600; break;
		case 19200: baund =  B19200; break;
		case 38400: baund = B38400; break;
		case 57600: baund = B57600; break;
		case 115200: baund = B115200; break;
		case 230400: baund = B230400; break;
		case 460800: baund = B460800; break;
		case 500000: baund = B500000; break;
		case 576000: baund = B576000; break;
		case 921600: baund = B921600; break;
		case 1000000: baund = B1000000; break;
		case 1152000: baund = B1152000; break;
		case 1500000: baund = B1500000; break;
		case 2000000: baund = B2000000; break;
		case 2500000: baund = B2500000; break;
		case 3000000: baund = B3000000; break;
		case 3500000: baund = B3500000; break;
		case 4000000: baund = B4000000; break;
		default: free(h); return NULL;
	}
	
	h->newsetting.c_cflag = baund | CRTSCTS | bit | baund | parity | bstop| CLOCAL | CREAD;
	
	if ( m == SRL_CANONICAL || m == SRL_ACANONICAL)
	{
		h->newsetting.c_iflag = IGNPAR | ICRNL;
		h->newsetting.c_lflag = ICANON;
		
		h->newsetting.c_cc[VINTR] = 0;
		h->newsetting.c_cc[VQUIT] = 0;
		h->newsetting.c_cc[VERASE] = 0;
		h->newsetting.c_cc[VKILL] = 0;
		h->newsetting.c_cc[VEOF] = 4;
		h->newsetting.c_cc[VTIME] = 0;
		h->newsetting.c_cc[VMIN] = 1;
		h->newsetting.c_cc[VSWTC] = 0;
		h->newsetting.c_cc[VSTART] = 0;
		h->newsetting.c_cc[VSTOP] = 0;
		h->newsetting.c_cc[VSUSP] = 0;
		h->newsetting.c_cc[VEOL] = 0;
		h->newsetting.c_cc[VREPRINT] = 0;
		h->newsetting.c_cc[VDISCARD] = 0;
		h->newsetting.c_cc[VWERASE] = 0;
		h->newsetting.c_cc[VLNEXT] = 0;
		h->newsetting.c_cc[VEOL2] = 0;
	}
	else
	{
		h->newsetting.c_iflag = IGNPAR;
		h->newsetting.c_lflag = 0;
		h->newsetting.c_cc[VTIME] = timeout;
		h->newsetting.c_cc[VMIN] = nchar;
	}
	
	h->newsetting.c_oflag = 0;
	
	tcflush(h->f, TCIFLUSH);
	tcsetattr(h->f,TCSANOW,&h->newsetting);
	
	return h;
}

UINT32 srl_read(HSRL h,VOID* data,UINT32 sz)
{
	if ( sz >= SRL_MAX_INP ) return 0;
	return read(h->f,data,sz);
}

UINT32 srl_write(HSRL h,const VOID* data,UINT32 sz)
{
	return ( write(h->f,data,sz) < 0 ) ? 0 : 1;
}

VOID srl_close(HSRL h)
{
	tcsetattr(h->f,TCSANOW,&h->oldsetting);
	close(h->f);
	free(h);
}
