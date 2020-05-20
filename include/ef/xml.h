#ifndef __EF_XML_H__
#define __EF_XML_H__

#include <ef/type.h>
#include <ef/utf8.h>

typedef struct xml xml_s;

typedef enum {
	XML_DEF_INT,
	XML_DEF_UINT,
	XML_DEF_LONG,
	XML_DEF_ULONG,
	XML_DEF_STRING,
	XML_DEF_NUMBER,
	XML_DEF_VECTOR,
	XML_DEF_OBJECT
}xmlDef_e;

typedef struct xmlDef{
	struct xmlDef* vintrospect;
	struct xmlDef* parent;
	const char* name;
	xmlDef_e type;
	size_t offof;
	size_t size;
}xmlDef_s;

void xml_begin(void);

void xml_end(void);

xml_s* xml_load(const char* path);

void xml_free(xml_s* xml);

void xml_node_reset(xml_s* xml);
	
err_t xml_node_next(xml_s* xml);

err_t xml_node_child(xml_s* xml);

err_t xml_node_parent(xml_s* xml);

const utf8_t* xml_node_name(xml_s* xml);

const utf8_t* xml_node_content(xml_s* xml);

const utf8_t* xml_node_namespace(xml_s* xml);

int xml_attr_have(xml_s* xml);

err_t xml_attr_next(xml_s* xml);

const utf8_t* xml_attr_name(xml_s* xml);

const utf8_t* xml_attr_value(xml_s* xml);

const utf8_t* xml_attr_namespace(xml_s* xml);

void xml_dump(xml_s* xml);





#endif
