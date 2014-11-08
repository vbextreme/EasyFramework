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

int main(int argc, char **argv)
{	
	con_printfk("%/%32b\n%@",0x03);
	
	return 0;
	
    
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

