#include "gencode.h"

void gc_init(genCode_s* gc){
	gc->h.name     = NULL;
	gc->h.hinclude = ostr_new(DEFAULT_STRING_SIZE);
	gc->h.hproto   = ostr_new(DEFAULT_STRING_SIZE);
	gc->h.hstruct  = ostr_new(DEFAULT_STRING_SIZE);
	gc->h.tmpdocp  = ostr_new(DEFAULT_STRING_SIZE);
	gc->h.tmpprop  = ostr_new(DEFAULT_STRING_SIZE);
	gc->c.cfn      = ostr_new(DEFAULT_STRING_SIZE);
	gc->c.cinclude = ostr_new(DEFAULT_STRING_SIZE);
	gc->c.tmpbody  = ostr_new(DEFAULT_STRING_SIZE);
	gc->c.tmpprop  = ostr_new(DEFAULT_STRING_SIZE);
	gc->c.tmpreply = ostr_new(DEFAULT_STRING_SIZE);
	gc->c.tmpprepare = ostr_new(DEFAULT_STRING_SIZE);
}

void gc_delete(genCode_s* gc){
	ostr_free(gc->h.hinclude);
	ostr_free(gc->h.hproto);
	ostr_free(gc->h.hstruct);
	ostr_free(gc->h.tmpdocp);
	ostr_free(gc->h.tmpprop);
	ostr_free(gc->c.cfn);
	ostr_free(gc->c.cinclude);
	ostr_free(gc->c.tmpbody);
	ostr_free(gc->c.tmpprop);
	ostr_free(gc->c.tmpprepare);
	ostr_free(gc->c.tmpreply);
}

void gc_header_include_guard_begin(genCode_s* gc, const char* headername){
	__mem_free char* uph = str_dup(headername, 0);
	str_toupper(uph, uph);
	str_tr(uph, "/.", '_');
	ostr_printf(&gc->h.hinclude, "#ifndef __%s__\n#define __%s__\n\n", uph, uph);
}

void gc_header_include_guard_end(genCode_s* gc){
	ostr_puts(&gc->h.hproto, "\n#endif\n");
}

void gc_header_include(genCode_s* gc, const char* header){
	ostr_printf(&gc->h.hinclude, "#include <%s>\n", header);
}

err_t gc_header_declare_struct(genCode_s* gc, const char* name){
	__mem_free char* already = str_printf("struct %s", name);
	if( strstr(gc->h.hstruct, already) ) return -1;
	ostr_printf(&gc->h.hstruct, "typedef struct %s{\n", name);
	return 0;
}

void gc_header_declare_struct_element(genCode_s* gc, const char* type, const char* name, const char* doc){
	ostr_printf(&gc->h.hstruct, "\t%s %s; /**< %s*/\n", type, name, doc);
}

void gc_header_declare_struct_end(genCode_s* gc, const char* name){
	ostr_printf(&gc->h.hstruct, "}%s_s;\n\n", name);
}

void gc_header_declare_proto(genCode_s* gc, const char* name){
	ostr_printf(&gc->h.tmpprop, " %s(", name);
	ostr_printf(&gc->h.tmpdocp, "/** %s\n", name);
}

void gc_header_declare_proto_get(genCode_s* gc, const char* name){
	ostr_printf(&gc->h.tmpprop, " %s_get(", name);
	ostr_printf(&gc->h.tmpdocp, "/** property %s_get\n", name);
}

void gc_header_declare_proto_set(genCode_s* gc, const char* name){
	ostr_printf(&gc->h.tmpprop, " %s_set(", name);
	ostr_printf(&gc->h.tmpdocp, "/** property %s_set\n", name);
}

void gc_header_declare_proto_return(genCode_s* gc, const char* type, const char* doc){
	ostr_inscstr(&gc->h.tmpprop, 0, type);
	ostr_printf(&gc->h.tmpdocp, " * @return %s\n", doc);
}

void gc_header_declare_proto_arg(genCode_s* gc, const char* type, const char* name, const char* doc){
	const char* args = strchr(gc->h.tmpprop, '(');
	if( !args ) err_fail("internal proto error, '%s' not have (", gc->h.tmpprop);
	if( args[1] ){
		ostr_printf(&gc->h.tmpprop, ", %s %s", type, name);
	}
	else{
		ostr_printf(&gc->h.tmpprop, "%s %s", type, name);
	}
	ostr_printf(&gc->h.tmpdocp, " * @param %s %s\n", name, doc);
}

void gc_header_declare_proto_end(genCode_s* gc){
	ostr_puts(&gc->h.tmpprop, ");\n\n");
	ostr_puts(&gc->h.tmpdocp, "*/\n");
	ostr_puts(&gc->h.hproto, gc->h.tmpdocp);
	ostr_puts(&gc->h.hproto, gc->h.tmpprop);
	ostr_clear(gc->h.tmpdocp);
	ostr_clear(gc->h.tmpprop);
}

err_t gc_header_proto_separator(genCode_s* gc, const char* str){
	__mem_free char* sname = str_printf("/*** %s ***/", str);
	if( strstr(gc->h.hproto, sname ) ) return -1;

	ostr_puts(&gc->h.hproto, "/");
	int l = strlen(str) + 8;
	for( int i = 0; i < l; ++i)
		ostr_putch(&gc->h.hproto, '*');
	ostr_printf(&gc->h.hproto, "/\n/*** %s ***/\n/", str); 
	for( int i = 0; i < l; ++i)
		ostr_putch(&gc->h.hproto, '*');
	ostr_puts(&gc->h.hproto, "/\n\n");
	return 0;	
}

void gc_code_include(genCode_s* gc, const char* header){
	ostr_printf(&gc->c.cinclude, "#include <%s>\n", header);
}

void gc_code_fn_separator(genCode_s* gc, const char* str){
	ostr_puts(&gc->c.cfn, "/");
	int l = strlen(str) + 8;
	for( int i = 0; i < l; ++i)
		ostr_putch(&gc->c.cfn, '*');
	ostr_printf(&gc->c.cfn, "/\n/*** %s ***/\n/", str); 
	for( int i = 0; i < l; ++i)
		ostr_putch(&gc->c.cfn, '*');
	ostr_puts(&gc->c.cfn, "/\n\n"); 
}

void gc_code_declare_fn(genCode_s* gc, const char* name){
	ostr_printf(&gc->c.tmpprop, " %s(", name);
}

void gc_code_declare_fn_get(genCode_s* gc, const char* name){
	ostr_printf(&gc->c.tmpprop, " %s_get(", name);
}

void gc_code_declare_fn_set(genCode_s* gc, const char* name){
	ostr_printf(&gc->c.tmpprop, " %s_set(", name);
}

void gc_code_declare_fn_return(genCode_s* gc, const char* type){
	ostr_inscstr(&gc->c.tmpprop, 0, type);
}

void gc_code_declare_fn_arg(genCode_s* gc, const char* type, const char* name){
	const char* args = strchr(gc->c.tmpprop, '(');
	if( !args ) err_fail("internal fn error, '%s' not have (", gc->c.tmpprop);
	if( args[1] ){
		ostr_printf(&gc->c.tmpprop, ", %s %s", type, name);
	}
	else{
		ostr_printf(&gc->c.tmpprop, "%s %s", type, name);
	}
}

void gc_code_declare_fn_end(genCode_s* gc){
	ostr_puts(&gc->c.tmpprop, "){\n");
}

void gc_code_fn_method_prepare_struct(genCode_s* gc, const char* name, const char* sname, char** vename){
	ostr_printf(&gc->c.tmpprepare, "\tvoid** %s = vector_new(void*, %lu, NULL);\n", name, vector_count(vename));
	vector_foreach(vename, i){
		ostr_printf(&gc->c.tmpprepare, "\tvector_push_back(%s, &%s->%s);\n", name, sname, vename[i]);
	}
}

void gc_code_fn_method_begin(genCode_s* gc, const char* service, const char* object, const char* interface, const char* method){
	ostr_printf(&gc->c.tmpbody,
		"\tsdbusMessage_h msg = sdbus_method(\n\t\tsd,\n\t\t\"%s\", \"%s\",\n\t\t\"%s\", \"%s\"", 
		service, object, interface, method
	);
}

void gc_code_fn_call_method_add_arg(genCode_s* gc, const char* arg){
	ostr_printf(&gc->c.tmpbody, ",\n\t\t%s", arg);
}

void gc_code_fn_method_close(genCode_s* gc, size_t nargs){
	if( nargs ){
		ostr_puts(&gc->c.tmpbody, "\n\t);\n");
	}
	else{
		ostr_puts(&gc->c.tmpbody, ",\n\t\tNULL\n\t);\n");
	}
}

void gc_code_fn_method_test(genCode_s* gc){
	ostr_puts(&gc->c.tmpbody, "\tif( !msg ){\n\t\treturn NULL;\n\t}\n\n");
}

void gc_code_property_set(genCode_s* gc, const char* service, const char* object, const char* interface, const char* method, const char* variantname){
	ostr_printf(&gc->c.tmpbody,
		"\tif( sdbus_property_set(\n\t\t\tsd,\n\t\t\t\"%s\", \"%s\",\n\t\t\t\"%s\", \"%s\", \n\t\t\t&%s\n\t\t)\n\t){\n\t\treturn -1;\n\t}\n",
		service, object, interface, method, variantname
	);
}

void gc_code_property_get(genCode_s* gc, const char* service, const char* object, const char* interface, const char* method, const char* variantname){
	ostr_printf(&gc->c.tmpbody,
		"\tsdbusMessage_h msg = sdbus_property_get(\n\t\tsd,\n\t\t\"%s\", \"%s\",\n\t\t\"%s\", \"%s\",\n\t\t&%s\n\t);\n", 
		service, object, interface, method, variantname
	);
	ostr_puts(&gc->c.tmpbody, "\tif( !msg ){\n\t\treturn NULL;\n\t}\n\n");
}

void gc_code_property_variant(genCode_s* gc, const char* variantname, const char* sig){
	ostr_printf(&gc->c.tmpbody,	"\tsdbusVariant_s %s;\n", variantname);
	if( sig ) ostr_printf(&gc->c.tmpbody, "\t%s.type = \"%s\";\n", variantname, sig);
}

void gc_code_property_variant_set(genCode_s* gc, const char* variantname, const char* sig, const char* varname){
	if( *sig == 'a' || *sig == '{' || *sig == 'v' || *sig == '(' ){
		ostr_printf(&gc->c.tmpbody,	"\t%s.var.v = %s;\n", variantname, varname);
	}
	else{
		ostr_printf(&gc->c.tmpbody,	"\t%s.var.%c = %s;\n", variantname, *sig, varname);
	}
}

void gc_code_property_variant_ret(genCode_s* gc, const char* variantname, const char* sig, const char* varname){
	if( *sig == 'a' || *sig == '{' || *sig == 'v' || *sig == '(' ){
		ostr_printf(&gc->c.tmpbody,	"\t*%s = %s.var.v;", varname, variantname);
	}
	else{
		ostr_printf(&gc->c.tmpbody,	"\t*%s = %s.var.%c;\n", varname, variantname, *sig);
	}
}

void gc_code_read(genCode_s* gc, const char* signature){
	ostr_printf(&gc->c.tmpreply, "\tif( sdbus_message_read(msg, \"%s\"", signature);
}

void gc_code_read_add_arg(genCode_s* gc, const char* arg){
	ostr_printf(&gc->c.tmpreply, ",\n\t\t%s", arg);
}

void gc_code_read_close(genCode_s* gc){
	ostr_puts(&gc->c.tmpreply, ")\n\t){\n\t\tsdbus_message_free(msg);\n\t\treturn NULL;\n\t}\n");
}

void gc_code_fn_end(genCode_s* gc){	
	ostr_printf(&gc->c.tmpbody, "%s\n\treturn msg;\n};\n\n", gc->c.tmpreply);
	ostr_puts(&gc->c.cfn, gc->c.tmpprepare);
	ostr_puts(&gc->c.cfn, gc->c.tmpprop);
	ostr_puts(&gc->c.cfn, gc->c.tmpbody);
	ostr_clear(gc->c.tmpprepare);
	ostr_clear(gc->c.tmpprop);
	ostr_clear(gc->c.tmpbody);
	ostr_clear(gc->c.tmpreply);
}

void gc_code_property_end(genCode_s* gc, int getset){
	ostr_printf(&gc->c.tmpbody, "%s\n\treturn %s;\n};\n\n", gc->c.tmpreply, getset ? "msg" : "0");
	ostr_puts(&gc->c.cfn, gc->c.tmpprepare);
	ostr_puts(&gc->c.cfn, gc->c.tmpprop);
	ostr_puts(&gc->c.cfn, gc->c.tmpbody);
	ostr_clear(gc->c.tmpprepare);
	ostr_clear(gc->c.tmpprop);
	ostr_clear(gc->c.tmpbody);
	ostr_clear(gc->c.tmpreply);
}


