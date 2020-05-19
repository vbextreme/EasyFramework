#ifndef __EF_XML_H__
#define __EF_XML_H__

#include <ef/type.h>
#include <ef/utf8.h>

typedef struct xml xml_s;

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

err_t xml_attr_next(xml_s* xml);

const utf8_t* xml_attr_name(xml_s* xml);

void xml_attr_value(xml_s* xml);

const utf8_t* xml_attr_namespace(xml_s* xml);







#endif
