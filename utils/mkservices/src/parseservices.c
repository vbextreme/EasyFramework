#include "gencode.h"

__private char* mtype[256] = {
	['y'] = "uint8_t",
	['b'] = "int",
	['n'] = "int16_t",
	['q'] = "uint16_t",
	['i'] = "int",
	['u'] = "uint32_t",
	['x'] = "int64_t",
	['t'] = "uint64_t",
	['d'] = "double",
	['h'] = "int",
	['o'] = "char*",
	['g'] = "char*",
	['s'] = "char*",
	['v'] = "sdbusVariant_s",
	['{'] = "sdbusKV_s"
};

void services_list(int acquired, int activable){
	sdbus_s* sd = sdbus_new();
	if( !sd ) err_fail("sdbus");
	
	sdbusBusNames_s* bn = sdbus_bus_names(sd);
	if( acquired ){
		vector_foreach(bn->acquired,i){
			printf("acquired:'%s'\n", bn->acquired[i]);
		}
	}
	if( activable ){
		vector_foreach(bn->activable, i){
			printf("activable:'%s'\n", bn->activable[i]);
		}
	}
	sdbus_bus_names_free(bn);
	
	sdbus_free(sd);
	return;
}

void service_introspect(const char* services, const char* object){
	sdbus_s* sd = sdbus_new();
	if( !sd ) err_fail("sdbus");
	
	char* xml = sdbus_introspect_raw(sd, services, object);
	if( !xml ) err_fail("introspect %s %s", services, object);
	printf("%s", xml);
	
	free(xml);
	sdbus_free(sd);
	return;
}

__private void gc_introspect_interface_method_arg(genCode_s* gc, const char* namespace, sdbusIIMArg_s* arg){
	static char* inout[] = {"out_", "in_"};

	const char* type = arg->type;
	__ostr_free char* typename = ostr_new(DEFAULT_STRING_SIZE);
	__ostr_free char* varname = ostr_new(DEFAULT_STRING_SIZE);
	__ostr_free	char* repvar = ostr_new(DEFAULT_STRING_SIZE);

	__vector_free char** vstypeelement = vector_new(char*, DEFAULT_STRING_SIZE, 0);
	__vector_free char** vsnameelement = vector_new(char*, DEFAULT_STRING_SIZE, 0);

	int isstruct = 0;
	int isarray = 0;
	
	if( *type == 'a' ){
		while( *type == 'a' ){
			++isarray;
			++type;
		}
	}

	ostr_printf(&varname, "%s%s", inout[arg->in], arg->name);

	unsigned index = *type;
	if( mtype[index] ){
		ostr_puts(&typename, mtype[index]);
	}
	else{
		if( *type != '(' ) err_fail("unknow type %c in arg type %s", *type, arg->type);
		isstruct = 1;
		ostr_printf(&typename, "%s_%s", namespace, arg->name);
		err_t redeclared = gc_header_declare_struct(gc, typename);
		++type;
		int subarray = 0;
		size_t counter = 0;
		while( *type ){
			index = *type;
			if( *type == 'a' ){
				++subarray;
			}
			else if( mtype[index] ){
				char* eltype = ostr_new(DEFAULT_STRING_SIZE);
				ostr_puts(&eltype, mtype[index]);
				for( int i = 0; i < subarray; ++i ) ostr_putch(&eltype, '*');
				vector_push_back(vstypeelement, eltype);

				char* elname = ostr_new(DEFAULT_STRING_SIZE);
				ostr_printf(&elname, "%s%s_%c%lu", subarray ? "v" : "", arg->name, *type == '{' ? 'k' : *type, counter++);
				vector_push_back(vsnameelement, elname);

				if( !redeclared ) gc_header_declare_struct_element(gc, eltype, elname, arg->type);

				if( *type == '{' ){
					int bal = 0;
					do{
						if( *type == '}' ) --bal;
						else if( *type == '{' ) ++bal;
						++type;
					}while( bal && *type );
					if( bal ) err_fail("{} not balanced");
					if( !*type ) --type;
				}
				subarray = 0;
			}
			else if( *type != '(' && *type != ')' ){
				err_fail("unknow type %c in arg type %s", *type, arg->type);
			}
			++type;
		}
		if( !redeclared ) gc_header_declare_struct_end(gc, typename);
	}

	if( isstruct ) ostr_puts(&typename, "_s*");
	for( int i = 0; i < isarray; ++i ) ostr_putch(&typename, '*');
	if( isarray ){
		ostr_insch(&varname, 0, 'v');
		if( isstruct ){
			ostr_printf(&repvar, "_auto_%s", varname);
			gc_code_fn_method_prepare_struct(gc, repvar, varname, vsnameelement);
		}
	}
	else if( *type == 'v' || *type == '{' ){
		ostr_insch(&typename, 0, '*');
	}
	if( !arg->in ) ostr_putch(&typename, '*');
	
	gc_header_declare_proto_arg(gc, typename, repvar[0] ? repvar : varname, arg->type);
	gc_code_declare_fn_arg(gc, typename, repvar[0] ? repvar : varname);

	if( !arg->in ){
		gc_code_read(gc, arg->type);
	}

	if( !isarray && isstruct ){
		if( arg->in ){
			vector_foreach(vsnameelement, i){
				__mem_free char* a = str_printf("%s->%s", varname, vsnameelement[i]);
				gc_code_fn_call_method_add_arg(gc, a);
			}
		}
		else{
			vector_foreach(vsnameelement, i){
				__mem_free char* a = str_printf("%s->%s", varname, vsnameelement[i]);
				gc_code_read_add_arg(gc, a);
			}
		}
	}
	else{
		if( arg->in ){
			gc_code_fn_call_method_add_arg(gc, varname);	
		}
		else{
			gc_code_read_add_arg(gc, varname);
		}
	}

	if( !arg->in ){
		gc_code_read_close(gc);
	}

	vector_foreach(vsnameelement, i){
		ostr_free(vsnameelement[i]);
	}
	vector_foreach(vstypeelement, i){
		ostr_free(vstypeelement[i]);
	}
}

__private void gc_introspect_interface_method_proto(genCode_s* gc, const char* namespace, const char* service, const char* object, const char* interface, sdbusIIProto_s* pro){
	__mem_free char* fnname = str_printf("%s_%s", namespace, pro->name);
	gc_header_declare_proto(gc, fnname);
	gc_header_declare_proto_return(gc, "sdbusMessage_h", "msg or NULL for error, need to sdbus_message_free");
	gc_header_declare_proto_arg(gc, "sdbus_s*", "sd", "sd-bus object");
	gc_code_declare_fn(gc, fnname);
	gc_code_declare_fn_arg(gc, "sdbus_s*", "sd");
	gc_code_declare_fn_return(gc, "sdbusMessage_h");
	gc_code_fn_method_begin(gc, service, object, interface, pro->name);

	int argfn = 0;
	if( vector_count(pro->vargs) ){
		__ostr_free char* fulltype = ostr_new(DEFAULT_STRING_SIZE);
		ostr_putch(&fulltype, '"');
		vector_foreach(pro->vargs, i ){
			if( !pro->vargs[i].in ) continue;
			ostr_puts(&fulltype, pro->vargs[i].type);
		}
		if( fulltype[1] ){
			argfn = 1;
			ostr_putch(&fulltype, '"');	
			gc_code_fn_call_method_add_arg(gc, fulltype);
		}
	}

	vector_foreach(pro->vargs, i ){
		gc_introspect_interface_method_arg(gc, namespace, &pro->vargs[i]);
	}

	gc_code_fn_method_close(gc, argfn);
	gc_code_fn_method_test(gc);


	gc_code_declare_fn_end(gc);
	gc_code_fn_end(gc);
	gc_header_declare_proto_end(gc);
}

__private void property_parse_type(genCode_s* gc, char** typevar, const char* namespace, const char* argname, char* type){
	int isarray = 0;
	int isstruct = 0;
	
	if( *type == 'a' ){
		while( *type == 'a' ){
			++isarray;
			++type;
		}
	}

	unsigned index = *type;
	if( mtype[index] ){
		ostr_puts(typevar, mtype[index]);
	}
	else{
		if( *type != '(' ) err_fail("unknow type %c in arg type %s", *type, type);
		isstruct = 1;
		ostr_printf(typevar, "%s_%s", namespace, argname);
		if( !gc_header_declare_struct(gc, *typevar) ){
			++type;
			int subarray = 0;
			size_t counter = 0;
			while( *type ){
				index = *type;
				if( *type == 'a' ){
					++subarray;
				}
				else if( mtype[index] ){
					__ostr_free char* eltype = ostr_new(DEFAULT_STRING_SIZE);
					ostr_puts(&eltype, mtype[index]);
					for( int i = 0; i < subarray; ++i ) ostr_putch(&eltype, '*');

					__ostr_free char* elname = ostr_new(DEFAULT_STRING_SIZE);
					ostr_printf(&elname, "%s%s_%c%lu", subarray ? "v" : "", argname, *type == '{' ? 'k' : *type, counter++);
					gc_header_declare_struct_element(gc, eltype, elname, type);

					if( *type == '{' ){
						int bal = 0;
						do{
							if( *type == '}' ) --bal;
							else if( *type == '{' ) ++bal;
							++type;
						}while( bal && *type );
						if( bal ) err_fail("{} not balanced");
						if( !*type ) --type;
					}
					subarray = 0;
				}
				else if( *type != '(' && *type != ')' ){
					err_fail("unknow type %c in arg type %s", *type, type);
				}
				++type;
			}
			gc_header_declare_struct_end(gc, *typevar);
		}
	}

	if( isstruct ) ostr_puts(typevar, "_s*");
	for( int i = 0; i < isarray; ++i ) ostr_putch(typevar, '*');
}

__private void gc_introspect_interface_property(genCode_s* gc, const char* namespace, const char* service, const char* object, const char* interface, sdbusIIProperty_s* pro){
	__mem_free char* fnname = str_printf("%s_%s", namespace, pro->name);
	if( pro->access & SDBUS_PROPERTY_ACCESS_READ ){
		gc_header_declare_proto_get(gc, fnname);
		gc_header_declare_proto_return(gc, "sdbusMessage_h", "msg or NULL for error, need to sdbus_message_free");
		gc_header_declare_proto_arg(gc, "sdbus_s*", "sd", "sd-bus object");
		gc_code_declare_fn_get(gc, fnname);
		gc_code_declare_fn_return(gc, "sdbusMessage_h");
		gc_code_declare_fn_arg(gc, "sdbus_s*", "sd");
		__ostr_free char* typevar = ostr_new(DEFAULT_STRING_SIZE);
		property_parse_type(gc, &typevar, namespace, pro->name, pro->type);
		ostr_putch(&typevar, '*');
		gc_header_declare_proto_arg(gc, typevar, pro->name, pro->type);
		gc_code_declare_fn_arg(gc, typevar, pro->name);
		gc_code_declare_fn_end(gc);

		__mem_free const char* tmpvariant = str_printf("tmp%s", pro->name);
		gc_code_property_variant(gc, tmpvariant, pro->type);
		gc_code_property_get(gc, service, object, interface, pro->name, tmpvariant);
		gc_code_property_variant_ret(gc, tmpvariant, pro->type, pro->name);

		gc_header_declare_proto_end(gc);
		gc_code_property_end(gc, 1);
	}

	if( pro->access & SDBUS_PROPERTY_ACCESS_WRITE ){
		gc_header_declare_proto_set(gc, fnname);
		gc_header_declare_proto_return(gc, "err_t", "0 successfull -1 error");
		gc_header_declare_proto_arg(gc, "sdbus_s*", "sd", "sd-bus object");
		gc_code_declare_fn_set(gc, fnname);
		gc_code_declare_fn_return(gc, "err_t");
		gc_code_declare_fn_arg(gc, "sdbus_s*", "sd");

		__ostr_free char* typevar = ostr_new(DEFAULT_STRING_SIZE);
		property_parse_type(gc, &typevar, namespace, pro->name, pro->type);
		gc_header_declare_proto_arg(gc, typevar, pro->name, pro->type);
		gc_code_declare_fn_arg(gc, typevar, pro->name);
		gc_code_declare_fn_end(gc);
	
		__mem_free const char* tmpvariant = str_printf("tmp%s", pro->name);
		gc_code_property_variant(gc, tmpvariant, pro->type);
		gc_code_property_variant_set(gc, tmpvariant, pro->type, pro->name);
		gc_code_property_set(gc, service, object, interface, pro->name, tmpvariant);

		gc_header_declare_proto_end(gc);
		gc_code_property_end(gc, 0);
	}

}

__private char* gc_introspect_interface_namespace(const char* name){
	if( name == NULL ) name = "NULL_NAMESPACE";
	char* ns = str_dup(name, 0);
	char* rp = ns;
	while( (rp=strchr(rp,'.')) ) *rp = '_';
	return ns;
}

__private void gc_introspect_interface(genCode_s* gc, const char* service, const char* object, sdbusIInterface_s* ii, int verbose){
	__mem_free char* namespace = gc_introspect_interface_namespace(ii->name);
	if( gc_header_proto_separator(gc, namespace) ) return;
	gc_code_fn_separator(gc, namespace);

	vector_foreach(ii->vmethods, i){
		if( verbose ){
			printf("create methods: %s_%s\n", namespace, ii->vmethods[i].name);
		}
		gc_introspect_interface_method_proto(gc, namespace, service, object, ii->name, &ii->vmethods[i]);
	}
	
	vector_foreach(ii->vproperty, i){
		if( verbose ){
			printf("create property: %s_%s\n", namespace, ii->vproperty[i].name);
		}
		gc_introspect_interface_property(gc, namespace, service, object, ii->name, &ii->vproperty[i]);
	}

	/* nothing for now
	vector_foreach(ii->vsignal, i){
		if( verbose ){
			printf("create signal: %s(sdbus* sd", ii->vsignal[i].name);
			vector_foreach(ii->vsignal[i].vargs, j){
				printf(",%s[%s] %s", ii->vsignal[i].vargs[j].in ? "in" : "out", ii->vsignal[i].vargs[j].type, ii->vsignal[i].vargs[j].name);
			}
			printf(")\n");
		}
	}
	*/
}

__private void gc_introspect(genCode_s* gc, sdbusIntrospect_s* introspect, const char* service, const char* object, int onlyThisInterface, int verbose){
	vector_foreach(introspect->vinterfaces, i){
		if( onlyThisInterface && str_ancmp(introspect->vinterfaces[i].name, service) ) continue;
		if( verbose ) printf("interface:%s\n", introspect->vinterfaces[i].name);
		gc_introspect_interface(gc, service, object, &introspect->vinterfaces[i], verbose);

	}
	vector_foreach(introspect->vchild, i){
		gc_introspect(gc, introspect->vchild[i], service, object, onlyThisInterface, verbose);
	}
}

void service_make(const char* service, const char* object, int onlyThisInterface, FILE* fdh, FILE* fdc, const char* header, int verbose){
	sdbus_s* sd = sdbus_new();
	if( !sd ) err_fail("sdbus");
	
	sdbusIntrospect_s* introspect = sdbus_introspect(sd, service, object);
	if( !introspect ) err_fail("introspect %s %s", service, object);

	genCode_s gc;
	gc_init(&gc);

	gc_header_include_guard_begin(&gc, header);
	gc_header_include(&gc, "ef/type.h");
	gc_header_include(&gc, "ef/vector.h");
	gc_header_include(&gc, "ef/memory.h");
	gc_header_include(&gc, "ef/str.h");
	gc_header_include(&gc, "ef/os.h");
	gc_code_include(&gc, header);
	gc_introspect(&gc, introspect, service, object, onlyThisInterface, verbose);
	gc_header_include_guard_end(&gc);

	if( fdh ){
		fputs(gc.h.hinclude, fdh);
		fputs("\n", fdh);
		fputs(gc.h.hstruct, fdh);
		fputs("\n", fdh);
		fputs(gc.h.hproto, fdh);
	}
	if( fdc ){
		fputs(gc.c.cinclude, fdc);
		fputs("\n", fdc);
		fputs(gc.c.cfn, fdc);
	}

	sdbus_introspect_free(introspect);
	sdbus_free(sd);
	gc_delete(&gc);
}


