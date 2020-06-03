#ifndef __GENCODE_H__
#define __GENCODE_H__

#include <ef/type.h>
#include <ef/err.h>
#include <ef/str.h>
#include <ef/ostr.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/file.h>
#include <ef/xml.h>
#include <ef/os.h>

#define DEFAULT_STRING_SIZE 128

typedef struct gcHeader{
	char* name;
	char* hinclude;
	char* hstruct;
	char* hproto;
	char* tmpdocp;
	char* tmpprop;
}gcHeader_s;

typedef struct gcC{
	char* cinclude;
	char* cfn;
	char* tmpprop;
	char* tmpbody;
	char* tmpprepare;
	char* tmpreply;
}gcC_s;

typedef struct genCode{
	gcHeader_s h;
	gcC_s c;
}genCode_s;

void gc_init(genCode_s* gc);
void gc_delete(genCode_s* gc);
void gc_header_include_guard_begin(genCode_s* gc, const char* headername);
void gc_header_include_guard_end(genCode_s* gc);
void gc_header_include(genCode_s* gc, const char* header);
err_t gc_header_declare_struct(genCode_s* gc, const char* name);
void gc_header_declare_struct_element(genCode_s* gc, const char* type, const char* name, const char* doc);
void gc_header_declare_struct_end(genCode_s* gc, const char* name);
void gc_header_declare_proto(genCode_s* gc, const char* name);
void gc_header_declare_proto_get(genCode_s* gc, const char* name);
void gc_header_declare_proto_set(genCode_s* gc, const char* name);
void gc_header_declare_proto_return(genCode_s* gc, const char* type, const char* doc);
void gc_header_declare_proto_arg(genCode_s* gc, const char* type, const char* name, const char* doc);
void gc_header_declare_proto_end(genCode_s* gc);
err_t gc_header_proto_separator(genCode_s* gc, const char* str);

void gc_code_include(genCode_s* gc, const char* header);
void gc_code_fn_separator(genCode_s* gc, const char* str);
void gc_code_declare_fn(genCode_s* gc, const char* name);
void gc_code_declare_fn_get(genCode_s* gc, const char* name);
void gc_code_declare_fn_set(genCode_s* gc, const char* name);
void gc_code_declare_fn_return(genCode_s* gc, const char* type);
void gc_code_declare_fn_arg(genCode_s* gc, const char* type, const char* name);
void gc_code_declare_fn_end(genCode_s* gc);
void gc_code_fn_method_prepare_struct(genCode_s* gc, const char* name, const char* sname, char** vename);
void gc_code_fn_method_begin(genCode_s* gc, const char* service, const char* object, const char* interface, const char* method);
void gc_code_fn_call_method_add_arg(genCode_s* gc, const char* arg);
void gc_code_fn_method_close(genCode_s* gc, size_t nargs);
void gc_code_fn_method_test(genCode_s* gc);
void gc_code_property_set(genCode_s* gc, const char* service, const char* object, const char* interface, const char* method, const char* variantname);
void gc_code_property_get(genCode_s* gc, const char* service, const char* object, const char* interface, const char* method, const char* variantname);
void gc_code_property_variant(genCode_s* gc, const char* variantname, const char* sig);
void gc_code_property_variant_set(genCode_s* gc, const char* variantname, const char* sig, const char* varname);
void gc_code_property_variant_ret(genCode_s* gc, const char* variantname, const char* sig, const char* varname);
void gc_code_read(genCode_s* gc, const char* signature);
void gc_code_read_add_arg(genCode_s* gc, const char* arg);
void gc_code_read_close(genCode_s* gc);
void gc_code_fn_end(genCode_s* gc);
void gc_code_property_end(genCode_s* gc, int getset);

void services_list(int acquired, int activable);
void service_introspect(const char* services, const char* object);
void service_make(const char* service, const char* object, int onlyThisInterface, FILE* fdh, FILE* fdc, const char* header, int verbose);
	
#endif
