#include "test.h"
#include <ef/imap.h>
#include <termios.h>

/*@test -i --imap 'test imap, -a imaps://server.com;myemail@email.com -b <ls><show:from:to:dir><delete:id:dir><info:dir>'*/

char* psw_get(void){
	static char psw[1024];
	struct termios told;
	struct termios tnew;
	tcgetattr(0, &told);
	tnew = told;
	tnew.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &tnew);

	fputs("password: ", stdout);
	fflush(stdout);
	if( !fgets(psw, 1024, stdin) ) return NULL;
	
	tcsetattr(0, TCSANOW, &told);
	
	str_chomp(psw);
	return psw;
}

void test_imap_ls(imap_s* email){
	__vector_free char** ls = imap_ls(email);
	if( !ls ){
		dbg_error("ls");
		err_print();
		return;
	}
	vector_foreach(ls, i){
		printf("dir:%s\n", ls[i]);
		free(ls[i]);
	}
}

void test_imap_dir_stat(imap_s* email, const char* dir){
	imapExamine_s ex;
	if( imap_examine(&ex, email, dir) ){
		err_print();
		return;
	}
	printf("examine: exists:%lu recent:%lu idunsee:%ld\n", ex.exists, ex.recent, ex.unseeid);
}

void test_imap_fetch(imap_s* email, int from, int to, const char* dir){
	imapFetch_s* fetch = imap_fetch(email, dir, 1, from, to);
	if( !fetch ){
		err_print();
		return;
	}
	
	size_t count = (to-from)+1;
	for( size_t i = 0; i < count; ++i ){
		printf("id:%lu\n", fetch[i].id);
		printf("flags:%d\n", fetch[i].flag);
		printf("size:%lu\n", fetch[i].size);
		//printf("content:%s\n\n\n", fetch[i].mime);
	}

	imapMime_s* mime = imap_fetch_to_mime(fetch, count);
	free(fetch);

	if( !mime ){
		err_print();
		return;
	}

	for( size_t i = 0; i < count; ++i ){
		printf("-----------------------------------------------------------------------------------------------------------\n");
		printf("- xauth: %s xsid: %s\n", mime[i].xauth ? "pass" : "fail", mime[i].xsid ? "pass" : "fail");
		printf("- from    : %s\n", mime[i].from);
		printf("- to      : %s\n", mime[i].to);
		printf("- reply to: %s\n", mime[i].replyto);
		printf("- date    : %s\n", mime[i].date);
		printf("- subject : %s\n", mime[i].subject);
		imapMimeContent_s* content = mime[i].content;
		while( content ){
			printf("- content.type       : %s\n", content->type);
			printf("- \tcontent.filename   : %s\n", content->filename);
			printf("- \tcontent.disposition: %s\n", content->disposition);
			printf("- \tcontent.dataSize   : %lu\n", content->dataSize);
			content = content->next;
		}
		printf("-----------------------------------------------------------------------------------------------------------\n");
	}
 
	imap_mime_free(mime, count);
}

#define ACTION_LS     0
#define ACTION_INFO   1
#define ACTION_SHOW   2
#define ACTION_DELETE 3

int test_argb(const char** outdir, int* outfrom, int* outto, const char* argB){
	if( !str_ancmp(argB, "ls") ){
		return ACTION_LS;
	}

	if( !str_ancmp(argB, "info") ){
		argB += 4;
		if( *argB++ != ':' ) return -1;
		*outdir = argB;
		return ACTION_INFO;
	}

	if( !str_ancmp(argB, "show") ){
		argB += 4;
		if( *argB++ != ':' ) return -1;
		char* next;
		*outfrom = strtoul(argB, &next, 10);
		
		argB = next;
		if( *argB++ != ':' ) return -1;
		*outto = strtoul(argB, &next, 10);

		argB = next;
		if( *argB++ != ':' ) return -1;

		*outdir = argB;
		return ACTION_SHOW;
	}

	if( !str_ancmp(argB, "delete") ){
		argB += 6;
		if( *argB++ != ':' ) return -1;
		char* next;
		*outfrom = strtoul(argB, &next, 10);
		
		argB = next;
		if( *argB++ != ':' ) return -1;
		*outdir = argB;
		return ACTION_DELETE;
	}

	return -1;
}

char* test_arga(const char** outemail, const char* argA){
	const char* endimap = strchr(argA, ';');
	if( !endimap ) return NULL;
	*outemail = endimap+1;
	return str_dup(argA, endimap - argA);
}

/*@fn*/
void test_imap(const char* argA, __unused const char* argB){
	err_enable();
	if( !argA ){
		fprintf(stderr,"No server/email, write -a imap://server.com;email@email.com\n");
		err_restore();
		return;
	}
	if( !argB ){
		fprintf(stderr,"No action\n");
		err_restore();
		return;
	}

	const char* email = NULL;
	__mem_free char* server = test_arga(&email, argA);
	if( !server ){
		fprintf(stderr,"No server/email, write -a imap://server.com;email@email.com\n");
		err_restore();
		return;
	}
	
	const char* dir = NULL;
	int from = -1;
	int to = -1;
	int mode = test_argb(&dir, &from, &to, argB);
	if( mode < 0 ){
		fprintf(stderr,"action error\n");
		err_restore();
		return;
	}

	dbg_info("server:%s",server);
	dbg_info("email :%s",email);
	dbg_info("action:%d",mode);
	dbg_info("dir   :%s",dir);
	dbg_info("from  :%d",from);
	dbg_info("to    :%d",to);

	imap_s* imap = imap_new(4096, WWW_FLAGS_SSL, NULL);
	if( !imap ){
		err_print();
		return;
	}
	//"imaps://outlook.office365.com"
	imap_server(imap, server);
	imap_email_password(imap, email, psw_get());

	switch( mode ){
		case ACTION_LS: test_imap_ls(imap); break;
		case ACTION_INFO: test_imap_dir_stat(imap, dir); break;
		case ACTION_SHOW: test_imap_fetch(imap, from, to, dir); break;
		case ACTION_DELETE:
			if( imap_flags(imap, dir, from, '+', IMAP_FETCH_FLAGS_DELETED) ){
				err_print();
			}
		break;
	}

	imap_free(imap);
	err_restore();
}
