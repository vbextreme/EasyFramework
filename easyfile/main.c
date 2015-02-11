#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <easyconsole.h>
#include "easyfile.h"

VOID dbg_bit16(INT16 v)
{
    unsigned int i,pw;
    pw = 1;
    pw <<= 15;
    for (i = 0; i < 16 ; i++,pw >>= 1 )
    {
        if ( v & pw)
        {
            putchar('1');
        }
        else
        {
            putchar('0');
        }
    }
}

VOID printprivileges(PRIVILEGE p)
{
    unsigned int i,pw;
    pw = 1;
    pw <<= 12;
    for (i = 0; i < 12 ; i++,pw >>= 1 )
    {
        if ( p & pw)
        {
            putchar('1');
        }
        else
        {
            putchar('0');
        }
    }
}

int main()
{
	FILE* f = fopen("test","w+");
	
	cfg_write("daniele","1",f);
	cfg_write("pippo","4",f);
	cfg_write("urca","7",f);
	
	cfg_write("pippo","2",f);
	cfg_write("urca","3",f);
	
	fclose(f);
	
	f = fopen("test","r");
	char a[100],b[100];
	
		while ( cfg_read(a,b,f) )
		{
			printf("\'%s\' == \'%s\'\n",a,b);
		}
	fclose(f);
	
    return 0;
}

#endif
