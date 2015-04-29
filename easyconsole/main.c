#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "easyconsole.h"
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define msleep(MS) usleep((MS)*1000)

VOID itob(CHAR* b, UINT32 v, INT32 sz)
{
	for ( --sz; sz >= 0; --sz)
		*b++ = ((v >> sz) & 1) + '0';
	*b = '\0';
}

#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio_ext.h>
#include <linux/input.h>
#include <easythread.h>

typedef struct _COLORIZED
{
	UINT32 sxw;
	UINT32 syw;
	CHAR word[128];
	UINT32 color;
}COLORIZED;


INT32 prew(UINT32* szb, CHAR** b, CHAR** cbu, INT32* c, UINT32* sty, UINT32 stx, UINT32 scrh, UINT32 scrw)
{
	COLORIZED col[1024];
	static UINT32 ncol = 0;
	static CHAR* cu = NULL;
	static UINT32 insty;
	static UINT32 instx;
	
	if (cu == NULL )
	{
		cu = col[0].word;
		insty = *sty;
		instx = stx;
	}
		
	
	CHAR* buf = *b;
	CHAR* cbuf = *cbu;
	
	UINT32 cb = cbuf - buf;
	UINT32 cy,cx;
	UINT32 i;
	
	cy = *sty + ( cb + stx -1) / scrw;
	if ( cy == *sty ) 
		cx =  (cb + stx) % scrw;
	else
		cx = (cb - (scrw - stx)) % scrw;
		
	if (!cx ) cx = scrw;
	
	if ( insty != *sty )
	{
		insty = *sty;
		for ( i = 0; i < ncol; ++i)
			col[i].syw = insty;
	}
	
	if ( *c != ' ' )
	{
		for ( i = 0; i < ncol; ++i)
		{
			con_gotorc(col[i].syw,col[i].sxw);
			con_setcolor(0,col[i].color);
			printf("%s",col[i].word);
		}
		*cu++ = *c;
		con_setcolor(0,0);
		con_gotorc(cy,cx);
		con_flush();
		return CON_INPEX_NONE;
	}
	
    *cu = '\0';
	
	if ( !strcmp(col[ncol].word,"con") )
	{
		col[ncol].syw = cy;
		col[ncol].sxw = cx - (( cu - col[ncol].word) + 1);
		col[ncol].color = CON_COLOR_RED;
		++ncol;
		cu = col[ncol].word;
	}
	else
	{
		cu = col[ncol].word;
	}
	
	for ( i = 0; i < ncol; ++i)
	{
		con_gotorc(col[i].syw,col[i].sxw);
		con_setcolor(0,col[i].color);
		printf("%s",col[i].word);
	}
	con_setcolor(0,0);
	con_gotorc(cy,cx);
	con_flush();
	
	return CON_INPEX_NONE;
}

int main(int argc, char **argv)
{		
	
	printf("input$ ");
	con_flush();
	CHAR* r = con_input(NULL,NULL,FALSE,NULL,0);
	
	printf("\n[%s]\n",r);
	
	free(r);
	return 0;
	
	
	/*
	CONMSG cm;
	con_msg(&cm,"init",0);
		con_printfk_reg('a',pk_sum);
	con_msg(&cm,NULL,100);
	
	con_printfk("5 + 5 = %5.5a\n");
	
	INT32 a = 13;
	INT32 b = 37;
	con_printfk("%d + %d = %*.*a\n%@",a,b,a,b);
	
	
	con_msg(&cm,"press any key to continue...",0);
		con_async(1,NULL);
			while(!con_kbhit());
			CHAR c = con_getchex();
		con_async(0,NULL);
		if ( c == 27 ) 
		{
			con_msg(&cm,NULL,-1);
			return 0;
		}
	con_msg(&cm,NULL,100);
	
	con_cls();
	UINT32 sw,sh;
	con_getmaxrc(&sh,&sw);
	
	
	con_rect(1,1,sh,sw);
	con_circle(sh/2+1,sw/2,sh/2 - 1,'.');
	
	con_line(sh/2 + 1, sw/2 - sw/4, sh/2 + 1, sw/2 + sw/4, '-');
	con_line(sh/2 - sh/3 + 1, sw/2 - sw/4, sh/2 + sh/3 + 1, sw/2 + sw/4, '\\');
	con_line(sh/2 + sh/3 + 1, sw/2 - sw/4, sh/2 - sh/3 + 1, sw/2 + sw/4, '/');
	
	con_gotorc(2,2);
	con_printfk("press any key to continue...%@");
	con_async(1,NULL);
		while(!con_kbhit());
		c = con_getchex();
	con_async(0,NULL);
	
	con_cls();
	
	return 0;
	*/
	
	
    con_async(1);
    
    INT32 c;
    while (1)
    {
		printf("wait kb\n");
		con_flush();
		while(!con_kbhit()) thr_sleep(0.001);
		printf("get kb\n");	
		con_flush();
		c = con_getchex();
		printf("[%d(%c)]",c,c);
			
		con_flush();
		
		if ( c == 'q' ) break;
	}
    
    con_async(0);
    
    return 0;
}

#endif

