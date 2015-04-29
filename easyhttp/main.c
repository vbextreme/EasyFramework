#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "easyhttp.h"
#include <easyconsole.h>
#include <easyopt.h>
#include <easystring.h>

#define MODE_GET 0
#define MODE_POST 1
#define MODE_POSTFORM 2
#define MODE_DOWNLOAD 3
#define MODE_UPLOAD 4
#define MODE_LIST 5
#define MODE_RENAME 6
#define MODE_DELETE 7
#define MODE_MKDIR 8
#define MODE_RMDIR 9
#define MODE_DEBUG 9999

/*
VOID rpro(VOID* data, FLOAT64 wr, FLOAT64 tot, FLOAT64 sp)
{
	printf("%8.1f/%8.1f    %8.4f - %8.4f\n",wr,tot,sp/1024.0,(tot - wr) / sp);
}
*/

static UINT32 cy,cx;

VOID progressprint()
{
	puts("-               name               -       size kb       - speed - tileft -");
	con_getrc(&cy,&cx);
}

VOID dataprogress(VOID* data, FLOAT64 wr, FLOAT64 tot, FLOAT64 sp)
{
	HTPDATA* d = (HTPDATA*) data;
	
	con_gotorc(cy,cx);
	printf("-%34.34s-%10.2f/%10.2f-%7.2f-%8.2f-",d->url,wr/1024.0,tot/1024.0,sp/1024.0,(tot - wr) / sp);
	con_flush();
}

VOID duprogress(VOID* data, FLOAT64 wr, FLOAT64 tot, FLOAT64 sp)
{
	HTPDOWNUP d = (HTPDOWNUP) data;
	
	con_gotorc(cy,cx);
	printf("-%34.34s-%10.2f/%10.2f-%7.2f-%8.2f-",htp_downup_url(d),wr/1024.0,tot/1024.0,sp/1024.0,(tot - wr) / sp);
	con_flush();
}

VOID errprint()
{
	printf("Error:%s\n",htp_errstr());
}

int main(INT32 argc, CHAR** argv)
{
	
	MYOPT opt = opt_new("help,ssl,"
						"Http:(url),Ftp:(url),Email:(url),"
						"User:,Password:,"
						"~Get::(extra),~Post:,~postForm:,"
						"~Download:,~Upload:,List::([email]dir+f info+f [generic]header body full),"
						"Rename:(from),Dest:(to),Kill:,~Mkdir:,~Rmdir:,"
						"debug");
	
	BOOL httpmode = FALSE;
	BOOL ftpmode = FALSE;
	BOOL emailmode = FALSE;
	INT32 mode = 0;
	BOOL ssl = FALSE;
	CHAR url[1024] = "\0";
	CHAR arg0[1024] = "\0";
	CHAR arg1[1024] = "\0";
	CHAR usr[1024] = "\0";
	CHAR psw[1024] = "\0";
	CHAR* u = NULL;
	CHAR* p = NULL;
	
	INT32 o;
	CHAR* arg;
	
	o = opt_parse(&arg,opt,argc,argv);
	do
	{
		switch ( o )
		{
			default: case OPTLONG: opt_usage("htp",opt); return 0;
			case OPTLONG + 1: ssl = TRUE; break;
			case OPTLONG + 18: mode = MODE_DEBUG; break;
			case 'h': httpmode = TRUE; strcpy(url,arg); break;
			case 'f': ftpmode = TRUE; strcpy(url,arg); break;
			case 'e': emailmode = TRUE; strcpy(url,arg); break;
			case 'u': strcpy(usr,arg); u = usr; break;
			case 'p': strcpy(psw,arg); p = psw; break;
			case 'G': if ( arg ) strcpy(arg0,arg); mode = MODE_GET; break;
			case 'P': strcpy(arg0,arg);	mode = MODE_POST; break;
			case 'F': strcpy(arg0,arg);	mode = MODE_POSTFORM; break;
			case 'D': strcpy(arg0,arg); mode = MODE_DOWNLOAD; break;
			case 'U': strcpy(arg0,arg); mode = MODE_UPLOAD; break;
			case 'l': mode = MODE_LIST; if(arg) strcpy(arg0,arg); break;
			case 'r': strcpy(arg0,arg); mode = MODE_RENAME; break;
			case 'd': strcpy(arg1,arg); break;
			case 'k': strcpy(arg0,arg); mode = MODE_DELETE; break;
			case 'M': strcpy(arg0,arg); mode = MODE_MKDIR; break;
			case 'R': strcpy(arg0,arg); mode = MODE_RMDIR; break;
		}
	}while( -1 != (o = opt_parse(&arg,opt,argc,argv)) );
	
	htp_init();
	
	HTPDATA* d;
	HTPDOWNUP h;
	EMALIST* el;
	EMALINFO* ei;
	EEMAIL* em;
	
	switch (mode)
	{
		case MODE_DEBUG:
			puts("DBG");
			em = ema_list(url,"Inbox",1,2,FALSE,u,p,ssl);
			if( em )
			{ 
				//printf("<HEADER>\n%s\n</HEADER>\n",d->header.data);
				//printf("<BODY>\n%s\n</BODY>\n",d->body.data);
				
				printf("<LIST>\n");
					INT32 i;
					for ( i = 0; i < em[0].rfile; ++i )
					{
						printf("Date:%s\n",em[i].date);
						printf("From:%s\n",em[i].from);
						printf("To:%s\n",em[i].to);
						printf("Subject:%s\n",em[i].subject);
						printf("Body:%128.128s\n",em[i].body);
						printf("--------------------------------------------------\n");
					}
				printf("</LIST>\n");	
				//ema_list_free(el);
				
				//htp_data_free(d);
			}
			else
				htp_errstr();
		break;
		
		case MODE_GET:
			if ( !httpmode ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg0[0] != '\0' ) { strcat(url,"?"); strcat(url,arg0); }
			progressprint();
			d = htp_get(url,ssl,NULL,dataprogress);
			putchar('\n');
			if ( d )
			{
				printf("%s\n",d->body.data);
				htp_data_free(d);
			}
			else
				errprint();
		break;
		
		case MODE_POST:
			if ( !httpmode ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			progressprint();
			d = htp_post(url,arg0,strlen(arg0),ssl,NULL,dataprogress);
			putchar('\n');
			if ( d )
			{
				printf("%s\n",d->body.data);
				htp_data_free(d);
			}
			else
				errprint();
		break;
		
		case MODE_POSTFORM:
			if ( !httpmode ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			
			HTPFORM f = htp_form_new(url,ssl,NULL,dataprogress);
			
			CHAR* pa = arg0;
			CHAR* n;
			CHAR* v;
			
			while(1)
			{
				n = pa;
				pa = str_movetoc(pa,'=');
				*pa++ = '\0';
				v = pa;
				pa = str_movetoc(pa,'&');
				if ( !*pa ) 
				{
					htp_form_add(f,n,v,FALSE);
					break;
				}
				*pa++ = '\0';
				htp_form_add(f,n,v,FALSE);
			}
			
			progressprint();
			d = htp_form(f);
			putchar('\n');
			if ( d )
			{
				printf("%s\n",d->body.data);
				htp_form_free(f);
			}
			else
				errprint();
		break;
		
		case MODE_DOWNLOAD:
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			progressprint();
			if ( httpmode )
			{
				h = htp_download(arg0,url,FALSE,ssl,duprogress);
			}
			else
			{
				h = ftp_download(arg0,url,u,p,FALSE,ssl,duprogress);
			}
			if ( htp_downup_complete(h,TRUE) == -1 ) {putchar('\n');errprint();}
			putchar('\n');
			
			htp_downup_free(h);
		break;
	
		case MODE_UPLOAD:
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( httpmode )
			{
				htp_terminate(); 
				opt_usage("htp",opt); 
				return 0;
			}
			else
			{
				progressprint();
				h = ftp_upload(url,u,p,arg0,ssl,duprogress);
			}
			
			if ( htp_downup_complete(h,TRUE) == -1 ) errprint();
			putchar('\n');
			htp_downup_free(h);
		break;
		
		case MODE_LIST:
			if ( ftpmode )
			{
				progressprint();
				FTPLIST* fl = ftp_list(url,u,p,ssl,NULL,dataprogress);
				putchar('\n');
				if ( fl )
				{
					INT32 i;
					for ( i = 0; i < fl[0].rfile; ++i)
					{
						printf("%s - %s\n",(fl[i].ftype == FTP_FTYPE_DIR) ? "D" : (fl[i].ftype == FTP_FTYPE_REG) ? "R" : (fl[i].ftype == FTP_FTYPE_LINK) ? "L" : "E",fl[i].name);
					}
				
					if (!strcmp(arg0,"header") || !strcmp(arg0,"full") )
					{
						printf("<HEADER>\n%s\n<HEADER>\n",fl[0].d->header.data);
					}
					if (!strcmp(arg0,"body") || !strcmp(arg0,"full") )
					{
						printf("<BODY>\n%s\n<BODY>\n",fl[0].d->body.data);
					}
				
					ftp_list_free(fl);
				}
				else
					errprint();
			}
			else if (emailmode )
			{
				
				if ( !arg0[0] || !strncmp(arg0,"dir",3)  )
				{
					el = ema_dirlist(url,u,p,ssl);
						if ( !el ) {errprint(); break;}
					
					if ( arg0[0] && arg[3] == 'f' )
					{
						printf("<HEADER>\n%s\n</HEADER>\n",el[0].d->header.data);
						printf("<BODY>\n%s\n</BODY>\n",el[0].d->body.data);
					}
					
					printf("<LIST>\n");
					INT32 i;
					for ( i = 0; i < el[0].rfile; ++i )
						printf("%s\n",el[i].name);
					printf("</LIST>\n");	
					ema_list_free(el);
				}
				else if ( !strncmp(arg0,"info",4)  )
				{
					if ( !arg1[0] ) { htp_terminate(); opt_usage("htp",opt); return 0;}
					ei = ema_infodir(url,arg1,u,p,ssl);
						if ( !ei ) {errprint(); break;}
						
					if ( arg[4] == 'f' )
					{
						printf("<HEADER>\n%s\n</HEADER>\n",ei->d->header.data);
						printf("<BODY>\n%s\n</BODY>\n",ei->d->body.data);
					}
					printf("count:%u\nrecent:%u\n",ei->count,ei->recent);
					
					ema_linfo_free(ei);
				}
				else
				{ 
					htp_terminate();
					opt_usage("htp",opt);
					return 0;
				}
			}
			else
			{ 
				htp_terminate();
				opt_usage("htp",opt);
				return 0;
			}
			
		break;
		
		case MODE_RENAME:
			if ( !ftpmode ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg1[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( ftp_rename(url,arg0,arg1,u,p,ssl) == -1 )
				errprint();
		break;
		
		case MODE_DELETE:
			if ( !ftpmode ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( ftp_delete(url,arg0,u,p,ssl) == -1 )
				errprint();
		break;
		
		case MODE_MKDIR:
			if ( !ftpmode ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( ftp_mkdir(url,arg0,u,p,ssl) == -1 )
				errprint();
		break;
		
		case MODE_RMDIR:
			if ( !ftpmode ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( arg0[0] == '\0' ) { htp_terminate(); opt_usage("htp",opt); return 0;}
			if ( ftp_rmdir(url,arg0,u,p,ssl) == -1 )
				errprint();
		break;
		
		
	}
	
	htp_terminate();
	
	/*
	puts("Test ftp download");
	
	CHAR* url = "ftp://ftp.gnu.org/README";
	HTPDOWNUP h = ftp_download("TEST",url,NULL,NULL,FALSE,TRUE,rpro);
	
	if ( htp_downup_complete(h,TRUE) == -1 )
	{
		puts("error\n");
	}
	else
	{
		puts("ok\n");
	}
	
	htp_downup_free(h);
	*/
	/*
	CHAR site[1024] = "ftp://ftp.gnu.org/";
	
	printf("%s\n",site);
	
	if ( !url_walk(site,"./gnu") ) {puts("error"); return 0;}
	printf("%s\n",site);
	
	if ( !url_walk(site,"./ciao") ) {puts("error"); return 0;}
	printf("%s\n",site);
	
	if ( !url_walk(site,"../miao") ) {puts("error"); return 0;}
	printf("%s\n",site);
	
	if ( !url_walk(site,"..") ) {puts("error"); return 0;}
	printf("%s\n",site);
	
	if ( !url_walk(site,"..") ) {puts("error"); return 0;}
	printf("%s\n",site);
	
	if ( !url_walk(site,"ftp://ftp.gnu.org/uno/due/tre") ) {puts("error"); return 0;}
	printf("%s\n",site);
	
	if ( !url_walk(site,"/") ) {puts("error"); return 0;}
	printf("%s\n",site);
	*/
	
	/*
	CHAR* site = "ftp://ftp.gnu.org/gnu/";
	
	FTPLIST* ft = ftp_list(site,NULL,NULL,FALSE,NULL,NULL);
	
	if ( !ft )
	{
		printf("Error:%s\n",htp_errstr());
		htp_terminate();
		return 0;
	}
	
	printf("- rf  -   pri   - t -   size   -       data      -              name              -              link              -\n");
	INT32 i;
	for(i = 0; i < ft[0].rfile; ++i)
		printf("-%5d-%s-%3d-%10u-%17s-%32s-%32s-\n",ft[i].rfile,ft[i].pri,ft[i].ftype,ft[i].size,ft[i].date,ft[i].name,(ft[i].ftype == FTP_FTYPE_LINK) ? ft[i].link : "----------");
	
	ftp_list_free(ft);
	*/
	/*
	HTPFORM f = htp_form_new("http://www.cs.tut.fi/cgi-bin/run/~jkorpela/echo.cgi",NULL,NULL);
	htp_form_add(f,"Comments","supercalifragilistichispiralidoso",FALSE);
	htp_form_add(f,"SUBMIT","Send",FALSE);
	HTPDATA* d = htp_form(f);
	
	if ( d == NULL )
	{
		puts("error");
		return 0;
	}
	
	printf("<HEADER>\n%s\n</HEADER>\n",d->header.data);
	printf("<BODY>\n%s\n</BODY>\n",d->body.data);
	
	htp_form_free(f);
	*/
	
	/*
	//con_msg(&md,"test download",0);
	puts("Test download");
	//CHAR* url = "http://www.biomedcentral.com/content/supplementary/1471-2105-15-258-s9.zip";
	CHAR* url = "http://download.thinkbroadband.com/5MB.zip";
	HTPDOWN h = htp_download("test.zip",url,FALSE,rpro);
	
	if ( htp_download_complete(h,TRUE) == -1 )
	{
		puts("error\n");
		//con_msg(&md,NULL,-1);
	}
	else
	{
		puts("ok\n");
		//con_msg(&md,NULL,100);
	}
	
	htp_download_free(h);
	*/
	/*
	puts("Send request...");
	
	HTPDATA* r = htp_get("http://ipecho.net/plain",NULL,rpro);
	
	if ( r == NULL )
	{
		puts("error");
		htp_terminate();
		return 0;
	}
	
	r->header.data[r->header.rem] = '\0';
	r->body.data[r->body.rem] = '\0';
	
	printf("<HEAD>\n%s\n</HEAD>\n",r->header.data);
	printf("<BODY>\n%s\n</BODY>\n",r->body.data);
	
	htp_data_free(r);
	*/
	
	
	htp_terminate();
	
	return 0;
}

#endif
