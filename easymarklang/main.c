#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include "easymarklang.h"
#include <easystring.h>

#ifdef _DEBUG
	#include <easybenchmark.h>
	BCH_PERF_GLOBAL;
#endif

VOID printcontent(CHAR* c)
{
	if ( *c ) printf("%s",c);
}

VOID printtype(EMLTAG* tag,BOOL ext)
{
	switch ( tag->type )
	{
		case TT_ERR:
			printf("__error__");
			if (ext) printf(" type = \"%d\" ",tag->type);
		break;
		case TT_COMMENT:
			printf("__comment__");
			if ( ext ) printf(" type = \"%d\" value = \"%s\" ",tag->type, tag->tag);
		break;
		case TT_CONTENT:
			printf("__content__");
			if ( ext ) printf(" type = \"%d\" value = \"%s\" ",tag->type, tag->tag);
		break;
		case TT_CDATA:
			printf("__cdata__");
			if (ext) printf(" type = \"%d\" ",tag->type);
		break;
		case TT_TAG:
			printf("%s", tag->tag);
			if (ext) printf(" type = \"%d\" ",tag->type);
		break;
	}
}
		

VOID printtag(EMLTAG* tag, INT32 nt)
{
	INT32 i;
	for ( i = 0; i < nt; ++i) putchar('\t');
	
	printf("<");
	printtype(tag,1);
	
	EMLATT* att = tag->att;
	for (; att; att = att->next)
		printf("%s = \"%s\" ",att->name,att->val);
	printf(">\n");
	
	if ( tag->child )
	{
		printtag(tag->child,nt+1);
	}
	
	for ( i = 0; i < nt; ++i) putchar('\t');
	printf("</");
	printtype(tag,0);
	printf(">\n");
	
	if ( tag->next ) printtag(tag->next,nt);
}


int main()
{
	FILE* ft = fopen("test.xml","r");
	fseek(ft,0,SEEK_END);
	UINT32 sz = ftell(ft);
	CHAR* test = malloc(sz + 1);
	fseek(ft,0,SEEK_SET);
	fread(test,1,sz,ft);
	test[sz] = '\0';
	fclose(ft);
	
	//printf("%s\n",test);
	//return 0;
	
	/*
	FILE* ft = fopen("test.xml","w");
	fprintf(ft,"<Tests>\n");
	INT32 i;
	for ( i = 0; i < 10000; ++i )
	{
		fprintf(ft,"    <Test TestId=\"%4d\" TestType=\"CMD\">\n"
				   "        <Name>Find succeeding characters</Name>\n"
				   "        <CommandLine>Examp2.EXE</CommandLine>\n"
			       "        <Input>abc</Input>\n"
				   "        <Output>def</Output>\n"
				   "    </Test>\n",i);
	}
	fprintf(ft,"</Tests>");
	fclose(ft);
	return 0;
	
	CHAR test[] = "<html>\n"
				  "<!--comment-->\n"
				  "    <head>\n"
				  "        nothing\n"
				  "    </head>\n"
				  "    <body>\n"
				  "         page one\n"
				  "         <empty/>\n"
				  "         <img src=\"io.jpg\" width=\"100\"/>\n"
				  "    </body>\n"
				  "</html>";
	*/
	
	#ifdef _DEBUG
		BCH_PERF_INIT;
		BCH_PERF_START;
	#endif
	
	EML eml;
	eml_init(&eml);
	eml.data = test;
	
	
	INT32 ret = eml_parse(&eml);
	if ( ret != 0)
	{
		printf("Error:%d\nFrom[%s]\n",ret,eml.perr);
		return 0;
	}
	
	
	eml_spiderclean(&eml,eml.tag, EML_SPIDER_FULL);
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
		BCH_PERF_SAVE("time.1.out");
	#endif
	
	//return 0;
	
	printtag(eml.tag,0);
	
	eml_tag_free(eml.tag);
	
	return 0;
}


#endif
