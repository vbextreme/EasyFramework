#ifdef _APP

#include "main.h"

int main()
{
	BYTE cmd[4];
	
	HSRL com = srl_open("/dev/ttySAC0",SRL_NONCANONICAL,9600,8,0,1,10000,4);
	if ( com == NULL )
	{
		puts("error open com");
		return 0;
	}
	
	cmd[0] = 0;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;
	
	if ( !srl_write(com,cmd,4) )
	{
		puts("error open com");
		srl_close(com);
		return 0;
	}
	
	cmd[1] = 1;
	cmd[2] = 2;
	cmd[3] = 3;
	
	while (1)
	{
		int r = srl_read(com,&cmd,4);
		if ( r ) printf("read %d:%d %d %d %d\n",r,cmd[0],cmd[1],cmd[2],cmd[3]);
	}
	
	srl_close(com);
	
	return 0;
}
#endif
