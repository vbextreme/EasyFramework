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
	
	UINT32 fbit;
	
	if ( bit == 8 && parity == 0 && bstop == 1 )
	{
		fbit = CS8;
	}
	else
	{
		///no else mode for now
		free(h);
		return NULL;
	}
	
	switch (baund)
	{
		case 4800: baund = B4800; break;
		case 9600: baund = B9600; break;
		case 19200: baund =  B19200; break;
		case 38400: baund = B38400; break;
		case 57600: baund = B57600; break;
		case 115200: baund = B115200; break;
		default: free(h); return NULL;
	}
	
	h->newsetting.c_cflag = baund | CRTSCTS | fbit | CLOCAL | CREAD;
	
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

UINT32 srl_write(HSRL h,VOID* data,UINT32 sz)
{
	return ( write(h->f,data,sz) < 0 ) ? 0 : 1;
}

VOID srl_close(HSRL h)
{
	tcsetattr(h->f,TCSANOW,&h->oldsetting);
	close(h->f);
	free(h);
}
