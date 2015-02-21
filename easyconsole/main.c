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
#include <easyfile.h>


BOOL directkey(CHAR* d, CHAR* p)
{
	CHAR lk[512];
	INT32 ret;
	if ( (ret = readlink(p, lk, 511) ) == -1) return FALSE;
	lk[ret] = '\0';
	sprintf(d,"/dev/input/%s",&lk[3]);
	return TRUE;
}

BOOL testkey(CHAR* dev)
{
	CONMSG m;
	
	con_msg(&m,"Test keyboard ",0);
	printf("(%s) ",dev);
	printf("Press \'Ctrl\' or wait to exit");
	con_flush();
	
	INT32 autoexit = 120;
	
	con_async(1,dev);
	
	CHAR c;
	while(1)
	{	
		while (!con_kbhit() && autoexit-- ){ msleep(50);}
		if ( autoexit <= 0 ) {c = 'q'; break;}
		c = con_getchex();
		break;
	}
	
	con_async(0,NULL);
	
	if ( c == 'q' ) 
	{
		con_msg(&m,NULL,-1);
		puts("\n\tforce quit");
		return FALSE;
	}
	else if ( c == CON_KEY_CTRL )
	{
		con_msg(&m,NULL,100);
		puts("\n\tfound \'Ctrl\'");
		return TRUE;
	}
	
	con_msg(&m,NULL,-1);
	printf("\n\tkeymap not found %d\n", c);
	return FALSE;
}


int main(int argc, char **argv)
{	
	CONMSG m;
	
	con_msg(&m,"Search input device ",0);
	
	CHAR allinput[512][512];
	CHAR d[512];
	UINT32 countd = 0;
	
	FILETYPE ft = dir_list(d,TRUE,FT_LINK,"/dev/input/by-path/");
		if ( ft < 0 ) { puts("no device"); con_msg(&m,NULL,-1); return 0;}
	
	do
	{
		sprintf(allinput[countd++],"/dev/input/by-path/%s",d);
		ft = dir_list(d,TRUE,FT_LINK,NULL);
	}while( ft != -1);
	
	con_msg(&m,NULL,100);
	printf("found %d input device\n",countd);
	
	INT32 i;
	for ( i = 0; i < countd; ++i)
	{
		printf("(%d)%s\n",i,allinput[i]);
	}
	
	con_msg(&m,"Search Keyboard input ",0);
	
	INT32 idk = -1;
	for ( i = 0; i < countd; ++i)
	{
		INT32 l = strlen(allinput[i]);
		if ( !strncmp(&allinput[i][l-3],"kbd",3) ) 
		{
			idk = i;
			break;
		}
		con_msg(&m,NULL,(100*i)/countd);
	}
	
	if ( idk == -1 || idk >= countd )
	{
		idk = -1;
		con_msg(&m,NULL,-1);
		puts("not found keyboard");
	}
	else
	{
		con_msg(&m,NULL,100);
		printf("found id(%d)\n",idk);
		
		if ( !directkey(d,allinput[idk]) )
		{
			puts("Error to open link keyboard");
			return 0;
		}
		
		if ( testkey(d) )
		{
			puts("con_async() wotk fine");
			return 0;
		}
	}
	
	for ( i = 0; i < countd; ++i)
	{
		printf("Debug Input device:%d...",i);
		if ( !directkey(d,allinput[i]) )
		{
			puts("Error to open link input");
			continue;
		}
		
		puts("open input");
		if ( testkey(d) )
		{
			printf("Correct input device at:%d\n",i);
			return 0;
		}
	}
	
	printf("No input device found\n");
	/*
    con_async(1,NULL);
    
    CHAR c;
    while (1)
    {
		printf("wait kb\n");
		con_flush();
		while(!con_kbhit());
		printf("get kb\n");	
		con_flush();
		c = con_getchex();
		printf("%d\n",c);
		con_flush();
		if ( c == 27 ) break;
	}
    
    con_async(0,NULL);
    */
    return 0;
}

#endif

