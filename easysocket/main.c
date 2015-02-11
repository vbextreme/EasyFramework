#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <easythread.h>
#include <easyconsole.h>
#include "easysocket.h"


#define CHATR 30
#define CHATC 80

#define CPOSR 1
#define CPOSC 1

#define INPR 32
#define INPC 1

#define MAXNAME 64
#define MAXINPUT 164

CHAR scr[CHATR][CHATC];
INT32 row = 0;
INT32 col = 0;
BOOL LOOP =  TRUE;
CHAR myname[MAXNAME];
BOOL issrv = FALSE;

void scr_scrool()
{
	INT32 i;
	for ( i = 0; i < CHATR - 1; ++i)
		strcpy(scr[i],scr[i+1]);
	scr[i][0] = '\0';
}

void scr_flush()
{
	con_cls();
	INT32 i;
	for ( i = 0; i < CHATR; ++i)
	{
		con_gotorc(CPOSR + i,CPOSC);
		printf("%s",scr[i]);
	}
	con_gotorc(INPR + i,INPC);
	printf("-->");
	con_flush();
}

void scr_write(CHAR* c, INT32 sz)
{
	while (sz--)
	{
		if ( *c == '\n' )
		{
			scr[row][col] = '\0';
			if ( row == CHATR -1 )
			{
				scr_scrool();
			}
			else
			{
				++row;
				col = 0;
			}
			++c;
			continue;
		}
			
		scr[row][col++] = *c++;
	}
	scr[row][col] = '\0';
	if ( row == CHATR -1 )
	{
		scr_scrool();
	}
	else
	{
		++row;
		col = 0;
	}
}

INT32 onconnect(SCK s,void* v,int sz)
{
	CHAR nws[MAXINPUT];
	sprintf(nws,"%s$ connected",myname);
	scr_write(nws,strlen(nws));
	scr_flush();
	
	sprintf(nws,"#?#%s",myname);
	sck_pack(s,1,0,strlen(nws),nws);
	
	return 0;
}

int ondisconnect(SCK s,void* v,int sz)
{
    CHAR nws[MAXINPUT];
	if ( issrv )
		strcpy(nws,"sys$ client disconnected");
	else
	{
		sprintf(nws,"%s$ disconnected, press enter to exit",myname);
		LOOP = FALSE;
	}
	scr_write(nws,strlen(nws));
	scr_flush();
	return 0;
}

int onerr(SCK s,void* v,int sz)
{
	CHAR nws[MAXINPUT];
	sprintf(nws,"%s$ Error, %d",myname,sz);
	scr_write(nws,strlen(nws));
	scr_flush();
	return 0;
}

int onincoming(SCK s,void* v,int sz)
{	
	CHAR nws[MAXINPUT];
	
	if ( sz != -1 )
	{
		sprintf(nws,"%s$ Error, incoming data",myname);
		scr_write(nws,strlen(nws));
		scr_flush();
		return 0;
	}
	
	PACK* pk = (PACK*) v;
	if ( issrv )
	{
		CHAR* d = (CHAR*) pk->data;
		if ( !strncmp(d,"#?#",3) )
		{
			d += 3;
			CHAR* a = malloc(MAXNAME);
			strcpy(a,d);
			sck_arg_set(s,a,NULL,TRUE);
			
			sprintf(nws,"%s$ Registre:%s",myname,a);
			scr_write(nws,strlen(nws));
		}
		else if ( !strncmp(d,"#$",2) )
		{
			CHAR* n = sck_arg_get(s);
			sprintf(nws,"sys$ send list client to:%s",n);
			scr_write(nws,strlen(nws));
	
			CHAR data[MAXINPUT];
			SCK my;
			for ( my = sck_enum(sck_server(s)); my; my = sck_enum(NULL) )
			{
				n = (CHAR*) sck_arg_get(my);
				sprintf(data,"sys$ %s",n);
				sck_pack(s,1,0,strlen(data),data);
			}
		}
		else if ( !strncmp(d,"#|",2) )
		{
			//redirect
		}
		else
		{
			scr_write((CHAR*)pk->data,pk->sz);
		} 
	}
	else
	{
		scr_write((CHAR*)pk->data,pk->sz);
	}
	scr_flush();
	return 0;
}

int onaccept(SCK s,void* v,int sz)
{
	CHAR nws[MAXINPUT];
	sprintf(nws,"%s$ Accept new user",myname);
	scr_write(nws,strlen(nws));
	scr_flush();
    return 1;
}

int onsend(SCK s,void* v,int sz)
{
	if ( sz != -1 )
	{
		CHAR nws[MAXINPUT];
		sprintf(nws,"%s$ Error, sending data",myname);
		scr_write(nws,strlen(nws));
		scr_flush();
		return 0;
	}
	
	PACK* pk = (PACK*) v;
	scr_write((CHAR*)pk->data,pk->sz);
	scr_flush();
    return 0;
}

int srv_message(SCK s,CHAR* inp)
{
	CHAR data[MAXINPUT];
	CHAR cli[MAXINPUT];
	CHAR *c = cli;
	
con_cls();
	
	while ( *inp && *inp != '$' ) *c++ = *inp++;
	if ( !*inp ) return 0;
	*c = '\0';
	++inp;
	
	sprintf(data,"%s$%s",myname,inp);
	
	SCK my;
	for ( my = sck_enum(s); my; my = sck_enum(NULL) )
	{
		CHAR* n = (CHAR*) sck_arg_get(my);
		if ( !strcmp(n,cli) )
		{
			break;
		} 
	}
	if ( !my ) 
	{
		return 0;
	}
	sck_pack(my,1,0,strlen(data),data);
	return 1;
}

int srv_myclient(SCK s)
{
	CHAR data[MAXINPUT];
	SCK my;
	for ( my = sck_enum(s); my; my = sck_enum(NULL) )
	{
		CHAR* n = (CHAR*) sck_arg_get(my);
		sprintf(data,"sys$ %s",n);
		scr_write(data,strlen(data));
	}
	return 1;
}

int cli_message(SCK s,CHAR* inp)
{
	CHAR data[MAXINPUT];
	if ( !strncmp(inp,"#?#",3) ) return 0;
	
	sprintf(data,"%s$%s",myname,inp);
	sck_pack(s,1,0,strlen(data),data);
	return 1;
}

int main(int argc, char** argv)
{
	if ( argc == 1 )
	{
		puts("use srv o cli");
		return 0;
	}
	
	if ( !strcmp(argv[1],"srv") ) issrv = TRUE;
	
	CHAR inp[MAXINPUT];
    
    printf("Insert your name$ ");
    con_flush();
    con_gets(myname,MAXNAME);
    
    con_cls();
    strcpy(inp,"sys$ command \'#q\' to exit");
    scr_write(inp,strlen(inp)); 
    strcpy(inp,"sys$ command \'#$\' list client");
    scr_write(inp,strlen(inp)); 
    
    if ( issrv )
    {
		strcpy(inp,"sys$ server for write: \'destname$message\'");
		scr_write(inp,strlen(inp)); 
	}
    
	SCK s;
	if ( issrv )
	{
		s = sck_srv_new(2850);
		sck_event(s,NULL,onaccept,ondisconnect,onerr,onsend,onincoming);
	}
	else
	{
		s = sck_cli_new("127.0.0.1",2850);
		sck_event(s,onconnect,NULL,ondisconnect,onerr,onsend,onincoming);
	}
	sck_buf_set(s,PKPACK,FALSE,0,NULL);
	sck_run(s);
    
    while(LOOP)
    {
		scr_flush();
		con_gets(inp,MAXINPUT - MAXNAME);
		
		if ( !strncmp(inp,"#q",2) )
		{
			break;
		}
		else if ( !strncmp(inp,"#$",2) )
		{
			if ( issrv )
				srv_myclient(s);
			else
				sck_pack(s,1,0,strlen(inp),inp);
			continue;
		}
		
		if ( issrv )
		{
			if ( !srv_message(s,inp) )
			{
				strcpy(inp,"sys$ message error");
				scr_write(inp,strlen(inp)); 
			}
		}
		else
		{
			if ( !cli_message(s,inp) )
			{
				strcpy(inp,"sys$ message error");
				scr_write(inp,strlen(inp)); 
			}
		}
	}
	
	sck_free(s);
	printf("good bye\n");
	
    return 0;
}

#endif
