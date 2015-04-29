#ifndef EASYMARKLANG_H_INCLUDED
#define EASYMARKLANG_H_INCLUDED

#include <easytype.h>

//content add & parse

#define EML_ERR_NODATA -1
#define EML_ERR_TAG    -2
#define EML_ERR_ATT    -3
#define EML_ERR_CLOSE  -4
#define EML_ERR_PREPRO -5

#define EML_SPIDER_TTERR  0x01
#define EML_SPIDER_EMPTY  0x02
#define EML_SPIDER_TRIM   0x04
#define EML_SPIDER_LTRIM  0x08
#define EML_SPIDER_RTRIM  0x10
#define EML_SPIDER_FULL   0xFF


typedef enum {TT_ERR = -1, TT_TAG = 0, TT_CONTENT, TT_CDATA, TT_COMMENT} TAGTYPE;

typedef struct _EMLATT
{
	CHAR* name;
	CHAR* val;
	struct _EMLATT* next;
	struct _EMLATT* prev;
}EMLATT;

typedef struct _EMLTAG
{
	TAGTYPE type;
	CHAR *tag;
    EMLATT* att;
    
    struct _EMLTAG* next;
    struct _EMLTAG* prev;
    struct _EMLTAG* parent;
    struct _EMLTAG* child;
    struct _EMLTAG* last;
}EMLTAG;

typedef struct _EML
{
	CHAR* data;
	EMLTAG* tag;
	CHAR* perr;
}EML;

VOID eml_init(EML* eml);
EMLTAG* eml_tag_new(TAGTYPE t, CHAR* tagn, EMLATT* att);
VOID eml_tag_add(EML* eml, EMLTAG* from, BOOL ischild, EMLTAG *tag);
EMLTAG* eml_tag_remove(EML* eml, EMLTAG *tag);
VOID eml_tag_free(EMLTAG* tag);
EMLTAG* eml_tag_nextlast(EMLTAG* tag);
EMLTAG* eml_tag_find(EMLTAG* tag, CHAR* name);

VOID eml_tag_att_add(EMLTAG* tag, CHAR* name, CHAR* val);
EMLATT* eml_tag_att_remove(EMLTAG* tag, EMLATT* att);
VOID eml_att_free(EMLATT* att);
EMLATT* eml_att_find(EMLATT* att, CHAR* name);

INT32 eml_parse(EML* eml);
VOID eml_spiderclean(EML* eml, EMLTAG* tag, INT32 mode);

#endif 
