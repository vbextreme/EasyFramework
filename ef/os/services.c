#include <ef/os.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/err.h>
#include <ef/xml.h>
#include <ef/str.h>

#include <stdarg.h>
#include <systemd/sd-bus.h>

typedef struct sdbus{
	sd_bus* bus;
}sdbus_s;

sdbus_s* sdbus_new(void){
	sdbus_s* sd = mem_new(sdbus_s);
	if( !sd ) err_fail("malloc");
	sd->bus = NULL;
	
	int err;
	if( (err=sd_bus_default_system(&sd->bus)) < 0 ){
		err_push("sdbus(%d): %s", err, strerror(-err));
		free(sd);
		return NULL;
	}

	return sd;
}

void sdbus_free(sdbus_s* sd){
	sd_bus_unref(sd->bus);
	free(sd);
}

sdbusBusNames_s* sdbus_bus_names(sdbus_s* sd){
	char** acq = NULL;
	char** act = NULL;

	int err = sd_bus_list_names(sd->bus, &acq, &act);
	if( err < 0 ){
		err_push("sdbus(%d): %s", err, strerror(-err));
		return NULL;
	}
	
	sdbusBusNames_s* bn = mem_new(sdbusBusNames_s);
	if( !bn ) err_fail("malloc");
	
	bn->acquired = vector_new(char*, 6, free);
	bn->activable = vector_new(char*, 6, free);

	for( size_t i = 0; acq[i]; ++i){
		vector_push_back(bn->acquired, acq[i]);
	}
	for( size_t i = 0; act[i]; ++i){
		vector_push_back(bn->activable, act[i]);
	}
	free(acq);
	free(act);
	
	return bn;
}

void sdbus_bus_names_free(sdbusBusNames_s* bn){
	vector_free(bn->acquired);
	vector_free(bn->activable);
	free(bn);
}

__private const char* endofsignature(const char* sig);

__private const char* endofsignature_p(char op, char cl, const char* sig){
	int b = 0;
	iassert(*sig == op);
	do{
		if( *sig == op ) ++b;
		else if ( *sig == cl ) --b;
		++sig;
	}while( *sig && b );
	if( b ) err_fail("wrong signature");
	return sig;
}

__private const char* endofsignature_a(const char* sig){
	iassert(*sig == 'a');
	while( *sig == 'a' ) ++sig;
	if( !*sig ) err_fail("wrong signature, terminate with array");
	return endofsignature(sig);
}

__private const char* endofsignature(const char* sig){
	switch( *sig ){
		case '(': return endofsignature_p('(', ')', sig);
		case '{': return endofsignature_p('{', '}', sig);
		case 'a': return endofsignature_a(sig);
		default: return sig+1;
	}
	err_fail("signature");
	return NULL;
}

__private const char* sdbus_signature_bs(char* out, const char* type){
	int b = 1;
	iassert(*type == '(');
	++type;
	do{
		if( *type == '(' ) ++b;
		else if ( *type == ')' ) --b;
		*out++ = *type++;
	}while( *type && b );
	if( b ){
		err_push("type wrong ()");
		return NULL;
	}
	out[-1] = 0;
	return type-1;
}

__private const char* sdbus_signature_kv(const char* type){
	int b = 1;
	iassert(*type == '{');
	++type;
	do{
		if( *type == '{' ) ++b;
		else if ( *type == '}' ) --b;
		++type;
	}while( *type && b );
	if( b ){
		err_push("type wrong {}");
		return NULL;
	}
	return type-1;
}

__private err_t sdbus_method_append_v(sd_bus_message* msg, sdbusVariant_s* v);
__private err_t sdbus_method_append_kv(sd_bus_message* msg, sdbusKV_s* kv);
__private err_t sdbus_method_append_array(sd_bus_message* msg, const char** type, void* vec);

__private err_t sdbus_method_append_array_struct_element(sd_bus_message* msg, const char** fromtype, void** vobj){
	__mem_free char* sig = mem_many(char, strlen(*fromtype));
	const char* nextsig = sdbus_signature_bs(sig, *fromtype);
   	if( !nextsig ) return -1;
	*fromtype = nextsig;

	int ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT_BEGIN, sig);
	if( ret ){
		err_push("open container");
		return ret;
	}

	const char* st = sig;
	while( *st ){
		switch( *st ){
			case 'y': case 'b': 
			case 'n': case 'q':
			case 'h': case 'i': case 'u': 
			case 'x': case 't': 
			case 'd':
			case 's': case 'o': case 'g':{
				if( !vector_count(vobj) ){
					err_push("no object in vector");
					return -1;
				}
				void* el = vector_pull_back(vobj);
				ret = sd_bus_message_append_basic(msg, *st, el); 
			}
			break;

			case '(':
				ret = sdbus_method_append_array_struct_element(msg, &st, vobj);
			break;
		
			case '{':{
				sdbusKV_s* el = vector_pull_back(vobj);
				ret = sdbus_method_append_kv(msg, el);
				st = sdbus_signature_kv(st);
				if( !st ) return -1;
			}
			break;

			case 'v':{
				sdbusVariant_s* el = vector_pull_back(vobj);
				ret = sdbus_method_append_v(msg, el);
			}break;

			case 'a':{
				void** el = vector_pull_back(vobj);
				ret = sdbus_method_append_array(msg, &st, el);
			}
		}
		if( ret < 0 ){
			err_push("append on struct %s", st);
			return ret;
		}
		if( *st ) ++st;
	}

	return sd_bus_message_close_container(msg);
}

__private err_t sdbus_method_append_array(sd_bus_message* msg, const char** type, void* vec){
	++(*type);
	const char* sig = *type;
	int ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_ARRAY, sig+1);
	int endarray=0;
	if( ret ){
		err_push("open container");
		return ret;
	}

	while( !endarray && **type ){
		switch( **type ){
			case 'y':{
				uint8_t* vy = vec;
				vector_foreach(vy, i){
					ret = sd_bus_message_append_basic(msg, 'y', &vy[i]);
					if( ret < 0 ){
						err_push("append array basic byte");
						return ret;
					}
				}
				endarray = 1;
			}
			break;
			case 'h': case 'i': case 'b':{
				int* vi = vec;
				vector_foreach(vi, i){
					ret = sd_bus_message_append_basic(msg, **type, &vi[i]); 
					if( ret < 0 ){
						err_push("append array basic int");
						return ret;
					}
				}
				endarray = 1;
			}
			break;
			case 'u':{
				uint32_t* vi = vec;
				vector_foreach(vi, i){
					ret = sd_bus_message_append_basic(msg, 'u', &vi[i]); 
					if( ret < 0 ){
						err_push("append array basic unsigned");
						return ret;
					}
				}
				endarray = 1;
			}
			break;
			case 'n':{
				int16_t* vi = vec;
				vector_foreach(vi, i){
					ret = sd_bus_message_append_basic(msg, 'n', &vi[i]); 
					if( ret < 0 ){
						err_push("append array basic sint");
						return ret;
					}
				}
				endarray = 1;
			}
			break;
			case 'q':{
				uint16_t* vi = vec;
				vector_foreach(vi, i){
					ret = sd_bus_message_append_basic(msg, 'q', &vi[i]); 
					if( ret < 0 ){
						err_push("append array basic suint");
						return ret;
					}
				}
				endarray = 1;
			}
			break;
			case 'x':{
				int64_t* vi = vec;
				vector_foreach(vi, i){
					ret = sd_bus_message_append_basic(msg, 'x', &vi[i]); 
					if( ret < 0 ){
						err_push("append array basic long");
						return ret;
					}
				}
				endarray = 1;
			} 
			break;
			case 't':{
				uint64_t* vi = vec;
				vector_foreach(vi, i){
					ret = sd_bus_message_append_basic(msg, 't', &vi[i]); 
					if( ret < 0 ){
						err_push("append array basic ulong");
						return ret;
					}
				}
				endarray = 1;
			}
			break;
			case 'd':{
				double* vi = vec;
				vector_foreach(vi, i){
					ret = sd_bus_message_append_basic(msg, 'd', &vi[i]); 
					if( ret < 0 ){
						err_push("append array basic double");
						return ret;
					}
				}
				endarray = 1;
			}
			break;
			case 's': case 'o': case 'g':{
				char** strs = vec;
				vector_foreach(strs, i){
					ret = sd_bus_message_append_basic(msg, **type, strs[i]); 
					if( ret < 0 ){
						err_push("append array basic string");
						return ret;
					}
				}
				endarray = 1;
			}
			break;

			case 'v':{
				sdbusVariant_s* va = vec;
				vector_foreach(va, i){
					ret = sdbus_method_append_v(msg, &va[i]); 
					if( ret < 0 ){
						err_push("append array variant");
						return ret;
					}
				}
				endarray = 1;
			}
			break;

			case '{':{
				sdbusKV_s* vkv = vec;
				vector_foreach(vkv, i){
					ret = sdbus_method_append_kv(msg, &vkv[i]); 
					if( ret < 0 ){
						err_push("append array kv");
						return ret;
					}
				}
				*type = sdbus_signature_kv(*type);
				if( !*type ) return -1;
				endarray = 1;
			}
			break;

			case '(':{
				void** vs = vec;
				const char* mt = *type;
				vector_foreach(vs, i){
					mt = *type;
					ret = sdbus_method_append_array_struct_element(msg, &mt, vs[i]);
					if( ret < 0 ){
						err_push("append array struct");
						return ret;
					}
				}
				*type = mt;
				if( !*type ) return -1;
				endarray = 1;
			}
			break;
	
			case 'a':{
				void** vs = vec;
				const char* mt = *type;
				vector_foreach(vs, i){
					mt = *type;
					ret = sdbus_method_append_array(msg, &mt, vs[i]);
					if( ret < 0 ){
						err_push("append array struct");
						return ret;
					}
				}
				*type = mt;
				endarray = 1;
			}
			break;
		}
		if( *type ) ++(*type);
	}

	return sd_bus_message_close_container(msg);
}

__private err_t sdbus_method_variant_append(sd_bus_message* msg, sdbusVariant_s* v){
	int ret = 0;

	const char* type = v->type;
	while( *type ){
		switch( *type ){
			case 'y': ret = sd_bus_message_append_basic(msg, 'y', &v->var.y); break;
			case 'n': ret = sd_bus_message_append_basic(msg, 'n', &v->var.n); break;
			case 'q': ret = sd_bus_message_append_basic(msg, 'q', &v->var.q); break;
			case 'b': ret = sd_bus_message_append_basic(msg, 'b', &v->var.b); break;
			case 'h': ret = sd_bus_message_append_basic(msg, 'h', &v->var.h); break;
			case 'i': ret = sd_bus_message_append_basic(msg, 'i', &v->var.i); break;
			case 'u': ret = sd_bus_message_append_basic(msg, 'u', &v->var.u); break;
			case 'x': ret = sd_bus_message_append_basic(msg, 'x', &v->var.x); break;
			case 't': ret = sd_bus_message_append_basic(msg, 't', &v->var.t); break;
			case 'd': ret = sd_bus_message_append_basic(msg, 'd', &v->var.d); break;
			case 's': ret = sd_bus_message_append_basic(msg, 's', v->var.s);  break;
			case 'o': ret = sd_bus_message_append_basic(msg, 'o', v->var.o);  break;
			case 'g': ret = sd_bus_message_append_basic(msg, 'g', v->var.g);  break;
			
			case '(': ret = sdbus_method_append_array_struct_element(msg, &type, v->var.v); break;
			
			case 'a': ret = sdbus_method_append_array(msg, &type, v->var.v); break;

			case '{':
				ret = sdbus_method_append_kv(msg, v->var.v);
				type = sdbus_signature_kv(type);
			break;
		
			case 'v':{
				sdbusVariant_s* sv = v->var.v;
				ret = sdbus_method_append_v(msg, sv);
			}
			break;
				
			default:
				err_push("unknow variant %c", *type);
				return -1;
			break;
		}
		if( ret < 0 ){
			err_push("append variant %s", type);
			return ret;
		}
		if( *type ) ++type;
	}
	return 0;
}

__private err_t sdbus_method_append_kv(sd_bus_message* msg, sdbusKV_s* kv){
	__mem_free char* sig = str_printf("%s%s", kv->key.type, kv->value.type);
		
	int ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_DICT_ENTRY, sig);
	if( ret ){
		err_push("open container");
		return ret;
	}

	ret = sdbus_method_variant_append(msg, &kv->key);
	if( ret < 0 ) return ret;
	sdbus_method_variant_append(msg, &kv->value);
	if( ret < 0 ) return ret;

	return sd_bus_message_close_container(msg);
}

__private err_t sdbus_method_append_v(sd_bus_message* msg, sdbusVariant_s* v){
	int ret = sd_bus_message_open_container(msg, 'v', v->type);
	if( ret < 0 ){
		err_push("open container");
		return ret;
	}

	sdbus_method_variant_append(msg, v);
	if( ret < 0 ) return ret;

	return sd_bus_message_close_container(msg);
}

sdbusMessage_h sdbus_methodv(sdbus_s* sd, const char* service, const char* object, const char* interface, const char *member, const char *types, va_list ap){
	sd_bus_message* reply = NULL;
	sd_bus_message* msg   = NULL;
	sd_bus_error serr = SD_BUS_ERROR_NULL;

	//dbg_info("call %s %s %s %s", service, object, interface, member);
	int ret = sd_bus_message_new_method_call(sd->bus, &msg, service, object, interface, member);
    if( ret < 0 ){
		err_push("message new method");
		goto fail;
	}

	if( types && *types ){
		while( *types ){
			switch( *types ){
				case 'y':{
					uint8_t y = va_arg(ap, int);
					ret = sd_bus_message_append_basic(msg, 'y', &y);
				}
				break;
				case 'n':{
					int16_t n = va_arg(ap, int);
					ret = sd_bus_message_append_basic(msg, 'n', &n);
				}
				break;
				case 'q':{
					uint16_t q = va_arg(ap, unsigned);
					ret = sd_bus_message_append_basic(msg, 'n', &q);
				}
				break;
				case 'b': case 'h':	case 'i':{
					int i = va_arg(ap, int);
					ret = sd_bus_message_append_basic(msg, *types, &i);
				}
				break;
				case 'u':{
					uint32_t u = va_arg(ap, unsigned);
					ret = sd_bus_message_append_basic(msg, 'u', &u);
				}
				break;
				case 'x':{
					int64_t x = va_arg(ap, long);
					ret = sd_bus_message_append_basic(msg, 'x', &x);
				}
				break;
				case 't':{
					uint64_t t = va_arg(ap, unsigned long);
					ret = sd_bus_message_append_basic(msg, 't', &t);
				}
				break;
				case 'd':{
					double d = va_arg(ap, double);
					ret = sd_bus_message_append_basic(msg, 'd', &d);
				}
				break;
				case 's': case 'o':	case 'g':{
					char* s = va_arg(ap, char*);
					ret = sd_bus_message_append_basic(msg, *types, s);
				}
				break;
				case 'v':{
					sdbusVariant_s* v = va_arg(ap, sdbusVariant_s*);
					ret = sdbus_method_append_v(msg, v);
				}
				break;
				case '(':{
					__mem_free char* sig = mem_many(char, strlen(types));
					if( !(types=sdbus_signature_bs(sig, types)) ) goto fail;
					ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT_BEGIN, sig);
				}
				break;
				case ')':
					ret = sd_bus_message_close_container(msg);
				break;
				case '{':{
					sdbusKV_s* kv = va_arg(ap, sdbusKV_s*);
					ret = sdbus_method_append_kv(msg, kv);
					types = sdbus_signature_kv(types);
				}
				break;
				case 'a':{
					void* vec = va_arg(ap, void*);
					ret = sdbus_method_append_array(msg, &types, vec);
				}
			}//switch types
			if( ret < 0 ) goto fail;
			if( *types ) ++types;
		}//while types
	}//if types
	
	if( (ret = sd_bus_call(sd->bus, msg, 0, &serr, &reply)) < 0 ){
		err_push("call method");
		goto fail;
	}
	sd_bus_message_unref(msg);

	return reply;

fail:
	ret = sd_bus_error_set_errno(&serr, ret);
	err_push("sdbus(%d):'%s'", ret, strerror(-ret));
	err_push("sdbus.err.name: %s", serr.name);
	err_push("sdbus.err.message: %s", serr.message);
	sd_bus_error_free(&serr);
	if( msg ) sd_bus_message_unref(msg);
	if( reply ) sd_bus_message_unref(reply);
	return NULL;
}

sdbusMessage_h sdbus_method(sdbus_s* sd, const char* service, const char* object, const char* interface, const char* member, const char* types, ...){
	va_list ap;
	sdbusMessage_h reply;
	va_start(ap, types);
	reply = sdbus_methodv(sd, service, object, interface, member, types, ap);
    va_end(ap);	
	return reply;
}

err_t sdbus_property_set(sdbus_s* sd, const char* service, const char* object, const char* interface, const char *member, sdbusVariant_s* v){
	sdbusMessage_h msg = sdbus_method(sd, service, object, "org.freedesktop.DBus.Properties", "Set", "ssv", interface, member, v);
	if( msg ){
		sdbus_message_free(msg);
		return 0;
	}
	return -1;
}

sdbusMessage_h sdbus_property_get(sdbus_s* sd, const char* service, const char* object, const char* interface, const char *member, sdbusVariant_s* v){
	sdbusMessage_h msg = sdbus_method(sd, service, object, "org.freedesktop.DBus.Properties", "Get", "ss", interface, member, v);
	if( !msg ) return NULL;
	if( sdbus_message_read_variant(msg, v) ){
		sdbus_message_free(msg);
		return NULL;
	}
	return msg;
}

err_t sdbus_on_signal(sdbus_s* sd, const char* serviceSender, const char* object, const char* interface, const char* member, sdbusCallback_f fn, void* userdata){
	if( sd_bus_match_signal(sd->bus, NULL, serviceSender, object, interface, member, (sd_bus_message_handler_t)fn, userdata) < 0 ){
		err_push("set signal when match %s->%s::%s->%s", serviceSender, object, interface, member);
		return -1;
	}
	return 0;
}

void sdbus_event_parse(sdbus_s* sd, int async){
	int ret;
	while( (ret=sd_bus_process(sd->bus, NULL)) > 0 );
	if( !async && !ret ){
		sd_bus_wait(sd->bus, UINT64_MAX);
		while( (ret=sd_bus_process(sd->bus, NULL)) > 0 );
	}
}

int sdbus_fd_get(sdbus_s* sd){
	return sd_bus_get_fd(sd->bus);
}

int sdbus_fd_eventfd(sdbus_s* sd){
	//POLLIN POLLPRI?
	return sd_bus_get_events(sd->bus);
}

uint64_t sdbus_fd_get_timeout(sdbus_s* sd){
	uint64_t to;
	sd_bus_get_timeout(sd->bus, &to);
	return to;
}
	
void sdbus_message_free(sdbusMessage_h msg){
	sd_bus_message_unref(msg);
}

void sdbus_message_autofree(void* m){
	void** msg = (void**)m;
	if( *msg ) sdbus_message_free(*msg);
}

/**********************************/
/**********************************/
/**********************************/
/********** READ MESSAGE **********/
/**********************************/
/**********************************/
/**********************************/

void sdbus_read_free_variant_content(sdbusVariant_s* v){
	switch( *v->type ){
		case 'y': case 'n': case 'q': case 'b': case 'h': case 'i':
		case 'u': case 'x': case 't': case 'd':	case 's': case 'o': case 'g':
		break;
			
		case 'a':
			sdbus_read_free_array(v->type, v->var.v);
		break;

		case 'v':
			sdbus_read_free_variant_content(v->var.v);
			free(v->var.v);
		break;

		case '{':
			sdbus_read_free_kv_content(v->var.v);
			free(v->var.v);
		break;

		case '(':
			sdbus_read_free_vstruct(v->type, v->var.v);
		break;

		default: err_fail("wrong type");
	}
}

void sdbus_read_free_kv_content(sdbusKV_s* kv){
	sdbus_read_free_variant_content(&kv->key);
	sdbus_read_free_variant_content(&kv->value);
}

void sdbus_read_free_vstruct(const char* type, void* v){
	void** vec = v;

	vector_foreach(vec, i){
		switch( *type ){
			case 'y': case 'n': case 'q': case 'b': case 'h': case 'i':
			case 'u': case 'x': case 't': case 'd':	case 's': case 'o': case 'g':
				free(vec[i]);
			break;
			
			case 'a':
				sdbus_read_free_array(type, vec[i]);
			break;

			case 'v':
				sdbus_read_free_variant_content(vec[i]);
				free(vec[i]);
			break;

			case '{':
				sdbus_read_free_kv_content(vec[i]);
				free(vec[i]);
			break;

			case '(': case ')': break;

			default: err_fail("wrong type");
		}
		if( *type != '(' )
		   	type = endofsignature(type);
		else
			++type;
	}
	vector_free(vec);
}

void sdbus_read_free_array(const char* type, void* v){
	void** vec = v;
	++type;

	switch( *type ){
		case 'y': case 'n': case 'q': case 'b': case 'h': case 'i':
		case 'u': case 'x': case 't': case 'd':	case 's': case 'o': case 'g':
			vector_free(vec);
		break;
			
		case 'a': 
			vector_foreach(vec, i){
				sdbus_read_free_array(type,vec[i]);
			}
		break;

		case 'v':
			vector_foreach(vec, i){
				sdbus_read_free_variant_content(vec[i]);
			}
		break;

		case '{':
			vector_foreach(vec, i){
				sdbus_read_free_kv_content(vec[i]);
			}
		break;

		case '(':
			vector_foreach(vec, i){
				sdbus_read_free_vstruct(type,vec[i]);
			}
		break;

		default: err_fail("invalid type");
	}

	vector_free(vec);
}

__private err_t sdbus_message_read_vstructure(sdbusMessage_h msg, const char* type, void* v){
	sd_bus_error serr = SD_BUS_ERROR_NULL;
	err_t err = 0;
	const char* orgtype = type;
	void** vec = vector_new(void*, strlen(type), NULL);
	void* var = NULL;

	if( (err=sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT, type)) < 0) goto fail;
	
	++type;
	while( *type ){
		int basic = 1;
		switch( *type ){
			case 'y': var = mem_new(uint8_t); break;
			case 'n': var = mem_new(int16_t); break;
			case 'q': var = mem_new(uint16_t); break;
			case 'b': case 'h': case 'i': var = mem_new(int); break;
			case 'u': var = mem_new(uint32_t); break;
			case 'x': var = mem_new(int64_t); break;
			case 't': var = mem_new(uint64_t); break;
			case 'd': var = mem_new(double); break;
			case 's': case 'o': case 'g': var = mem_new(char*); break;
			
			case 'a': 
				basic = 0;
				if( sdbus_message_read_array(msg, type, &var) ) goto fail;
			break;

			case 'v':
				basic = 0;
				if( sdbus_message_read_variant(msg, var) ) goto fail;
			break;

			case '{':
				basic = 0;
				if( sdbus_message_read_kv(msg, type, var) ) goto fail;
			break;
			
			case '(':
				if( (err=sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT, type)) < 0) goto fail;
			break;

			case ')':
				if( (err=sd_bus_message_close_container(msg)) < 0) goto fail;
			break;
	
			default: err_fail("wrong type");
		}

		vector_push_back(vec, var);
		if( basic && sdbus_message_read_basic(msg, *type, var) ) goto fail;
		type = endofsignature(type);
	}	

	void** out = v;
	*out = vec;

	return 0;
fail:
	if( err ){
		err = sd_bus_error_set_errno(&serr, err);
		err_push("sdbus(%d):'%s'", err, strerror(-err));
		err_push("sdbus.err.name: %s", serr.name);
		err_push("sdbus.err.message: %s", serr.message);
		sd_bus_error_free(&serr);
	}
	else{
		err_push("wrong struct %s", type);
	}
	sdbus_read_free_vstruct(orgtype, vec);
	return -1;
}

__private err_t sdbus_message_read_unit(sdbusMessage_h msg, const char* type, sdbusVariant_s* v){
	v->type = (char*)type;
	
	switch( *type ){
		case 'y': return sdbus_message_read_basic(msg, *type, &v->var.y);
		case 'n': return sdbus_message_read_basic(msg, *type, &v->var.n);
		case 'q': return sdbus_message_read_basic(msg, *type, &v->var.q);
		case 'b': return sdbus_message_read_basic(msg, *type, &v->var.b);
		case 'h': return sdbus_message_read_basic(msg, *type, &v->var.h);
		case 'i': return sdbus_message_read_basic(msg, *type, &v->var.i);
		case 'u': return sdbus_message_read_basic(msg, *type, &v->var.u);
		case 'x': return sdbus_message_read_basic(msg, *type, &v->var.x);
		case 't': return sdbus_message_read_basic(msg, *type, &v->var.t);
		case 'd': return sdbus_message_read_basic(msg, *type, &v->var.d);
		case 's': return sdbus_message_read_basic(msg, *type, &v->var.s);
		case 'o': return sdbus_message_read_basic(msg, *type, &v->var.o);
		case 'g': return sdbus_message_read_basic(msg, *type, &v->var.g);
		case 'a': return sdbus_message_read_array(msg, type, &v->var.v);
		case '(': return sdbus_message_read_vstructure(msg, type, &v->var.v);

		case 'v':{
			sdbusVariant_s* variant = mem_new(sdbusVariant_s);
			if( !variant ) err_fail("eom");
			if( sdbus_message_read_variant(msg, variant) ){
				free(variant);
				v->var.v = NULL;
				return -1;
			}
			v->var.v = variant;
			return 0;
		}

			
		case '{':{
			sdbusKV_s* kv = mem_new(sdbusKV_s);
			if( !kv ) err_fail("eom");
			if( sdbus_message_read_kv(msg, type, kv) ){
				free(kv);
				return -1;
			}
			v->var.v = kv;
			return 0;
		}
		break;
		
				
		default: err_fail("wrong type");
	}
	return 0;
}

err_t sdbus_message_read_kv(sdbusMessage_h msg, const char* type, sdbusKV_s* v){
	err_t ret = sdbus_message_read_unit(msg, type+1, &v->key);
	if( ret < 0 ) return ret;
	type = endofsignature(type+1);
	return sdbus_message_read_unit(msg, type, &v->value);
}

err_t sdbus_message_read_variant(sdbusMessage_h msg, sdbusVariant_s* v){
	char* sig = NULL;
	if( sdbus_message_read_basic(msg, 'g', &sig) ) return -1;
	return sdbus_message_read_unit(msg, sig, v);
}

err_t sdbus_message_read_basic(sdbusMessage_h msg, const char type, void* ret){
	sd_bus_error serr = SD_BUS_ERROR_NULL;
	if( type == 'v' ) return sdbus_message_read_variant(msg, ret);
	err_t err = sd_bus_message_read_basic(msg, type, ret);
	if( err < 0 ){
		err_push("read basic %c", type);
		err = sd_bus_error_set_errno(&serr, err);
		err_push("sdbus(%d):'%s'", err, strerror(-err));
		err_push("sdbus.err.name: %s", serr.name);
		err_push("sdbus.err.message: %s", serr.message);
		sd_bus_error_free(&serr);
		return -1;
	}
	return 0;
}

#define MSG_READ_ARRAY_BASIC(ERR, MSG, TYPE, SDTYPE, COUNT, VVECTOR) do{\
	TYPE** vin = VVECTOR;\
	*vin = vector_new(TYPE, COUNT, NULL);\
	for( size_t i = 0; i < COUNT; ++i ){\
		TYPE rd;\
		if( (ERR=sd_bus_message_read_basic(msg, SDTYPE, &rd)) < 0 ){\
			vector_free(*vin);\
			goto fail;\
		}\
		vector_push_back(*vin, rd);\
	}\
}while(0)

err_t sdbus_message_read_array(sdbusMessage_h msg, const char* type, void* vec){
	sd_bus_error serr = SD_BUS_ERROR_NULL;
	err_t err;
	size_t count = 0;

	if( *type != 'a' ) err_fail("wrong type");
	++type;
	
	if( (err=sd_bus_message_open_container(msg, SD_BUS_TYPE_ARRAY, type)) < 0) goto fail;
	if( (err=sd_bus_message_read_basic(msg, 'i', &count)) < 0 ) goto fail;
	
	switch( *type ){
		case 'y': 
			MSG_READ_ARRAY_BASIC(err, msg, uint8_t, *type, count, vec);
		break;
		case 'h': case 'i': case 'b':
			MSG_READ_ARRAY_BASIC(err, msg, int, *type, count, vec);
		break;
		case 'u':
			MSG_READ_ARRAY_BASIC(err, msg, uint32_t, *type, count, vec);
		break;
		case 'n':
			MSG_READ_ARRAY_BASIC(err, msg, int16_t, *type, count, vec);
		break;
		case 'q':
			MSG_READ_ARRAY_BASIC(err, msg, uint16_t, *type, count, vec);
		break;
		case 'x':
			MSG_READ_ARRAY_BASIC(err, msg, int64_t, *type, count, vec);
		break;
		case 't':
			MSG_READ_ARRAY_BASIC(err, msg, uint64_t, *type, count, vec);
		break;
		case 'd':
			MSG_READ_ARRAY_BASIC(err, msg, double, *type, count, vec);
		break;
		case 's': case 'o': case 'g':
			MSG_READ_ARRAY_BASIC(err, msg, char*, *type, count, vec);
		break;

		case 'v':{
			char* sig = NULL;
			sdbusVariant_s** vvar = vec;
			if( (err=sd_bus_message_read_basic(msg, 'g', &sig)) ) goto fail;
			*vvar = vector_new(sdbusVariant_s, count, NULL);
			for( size_t i = 0; i < count; ++i ){
				sdbusVariant_s rd;
				if( (err=sdbus_message_read_unit(msg, sig, &rd)) < 0 ){
					vector_foreach(*vvar, j){
						sdbus_read_free_variant_content(&(*vvar)[i]);
					}
					vector_free(*vvar);
					goto fail;
				}
				vector_push_back(*vvar, rd);
			}
		}
		break;

		case '{':{
			sdbusKV_s** vvar = vec;
			*vvar = vector_new(sdbusKV_s, count, NULL);
			for( size_t i = 0; i < count; ++i ){
				sdbusKV_s rd;
				if( (err=sdbus_message_read_kv(msg, type, &rd)) < 0 ){
					vector_foreach(*vvar, j){
						sdbus_read_free_kv_content(&(*vvar)[i]);
					}
					vector_free(*vvar);
				}
				vector_push_back(*vvar, rd);
			}
		}
		break;

		case '(':{
			void*** vvar = vec;
			*vvar = vector_new(void*, count, NULL);
			for( size_t i = 0; i < count; ++i ){
				void* rd;
				if( (err=sdbus_message_read_vstructure(msg, type, &rd)) < 0 ){
					sdbus_read_free_vstruct(type, vvar);
					goto fail;
				}
				vector_push_back(*vvar, rd);
			}
		}
		break;
	
		case 'a':{
			void*** vin = vec;
			*vin = vector_new(void*, count, NULL);
			for( size_t i = 0; i < count; ++i ){
				void* el = NULL;
				if( sdbus_message_read_array(msg, type, &el) ){
					sdbus_read_free_array(type, vin);
					return -1;
				}
				vector_push_back(*vin, el);
			}
		}
		break;	 

		default: err_fail("wrong type");

	}
	
	if( (err=sd_bus_message_close_container(msg)) ) goto fail;

	return 0;
fail:
	err_push("read array %s", type);
	err = sd_bus_error_set_errno(&serr, err);
	err_push("sdbus(%d):'%s'", err, strerror(-err));
	err_push("sdbus.err.name: %s", serr.name);
	err_push("sdbus.err.message: %s", serr.message);
	sd_bus_error_free(&serr);
	return -1;
}

err_t sdbus_message_readv(sdbusMessage_h msg, const char* type, va_list ap){
	sd_bus_error serr = SD_BUS_ERROR_NULL;
	err_t ret = 0;
	while( *type ){
		switch( *type ){
			case 'y': case 'n': case 'q': case 'b': case 'h': case 'i': case 'u':
			case 'x': case 't':	case 'd': case 's': case 'o': case 'g': case 'v':{
				void* v = va_arg(ap, void*);
				if( sdbus_message_read_basic(msg, *type, v) ) return -1;
				type = endofsignature(type);
			}
			break;
			
			case 'a':{
				void* v = va_arg(ap, void*);
				if( sdbus_message_read_array(msg, type, v) ) return -1;
				type = endofsignature(type);
			}
			break;

			case '{':{
				void*v = va_arg(ap, void*);
				if( sdbus_message_read_kv(msg, type, v) ) return -1;
				type = endofsignature(type);
			}
			break;

			case '(':{
				const char* eot = endofsignature(type);
				__mem_free char* sig = str_dup(type, eot-type);
				if( (ret=sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT_BEGIN, sig)) < 0 ) goto fail;
				++type;
			}
			break;
			case ')':
				if( (ret=sd_bus_message_close_container(msg)) < 0 ) goto fail;
				++type;
			break;

			default: err_fail("wrong signature");
		}//switch types
	}//while types
	
	return 0;

fail:
	ret = sd_bus_error_set_errno(&serr, ret);
	err_push("sdbus(%d):'%s'", ret, strerror(-ret));
	err_push("sdbus.err.name: %s", serr.name);
	err_push("sdbus.err.message: %s", serr.message);
	sd_bus_error_free(&serr);
	return -1;
}

err_t sdbus_message_read(sdbusMessage_h msg, const char* types, ...){
	va_list ap;
	va_start(ap, types);
	err_t ret = sdbus_message_readv(msg, types, ap);
    va_end(ap);	
	return ret;
}

/********************************/
/********************************/
/********************************/
/********** INTROSPECT **********/
/********************************/
/********************************/
/********************************/

__private err_t parse_property(sdbusIIProperty_s* prp, xml_s* xml){
	prp->access = -1;
	prp->name = NULL;
	prp->type = NULL;

	do{
		char* name = (char*)xml_attr_name(xml);
		if( !strcmp(name, "type") ){
			prp->type = (char*)xml_attr_value(xml);
		}
		else if ( !strcmp(name, "name") ){
			prp->name = (char*)xml_attr_value(xml);
		}
		else if( !strcmp(name, "access") ){
			if( !strcmp((char*)xml_attr_value(xml), "read") ){
				prp->access = SDBUS_PROPERTY_ACCESS_READ;
			}
			else if( !strcmp((char*)xml_attr_value(xml), "write") ){
				prp->access = SDBUS_PROPERTY_ACCESS_WRITE;
			}
			else{
				prp->access = SDBUS_PROPERTY_ACCESS_READ | SDBUS_PROPERTY_ACCESS_WRITE;
			}
		}
	}while( !xml_attr_next(xml) );

	if( prp->access == -1 || prp->name == NULL || prp->type == NULL ){
		err_push("wrong property attributes");
		return -1;
	}

	return 0;
}

__private err_t parse_args(sdbusIIMArg_s* arg, xml_s* xml){
	arg->in = -1;
	arg->name = NULL;
	arg->type = NULL;

	do{
		char* name = (char*)xml_attr_name(xml);
		if( !strcmp(name, "type") ){
			arg->type = (char*)xml_attr_value(xml);
		}
		else if ( !strcmp(name, "name") ){
			arg->name = (char*)xml_attr_value(xml);
		}
		else if( !strcmp(name, "direction") ){
			if( !strcmp((char*)xml_attr_value(xml), "in") ){
				arg->in = 1;
			}
			else{
				arg->in = 0;
			}
		}
	}while( !xml_attr_next(xml) );

	if( arg->name == NULL || arg->type == NULL ){
		err_push("wrong argument attributes");
		return -1;
	}

	return 0;
}

__private err_t parse_method(sdbusIIProto_s* method, xml_s* xml){
	if( !strcmp((char*)xml_attr_name(xml), "name") ){
		method->name = (char*)xml_attr_value(xml);
	}
	else{
		method->name = NULL;
	}
	method->vargs = vector_new(sdbusIIMArg_s, 2, NULL);

	if( xml_node_child(xml) ){
		return 0;
	}

	do{
		if( !strcmp((char*)xml_node_name(xml), "arg") ){
			if( parse_args(vector_get_push_back(method->vargs), xml) ){
				dbg_error("arg");
				return -1;
			}
		}	
	}while( !xml_node_next(xml) );
	
	if( xml_node_parent(xml) ){
		err_push("cant return to parent");
		return -1;
	}
	return 0;
}

__private err_t parse_interface(sdbusIInterface_s* in, xml_s* xml){
	if( !strcmp((char*)xml_attr_name(xml), "name") ){
		in->name = (char*)xml_attr_value(xml);
	}
	else{
		in->name = NULL;
	}
	in->vmethods  = vector_new(sdbusIIProto_s, 2, NULL);
	in->vsignal   = vector_new(sdbusIIProto_s, 2, NULL);
	in->vproperty = vector_new(sdbusIIProperty_s, 2, NULL);

	if( xml_node_child(xml) ){
		err_push("need child");
		return -1;
	}

	do{
		char* name = (char*)xml_node_name(xml);
		if( !strcmp(name, "method") ){
			if( parse_method(vector_get_push_back(in->vmethods), xml) ){
				err_push("wrong method");
				return -1;
			}
		}
		else if( !strcmp(name, "signal") ){
			if( parse_method(vector_get_push_back(in->vsignal), xml) ){
				err_push("wrong signal");
				return -1;
			}
		}
		else if( !strcmp(name, "property") ){
			if( parse_property(vector_get_push_back(in->vproperty), xml) ){
				err_push("wrong property");
				return -1;
			}
		}
	}while( !xml_node_next(xml) );

	if( xml_node_parent(xml) ){
		dbg_error("parent");
		err_push("cant return to parent");
		return -1;
	}
	return 0;
}

__private sdbusIntrospect_s* parse_introspect(sdbus_s* sd, const char* service, const char* object, char* str){
	xml_s* xml = xml_parse(str);
	if( !xml ){
		free(str);
		return NULL;
	}
	sdbusIntrospect_s* i = mem_new(sdbusIntrospect_s);
	if( !i ) err_fail("eom");
	i->vchild = vector_new(sdbusIntrospect_s*, 6, NULL);
	i->vinterfaces = vector_new(sdbusIInterface_s, 6, NULL);
	i->str = str;
	i->xml = xml;

	if( strcmp((char*)xml_node_name(xml), "node") || xml_node_child(xml) ){
		sdbus_introspect_free(i);
		err_push("introspect root name != <node> || !child");
		return NULL;
	}

	do{
		char* name = (char*)xml_node_name(xml);
		if( !strcmp(name, "interface") ){
			if( parse_interface(vector_get_push_back(i->vinterfaces), xml) ){
				sdbus_introspect_free(i);
				err_push("wrong interface");
				return NULL;
			}
		}
		else if( !strcmp(name, "node") ){
			if( !strcmp((char*)xml_attr_name(xml), "name") ){
				__mem_free char* icname = str_printf("%s/%s", object, (char*)xml_attr_value(xml));
				sdbusIntrospect_s* icin = sdbus_introspect(sd, service, icname);
				if( !icin ){
					sdbus_introspect_free(i);
					err_push("wrong child element: %s", icname);
					return NULL;
				}
				vector_push_back(i->vchild, icin);
			}
		}
	}while( !xml_node_next(xml) );

	return i;
}

void sdbus_introspect_free(sdbusIntrospect_s* i){
	free(i->str);
	
	xml_free(i->xml);
	
	vector_foreach(i->vchild, j){
		sdbus_introspect_free(i->vchild[j]);
	}
	vector_free(i->vchild);

	vector_foreach(i->vinterfaces, j){
		vector_foreach(i->vinterfaces[j].vmethods, k){
			vector_free(i->vinterfaces[j].vmethods[k].vargs);
		}
		vector_foreach(i->vinterfaces[j].vsignal, k){
			vector_free(i->vinterfaces[j].vsignal[k].vargs);
		}
		vector_free(i->vinterfaces[j].vmethods);
		vector_free(i->vinterfaces[j].vsignal);
		vector_free(i->vinterfaces[j].vproperty);
	}
	vector_free(i->vinterfaces);
	
	free(i);
}

char* sdbus_introspect_raw(sdbus_s* sd, const char* service, const char* object){
	__sdbus_message_free sdbusMessage_h msg = sdbus_method(sd, service, object, "org.freedesktop.DBus.Introspectable", "Introspect", NULL);
	if(	!msg ) return NULL;	
	char* str = NULL;
	if( sdbus_message_read(msg, "s", &str) ) return NULL;
	return str_dup(str, 0);
}

sdbusIntrospect_s* sdbus_introspect(sdbus_s* sd, const char* service, const char* object){
	char* str = sdbus_introspect_raw(sd, service, object);
	if( !str ){
		err_push("no dbus responce");
		return NULL;
	}
	return parse_introspect(sd, service, object,str);
}


