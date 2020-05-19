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
	xmlNs* namespace;
	xmlAttr* attr;
	xmlNs* attrnamespace;
}xml_s;

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
	xml->namespace = xml->nav->ns;
	xml->attr = xml->nav->properties;
	xml->attrnamespace = xml->attr ? xml->attr->ns : NULL;

	return xml;
}

void xml_free(xml_s* xml){
	xmlFreeDoc(xml->doc);
	free(xml);
}

void xml_node_reset(xml_s* xml){
	xml->nav = xml->root;
	xml->namespace = xml->nav->ns;
	xml->attr = xml->nav->properties;
	xml->attrnamespace = xml->attr ? xml->attr->ns : NULL;
}	

err_t xml_node_next(xml_s* xml){
	for(; xml->nav; xml->nav = xml->nav->next){
		if( xml->nav->type == XML_ELEMENT_NODE ){
			xml->namespace = xml->nav->nsDef;
			xml->attr = xml->nav->properties;
			xml->attrnamespace = xml->attr ? xml->attr->ns : NULL;
			return 0;
		}
	}
	xml_node_reset(xml);
	return -1;
}

err_t xml_node_child(xml_s* xml){
	if( xml->nav->children ){
		xml->nav = xml->nav->children;
		xml->namespace = xml->nav->nsDef;
		xml->attr = xml->nav->properties;
		xml->attrnamespace = xml->attr ? xml->attr->ns : NULL;
		return 0;
	}
	xml_node_reset(xml);
	return -1;
}

err_t xml_node_parent(xml_s* xml){
	if( xml->nav->parent ){
		xml->nav = xml->nav->parent;
		xml->namespace = xml->nav->nsDef;
		xml->attr = xml->nav->properties;
		xml->attrnamespace = xml->attr ? xml->attr->ns : NULL;
		return 0;
	}
	xml_node_reset(xml);
	return -1;
}

const utf8_t* xml_node_name(xml_s* xml){
	return xml->nav->name;
}

const utf8_t* xml_node_content(xml_s* xml){
	return xml->nav->content;
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

err_t xml_attr_next(xml_s* xml){
	for(; xml->attr; xml->attr = xml->attr->next){
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

//TODO attribute content???
void xml_attr_value(xml_s* xml){
	if( xml->attr && xml->attr->children){
		dbg_info("type:%d name:%s content:%s", xml->attr->children->type, xml->attr->children->name, xml->attr->children->content);
	}
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








