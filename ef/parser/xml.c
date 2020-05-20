#include <ef/xml.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/str.h>

#include <stdarg.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

typedef struct xml{
	xmlDoc* doc;
	xmlNode* root;
	xmlNode* nav;
	xmlNode* content;
	xmlNs* namespace;
	xmlAttr* attr;
	xmlNs* attrnamespace;
}xml_s;

#define xml_subset(XML) do{\
	(XML)->content = (XML)->nav->children;\
	(XML)->namespace = (XML)->nav->ns;\
	(XML)->attr = (XML)->nav->properties;\
	(XML)->attrnamespace = (XML)->attr ? (XML)->attr->ns : NULL;\
}while(0)

__private void cbk_errors(__unused void* ctx, const char* msg, ...){
	char str[4096];
	va_list arg_ptr;
	va_start(arg_ptr, msg);
	vsnprintf(str, 4096, msg, arg_ptr);
	va_end(arg_ptr);
	err_push("%s", str);
	return;
}
__private xmlGenericErrorFunc errorfn = cbk_errors;

void xml_begin(void){
	LIBXML_TEST_VERSION
	initGenericErrorDefaultFunc(&errorfn);
}

void xml_end(void){
	xmlCleanupParser();
}

__private err_t node_first(xml_s* xml){
	xmlNode* node = xml->nav;
	for(; xml->nav; xml->nav = xml->nav->next){
		//dbg_info("%d:%s", xml->nav->type, xml->nav->name);
		if( xml->nav->type == XML_ELEMENT_NODE ){
			xml_subset(xml);
			return 0;
		}
	}
	xml->nav = node;
	xml_subset(xml);
	return -1;
}

xml_s* xml_load(const char* path){
	xml_s* xml = mem_new(xml_s);
	if( !xml ) err_fail("eom");

	xml->doc = xmlReadFile(path, NULL, 0);
	if( xml->doc == NULL ){
		free(xml);
		err_push("could not parse file %s", path);
		return NULL;
	}

	xml->root = xmlDocGetRootElement(xml->doc);
	if( !xml->root ){
		xmlFreeDoc(xml->doc);
		free(xml);
		err_push("could not get root element");
		return NULL;
	}
	xml->nav = xml->root;
	node_first(xml);

	return xml;
}

void xml_free(xml_s* xml){
	xmlFreeDoc(xml->doc);
	free(xml);
}

void xml_node_reset(xml_s* xml){
	xml->nav = xml->root;
	xml_subset(xml);
}	

err_t xml_node_next(xml_s* xml){
	xmlNode* old = xml->nav;
	for(xml->nav = xml->nav->next; xml->nav; xml->nav = xml->nav->next){
		if( xml->nav->type == XML_ELEMENT_NODE ){
			xml_subset(xml);
			return 0;
		}
	}
	xml->nav = old;
	return -1;
}

err_t xml_node_child(xml_s* xml){
	if( xml->nav->children ){
		xml->nav = xml->nav->children;
		if( node_first(xml) ){
			xml->nav = xml->nav->parent;
			return -1;
		}
		return 0;
	}
	return -1;
}

err_t xml_node_parent(xml_s* xml){
	if( xml->nav->parent ){
		xml->nav = xml->nav->parent;
		return node_first(xml);
	}
	xml_node_reset(xml);
	return -1;
}

const utf8_t* xml_node_name(xml_s* xml){
	return xml->nav->name;
}

inline __private const utf8_t* nonempty(const utf8_t* str){
	if( !str ) return str;
	const utf8_t* e = str;
	while( *e && (*e == ' ' || *e == '\t' || *e == '\n') ) ++e;	
	return *e ? str : NULL;
}

const utf8_t* xml_node_content(xml_s* xml){
	if( !xml->content ) return nonempty(xml->nav->content);
	return nonempty(xml->content->content);
}

const utf8_t* xml_node_namespace(xml_s* xml){
	if( xml->namespace ){
		const utf8_t* name = xml->namespace->prefix;
		xml->namespace = xml->namespace->next;
		return name;
	}
	xml->namespace = xml->nav->nsDef;
	return NULL;
}

int xml_attr_have(xml_s* xml){
	return xml->attr ? 1 : 0;
}

err_t xml_attr_next(xml_s* xml){
	for(xml->attr = xml->attr->next; xml->attr; xml->attr = xml->attr->next){
		if( xml->attr->type == XML_ATTRIBUTE_NODE ){
			xml->attrnamespace = xml->attr->ns;
			return 0;
		}
	}
	xml->attr = xml->nav->properties;
	xml->attrnamespace = xml->attr ? xml->attr->ns : NULL;
	return -1;
}

const utf8_t* xml_attr_name(xml_s* xml){
	return xml->attr ? xml->attr->name : NULL;
}

const utf8_t* xml_attr_value(xml_s* xml){
	if( xml->attr && xml->attr->children){
		return nonempty(xml->attr->children->content);
	}
	return NULL;
}

const utf8_t* xml_attr_namespace(xml_s* xml){
	if( xml->attrnamespace ){
		const utf8_t* name = xml->attrnamespace->prefix;
		xml->attrnamespace = xml->attrnamespace->next;
		return name;
	}
	if( xml->attr ) xml->attrnamespace = xml->attr->ns;
	return NULL;
}

__private void dumpatt(xmlNode* node, int nr){
	xmlAttr* att = node->properties;
	for(; att; att = att->next){
		for( int i = 0; i < nr; ++i) fputs("  ", stdout);
		printf("[%d | %d %s]::",att->atype, att->type, att->name);
		for( xmlNode* n = att->children; n; n = n->next){
			printf("|%s<%s>(%p)|", n->name, nonempty(n->content), n->children);
		}
		putchar('\n');
	}	
}

__private void dump(xmlNode* node, int nr){
	for(; node; node = node->next){
		for( int i = 0; i < nr; ++i) fputs("  ", stdout);
		printf("(%d)%s<%s>\n",node->type, node->name, nonempty(node->content));
		dumpatt(node, nr+1);
		dump(node->children, nr+1);
	}
}

void xml_dump(xml_s* xml){
	dump(xml->root, 0);
}






