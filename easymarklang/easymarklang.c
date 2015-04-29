#include "easymarklang.h"
#include <easystring.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
	#include <easybenchmark.h>
	extern BCH_PERF_GLOBAL;
#endif

static const CHAR* DOCNAME = "__document__";

VOID eml_init(EML* eml)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	eml->data = NULL;
	eml->tag = NULL;
	eml->perr = NULL;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
}	

/// /// ///
/// TAG ///
/// /// ///

EMLTAG* eml_tag_new(TAGTYPE t, CHAR* tagn, EMLATT* att)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	EMLTAG* tag = malloc(sizeof(EMLTAG));
	tag->type = t;
	tag->tag = tagn;
	tag->att = att;
	
	tag->next = NULL;
	tag->prev = NULL;
	tag->child = NULL;
	tag->parent = NULL;
	tag->last = NULL;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return tag;
}


VOID eml_tag_add(EML* eml, EMLTAG* from, BOOL ischild, EMLTAG *tag)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	if ( !eml->tag )
	{
		eml->tag = tag;
		return;
	}
	
	if ( ischild )
	{
		tag->parent = from;
		if ( from->child )
		{
			EMLTAG* l = from->child->last;//eml_tag_nextlast(from->child);
			l->next = tag;
			tag->prev = l;
			from->child->last = tag;
		}
		else
		{
			from->child = tag;
			tag->last = tag;
		}
		
		return;
	}
	
	EMLTAG* l = from->last;//eml_tag_nextlast(from);
	l->next = tag;
	tag->prev = l;
	from->last = tag;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
}

EMLTAG* eml_tag_remove(EML* eml, EMLTAG *tag)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	if ( eml->tag == tag )
	{
		eml->tag = eml->tag->next;
		if ( eml->tag ) eml->tag->prev = NULL;
	}
	else
	{
		if ( tag->prev ) tag->prev->next = tag->next;
		if ( tag->next ) tag->next->prev = tag->prev;
		if ( tag->parent && tag->parent->child == tag)
		{
			if ( tag->prev ) 
				tag->parent->child = tag->prev;
			else 
				tag->parent->child = tag->next;
			if ( tag->parent->child ) tag->parent->child->last = eml_tag_nextlast(tag->parent->child);
		}
	}
	
	tag->prev = NULL;
	tag->next = NULL;
	tag->parent = NULL;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return tag;
}

VOID eml_tag_free(EMLTAG* tag)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	if ( tag->child ) eml_tag_free(tag->child);
	if ( tag->next ) eml_tag_free(tag->next);
	if ( tag->att) eml_att_free(tag->att);
	free(tag);
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
}
	

EMLTAG* eml_tag_nextlast(register EMLTAG* tag)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	for (; tag->next; tag = tag->next);
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return tag;
}

EMLTAG* eml_tag_find(EMLTAG* tag, CHAR* name)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	EMLTAG* ret;
	
	for (; tag && strcmp(tag->tag,name); tag = tag->next)
	{
		if ( tag->child )
		{
			ret = eml_tag_find(tag->child,name);
			if ( ret ) 
			{
				#ifdef _DEBUG
					BCH_PERF_STOP;
				#endif
				return ret;
			}
		}
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return tag;
}

/// ///////// ///
/// ATTRIBUTE ///
/// ///////// ///

VOID eml_tag_att_add(EMLTAG* tag, CHAR* name, CHAR* val)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	EMLATT* att = malloc(sizeof(EMLATT));
	att->name = name;
	att->val = val;
	
	att->prev = NULL;
	att->next = tag->att;
	if ( tag->att ) tag->att->prev = att;
	tag->att = att;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
}

EMLATT* eml_tag_att_remove(EMLTAG* tag, EMLATT* att)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	if ( !att->prev )
	{
		tag->att = tag->att->next;
		if ( tag->att ) tag->att->prev = NULL;
	}
	else if ( !att->next )
	{
		att->prev->next = NULL;
	}
	else
	{
		att->prev->next = att->next;
		att->next->prev = att->prev;
	}
	
	att->next = NULL;
	att->prev = NULL;
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return att;
}
	
VOID eml_att_free(EMLATT* att)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	EMLATT* nx;
	for (; att; att = nx)
	{
		nx = att->next;
		free(att);
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
}
	
EMLATT* eml_att_find(EMLATT* att, CHAR* name)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	for(; att && strcmp(att->name,name); att = att->next);
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
			
	return att;
}
	
/// ////// ///
/// PARSER ///
/// ////// ///

TAGTYPE _parse_tag_type(register CHAR** data)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	CHAR* d = *data;
	
	if ( *d == '!' )
	{
		++d;
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		if ( *d == '-' )
		{
			++d;
			if ( *d != '-' ) return TT_ERR;
			++d;
			*data = d;
			return TT_COMMENT;
		}
		*data = d;
		return TT_CDATA;
	}
					
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	if ( !*d && !(*d >= 'a' && * d <= 'z') && !(*d >= 'A' && * d <= 'Z') ) return TT_ERR;
	return TT_TAG;
}

#define _EML_PARSE_ERR -1
#define _EML_PARSE_OK 0
#define _EML_PARSE_CLOSETAG 1
#define _EML_PARSE_ENDTAG 2
#define _EML_PARSE_NOTAG 3
#define _EML_PARSE_TAG 4

CHAR* _parse_tag_name(register CHAR** data, INT32* endt)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register CHAR* d = *data;
	CHAR* st = d;
	
		if ( !*d && !(*d >= 'a' && * d <= 'z') && !(*d >= 'A' && * d <= 'Z') ) {*data = d; *endt = _EML_PARSE_ERR; return NULL;}
	d = str_movetos(d," />\t");
		if ( !*d )
		{
			#ifdef _DEBUG
				BCH_PERF_STOP;
			#endif
			return NULL;
		}
	if ( *d == '/' )
	{
		if ( *(d+1) != '>' ) {*data = d; *endt = _EML_PARSE_ERR; return NULL;}
		*d = '\0';
		d += 2;
		*endt = _EML_PARSE_ENDTAG;
	}
	else if ( *d == '>' )
	{
		*d = '\0';
		++d;
		*endt = _EML_PARSE_CLOSETAG;
	}
	else
	{
		*d = '\0';
		++d;
		*endt = _EML_PARSE_OK;
	}
	
	*data = d;
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return st;
}
	
EMLATT* _parse_tag_att(register CHAR** data,register INT32* endt)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register CHAR* d = *data;
	
	EMLATT* att = NULL;
	EMLATT* newatt = NULL;
	CHAR* en;
	
	*endt = 0;
	
	while ( 1 )
	{
		d = str_firstvalidchar(d);
			if ( !*d ) { *endt = _EML_PARSE_ERR; return NULL;}
		if ( *d == '/' )
		{
			if ( *(d+1) != '>' ) {*data = d; *endt = _EML_PARSE_ERR; return NULL;}
			d += 2;
			*data = d;
			*endt = _EML_PARSE_ENDTAG;
			return att;
		}
		else if ( *d == '>' )
		{
			++d;
			*data = d;
			*endt = _EML_PARSE_CLOSETAG;
			return att;
		}
		newatt = malloc(sizeof(EMLATT));
		newatt->prev = NULL;
		newatt->next = NULL;
		newatt->name = NULL;
		newatt->val = NULL;
		
		///get attname
		newatt->name = d;
		d = str_movetos(d," =");
			if ( !*d ) goto PERR;
		en = d;
		d = str_skipspace(d);
			if ( *d != '=' ) goto PERR;
		++d;
		d = str_skipspace(d);
			if ( *d != '\"' ) goto PERR;
		++d;
		*en = '\0';
		
		///get attval
		newatt->val = d;
		d = str_movetoc(d,'\"');
			if ( !*d ) goto PERR;
		*d = '\0';
		++d;
		///add
		if ( !att )
			att = newatt;
		else
		{
			newatt->next = att;
			att->prev = newatt;
			att = newatt;
		}
	}
	
	*endt = _EML_PARSE_OK;
	*data = d;
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return att;
	
	PERR:
	free(newatt);
	*endt = _EML_PARSE_ERR; 
	for(; att; att = newatt);
	{
		newatt = att->next;
		free(att);
	}
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return NULL;
}

CHAR* _parse_tag_content(register CHAR** data,register INT32* ext)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register CHAR* d = *data;
	CHAR* st = d;
	
	d = str_firstvalidchar(d);
	d = str_movetoc(d,'<');
	if ( *d == '<' )
	{
		*d = '\0';
		*data = d + 1;
		*ext = _EML_PARSE_TAG;
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		return st;
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	*ext = _EML_PARSE_NOTAG;
	return st;
}

INT32 _parse_close(register CHAR** data, EMLTAG* tag)
{	
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register CHAR* d = *data;
	if ( *d != '/' ) 
	{
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		return -1;
	}
	++d;
	INT32 l = strlen(tag->tag);
	if ( strncmp(d,tag->tag,l) )
	{
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		return -1;
	}
	d += l;
	if ( *d != '>' )
	{
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		return -1;
	}
	*data = d + 1;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return 0;
}

INT32 _eml_prepro(register CHAR** data,EML* eml, EMLTAG* nwt)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	switch ( nwt->type )
	{
		default: case TT_ERR: case TT_TAG: break;
		
		case TT_COMMENT:
			nwt->type = TT_COMMENT;
			nwt->tag = *data;
			while ( **data )
			{
				eml->perr = *data;
				*data = str_movetoc(*data,'-');
				if ( !**data ) return EML_ERR_CLOSE;
				if ( !strncmp(*data,"-->",3)) 
				{
					**data = '\0'; *data += 3; 
					#ifdef _DEBUG
						BCH_PERF_STOP;
					#endif
					return _EML_PARSE_OK;}
			}
		break;
		
		case TT_CDATA:
		break;
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	eml->perr = *data;
	return EML_ERR_PREPRO;
}

INT32 _parse_tag(register CHAR** data,EML* eml, EMLTAG* nwt)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	EMLTAG* tag;
	INT32 ext;
	
	eml->perr = *data;
	nwt->type = _parse_tag_type(data);
		if ( nwt->type == TT_ERR ) {ext = EML_ERR_TAG; goto PERR;}
	if ( nwt->type != TT_TAG )
	{
		ext = _eml_prepro(data,eml,nwt);
		if ( ext != _EML_PARSE_OK ) return ext;
		*data = str_firstvalidchar(*data);
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		return 0;
	}
	
	
	eml->perr = *data;
	nwt->tag = _parse_tag_name(data,&ext);
		if ( !nwt->tag ) {ext = EML_ERR_TAG; goto PERR;}
		
	if ( ext == _EML_PARSE_OK )
	{
		nwt->att = _parse_tag_att(data,&ext);
			if ( ext == _EML_PARSE_ERR ) {eml->perr = *data; ext = EML_ERR_ATT; goto PERR;}
	}

	if ( ext == _EML_PARSE_ENDTAG )
	{
		*data = str_firstvalidchar(*data);
		eml->perr = NULL;
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		return 0;
	}
	
	while ( 1 )
	{
		CHAR* content = _parse_tag_content(data,&ext);
			if ( ext == _EML_PARSE_NOTAG ) {eml->perr = *data; ext = EML_ERR_CLOSE; goto PERR;}
		tag = eml_tag_new(TT_CONTENT,content,NULL);
		eml_tag_add(eml,nwt,TRUE,tag);
		
		if ( _parse_close(data,nwt) == _EML_PARSE_OK)
		{
			break;
		}
		else
		{
			tag = eml_tag_new(TT_ERR,NULL,NULL);
			eml_tag_add(eml,nwt,TRUE,tag);
			ext = _parse_tag(data,eml,tag);
			if ( ext ) {goto PERR;}
		}
	}
	
	eml->perr = NULL;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return 0;
	
	PERR:
	if ( nwt->prev )
	{
		nwt->prev->next = NULL;
		free(nwt);
	}
	else if ( nwt->parent )
	{
		nwt->parent->child = NULL;
		free(nwt);
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return ext;
}
			

INT32 eml_parse(EML* eml)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	CHAR* d = eml->data;
	EMLTAG* tag;
	INT32 ext;
	
	eml->tag = eml_tag_new(TT_TAG,(CHAR*)DOCNAME,NULL);
	
	while ( 1 )
	{
		CHAR* content = _parse_tag_content(&d,&ext);
		tag = eml_tag_new(TT_CONTENT,content,NULL);
		eml_tag_add(eml,eml->tag,TRUE,tag);
			
		if ( ext == _EML_PARSE_NOTAG ) break;
		
		tag = eml_tag_new(TT_ERR,NULL,NULL);
		eml_tag_add(eml,eml->tag,TRUE,tag);
		ext = _parse_tag(&d,eml,tag);
		if ( ext != _EML_PARSE_OK ) return ext;
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return 0;
}


BOOL _cobweb(EMLTAG* tag, INT32 mode)
{
	CHAR* f;
	
	if ( mode & EML_SPIDER_TTERR && tag->type == TT_ERR ) return TRUE; 
	
	if ( mode & EML_SPIDER_TRIM )
	{
		f = str_firstvalidchar(tag->tag);
		if ( !*f ) return TRUE;
	}
	
	if ( mode & EML_SPIDER_LTRIM )
	{
		tag->tag = str_firstvalidchar(tag->tag);
	}
	
	if ( mode & EML_SPIDER_RTRIM )
	{
		
		CHAR* os = NULL;
		f = tag->tag;
		
		f = str_movetos(f," \t\n");
		
		while (*f)
		{	
			os = f;
			f = str_skips(f," \t\n");
			f = str_movetos(f," \t\n");
		}
		
		if ( os ) *os = '\0';
	}
	
	return FALSE;
}

VOID eml_spiderclean(EML* eml, EMLTAG* tag, INT32 mode)
{
	if ( tag->child ) eml_spiderclean(eml, tag->child, mode);
	if ( tag->next ) eml_spiderclean(eml, tag->next, mode);
	
	if ( _cobweb(tag,mode) )
	{
		tag = eml_tag_remove(eml,tag);
		eml_tag_free(tag);
	}
}
	
	
	
	
	
	
	
