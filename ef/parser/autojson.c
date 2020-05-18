#include <ef/json.h>
#include <ef/err.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>

/*

struct kkk{
	char* name;
	char* val;
};

struct obj{
	int a;
	char* name
	int** vint;
	struct kkk sub;
};

{
	"a":1234,
	"name:"hello,
	"sub":{
		"name": "cao",
		"val": "value"
	}
}

*/

jsonDef_s* json_parse_new_object(size_t size){
	dbg_info("json start with object: %lu", size);
	jsonDef_s* def = mem_new(jsonDef_s);
	if( !def ) err_fail("malloc");

	def->name = NULL;
	def->type = JSON_DEF_OBJECT;
	def->offof = 0;
	def->size = size;
	def->vintrospect = vector_new(jsonDef_s, 6, NULL);
	def->parent = NULL;

	return def;	
}

jsonDef_s* json_parse_new_vector(void){
	dbg_info("json start with vector");
	jsonDef_s* def = mem_new(jsonDef_s);
	if( !def ) err_fail("malloc");

	def->name = NULL;
	def->type = JSON_DEF_VECTOR;
	def->offof = 0;
	def->size = 0;
	def->vintrospect = vector_new(jsonDef_s, 1, NULL);
	def->parent = NULL;

	return def;	
}

jsonDef_s* json_parse_declare_number(jsonDef_s* def, const char* name, size_t offof, size_t size, jsonDef_e typeNum){
	iassert(def);
	dbg_info("json declare number:: %d %s->%lu: %lu", typeNum, name, offof, size);

	jsonDef_s* el = vector_get_push_back(def->vintrospect);
	el->name = name;
	el->offof = offof;
	el->size = size;
	el->type = typeNum;
	el->vintrospect = NULL;
	el->parent = def;

	return el;
}

jsonDef_s* json_parse_declare_int(jsonDef_s* def, const char* name, size_t offof){
	return json_parse_declare_number(def, name, offof, sizeof(int*), JSON_DEF_INT);
}

jsonDef_s* json_parse_declare_uint(jsonDef_s* def, const char* name, size_t offof){
	return json_parse_declare_number(def, name, offof, sizeof(unsigned int*), JSON_DEF_UINT);
}

jsonDef_s* json_parse_declare_long(jsonDef_s* def, const char* name, size_t offof){
	return json_parse_declare_number(def, name, offof, sizeof(long*), JSON_DEF_LONG);
}

jsonDef_s* json_parse_declare_ulong(jsonDef_s* def, const char* name, size_t offof){
	return json_parse_declare_number(def, name, offof, sizeof(unsigned long*), JSON_DEF_ULONG);
}

jsonDef_s* json_parse_declare_double(jsonDef_s* def, const char* name, size_t offof){
	return json_parse_declare_number(def, name, offof, sizeof(double*), JSON_DEF_NUMBER);
}

jsonDef_s* json_parse_declare_string(jsonDef_s* def, const char* name, size_t offof){
	iassert(def);
	dbg_info("json declare string:: %d %s->%lu: %lu", JSON_DEF_STRING, name, offof, sizeof(char*));

	jsonDef_s* el = vector_get_push_back(def->vintrospect);
	el->name = name;
	el->offof = offof;
	el->size = 0;//sizeof(char**);
	el->type = JSON_DEF_STRING;
	el->vintrospect = NULL;
	el->parent = def;

	return el;
}

jsonDef_s* json_parse_declare_vector(jsonDef_s* def, const char* name, size_t offof){
	iassert(def);
	dbg_info("json declare vector:: %d %s->%lu: %d", JSON_DEF_VECTOR, name, offof, 1);

	jsonDef_s* el = vector_get_push_back(def->vintrospect);
	el->name = name;
	el->offof = offof;
	el->size = 0;
	el->type = JSON_DEF_VECTOR;
	el->vintrospect = vector_new(jsonDef_s, 1, NULL);
	el->parent = def;
	return el;
}

jsonDef_s* json_parse_declare_object(jsonDef_s* def, const char* name, size_t offof, size_t size){
	iassert(def);

	jsonDef_s* el = vector_get_push_back(def->vintrospect);
	el->name = name;
	el->offof = offof;
	el->size = size;
	el->type = JSON_DEF_OBJECT;
	el->vintrospect = vector_new(jsonDef_s, 6, NULL);
	el->parent = def;

	return el;
}

__private void def_copy(jsonDef_s* dst, jsonDef_s* src){
	dst->name = src->name;
	dst->offof = src->offof;
	dst->size = src->size;
	dst->type = src->type;
	dst->parent = src->parent;
	if( src->vintrospect ){
		dst->vintrospect = vector_new(jsonDef_s, vector_count(src->vintrospect), NULL);
		vector_foreach(src->vintrospect, i){
			jsonDef_s* el = vector_get_push_back(dst->vintrospect);
			def_copy(el, &src->vintrospect[i]);
		}
	}
	else{
		dst->vintrospect = NULL;
	}
}

jsonDef_s* json_parse_declare_def(jsonDef_s* def, jsonDef_s* sub){
	iassert(def);
	iassert(sub);

	jsonDef_s* el = vector_get_push_back(def->vintrospect);
	def_copy(el, sub);
	el->parent = def;
	
	return el;
}

__private void vdef_free(jsonDef_s* def){
	if( !def->vintrospect ){
		dbg_info("'%s' introspect not need free", def->name);
		return;
	}
	vector_foreach(def->vintrospect, i ){
		dbg_info("callback introspect[%lu].name:'%s'", i, def->vintrospect[i].name);
		vdef_free(&def->vintrospect[i]);
	}
	dbg_info("'%s' free introspect", def->name);
	vector_free(def->vintrospect);
}

void json_def_free(jsonDef_s* def){
	vdef_free(def);
	dbg_info("free '%s'", def->name);
	free(def);
}

#define usrval_stack(V,J) do{\
	V = J->usrval;\
	if( !stk || !vector_count(stk) ){\
	   	dbg_error("wrong stack");\
		return JP_ERR_STACK;\
	}\
}while(0)

typedef enum { 
	JP_ERR_DEF = JSON_ERR_USER + 1,
	JP_ERR_STACK,
	JP_ERR_SIZE,
	JP_ERR_TVEC,
	JP_ERR_TOBJ,
	JP_ERR_TNUM,
	JP_ERR_TDBL,
	JP_ERR_TSTR,
	JP_ERR_TBOL
}jperr_e;

#define GTYPE_MEM 0
#define GTYPE_VEC 1

typedef struct gtmem{
	void* adr;
	int type;
}gtmem_s;

typedef struct garbage{
	gtmem_s* mem;
}garbage_s;

__private void* gc_add_ref(void* jgc, int type, void* addr){
	garbage_s* gc = jgc;
	gtmem_s* m = vector_get_push_back(gc->mem);
	m->adr = addr;
	m->type = type;
	return addr;
}

__private void* gc_alloc(void* jgc, int type, size_t size){
	void* mem;
	switch( type ){
		case GTYPE_MEM:
			mem = malloc(size);
			if( !mem ) err_fail("malloc");
		break;

		case GTYPE_VEC:
			mem = vector_new_raw(sizeof(void*), 6, NULL);
		break;

		default:
			err_fail("wrong type");
	}
	return gc_add_ref(jgc, type, mem);
}

__private void gc_free(void* jgc, int clean){
	garbage_s* gc = jgc;

	if( clean ){
		vector_foreach(gc->mem, i){
			switch( gc->mem[i].type ){
				case GTYPE_MEM:
					free(gc->mem[i].adr);
				break;

				case GTYPE_VEC:
					vector_free(gc->mem[i].adr);
				break;

				default:
					err_fail("wrong type");
			}
		}
	}
	vector_free(gc->mem);
}

__private err_t jsa_vec_new(json_s* j){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def || def->type != JSON_DEF_VECTOR ){
		return def ? JP_ERR_DEF : JP_ERR_TVEC;
	}

	void** stk;
	usrval_stack(stk, j);   
	void** mem = vector_pull_back(stk);
	*mem = gc_alloc(j->usrgarbage, GTYPE_VEC, 0);
	vector_push_back(stk,*mem);
	
	return JSON_OK;
}

__private err_t jsa_vec_next(json_s* j){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def || def->type != JSON_DEF_VECTOR ){
		return def ? JP_ERR_DEF : JP_ERR_TVEC;
	}
	def = &def->vintrospect[0];
	j->usrctx = def;

	void** stk;
	usrval_stack(stk, j);
	void** v = vector_pull_back(stk);
	void** mem = vector_get_push_back(v);
	dbg_info("vref mem %p", mem);
	switch( def->type ){
		case JSON_DEF_OBJECT:
		case JSON_DEF_STRING:
			*mem = NULL;
		break;

		case JSON_DEF_VECTOR:
			*mem = gc_alloc(j->usrgarbage, GTYPE_VEC, 0);
		break;

		default:
			if( !def->size ){
				return JP_ERR_SIZE;
			}
			*mem = gc_alloc(j->usrgarbage, GTYPE_MEM, def->size);
			dbg_info("alloc ref:%p", *mem);
		break;
	}

	vector_push_back(stk,v);
	vector_push_back(stk,mem);
	
	return JSON_OK;
}

__private err_t jsa_vec_end(json_s* j){
	if( j->usrstat ) return JSON_OK;

	dbg_info("");
	jsonDef_s* def = j->usrctx;
	if( !def || def->type != JSON_DEF_VECTOR ){
		return def ? JP_ERR_DEF : JP_ERR_TVEC;
	}
	j->usrctx = def->parent;
	
	void** stk;
	usrval_stack(stk, j);
	--vector_count(stk);

	return JSON_OK;
}

__private err_t jsa_obj_new(json_s* j){
	if( j->usrstat ){
		++j->usrstat;
		return JSON_OK;
	}

	jsonDef_s* def = j->usrctx;
	if( !def || def->type != JSON_DEF_OBJECT ){
		return def ? JP_ERR_DEF : JP_ERR_TOBJ;
	}

	void** stk;
	usrval_stack(stk, j);
	void** mem = vector_pull_back(stk);
	void* alc = gc_alloc(j->usrgarbage, GTYPE_MEM, def->size);
	*mem = alc;
	memset(alc, 0, def->size);
	if( !*mem ) err_fail("malloc");
	dbg_info("new obj(%p)->size %lu  owner:%p", alc, def->size, mem);

	vector_push_back(stk, mem);

	return JSON_OK;
}

__private err_t jsa_obj_property(json_s* j, char** name, size_t len){
	if( j->usrstat > 1 ) return JSON_OK;
	j->usrstat = 0;

	jsonDef_s* def = j->usrctx;
	if( !def || def->type != JSON_DEF_OBJECT ){
		return def ? JP_ERR_DEF : JP_ERR_TOBJ;
	}

	void** stk;
	usrval_stack(stk, j);
	void** obj = vector_pull_back(stk);
	dbg_info("&obj: %p", *obj);

	vector_foreach(def->vintrospect, i){
		if( !str_equal(def->vintrospect[i].name, strlen(def->vintrospect[i].name), *name, len) ){
			void** objel = (void**)(ADDR(*obj) + def->vintrospect[i].offof);
			switch( def->vintrospect[i].type ){
				case JSON_DEF_STRING:
					*objel = NULL;
					dbg_info("find element is string:%s link addr: %p alloc: %p", def->vintrospect[i].name, ADDR(*obj) + def->vintrospect[i].offof, *objel);
				break;

				case JSON_DEF_VECTOR:
					*objel = NULL;
					dbg_info("find element is vector:%s link addr: %p alloc: %p", def->vintrospect[i].name, ADDR(*obj) + def->vintrospect[i].offof, *objel);
				break;

				default:
					if( !def->vintrospect[i].size ){
						return JP_ERR_SIZE;
					}
					*objel = gc_alloc(j->usrgarbage, GTYPE_MEM, def->vintrospect[i].size );
					dbg_info("find element:%s link addr: %p alloc: %p", def->vintrospect[i].name, ADDR(*obj) + def->vintrospect[i].offof, *objel);
				break;
			}
			vector_push_back(stk, obj);
			vector_push_back(stk, objel);
			dbg_info("context:%d -> %d", def->type, def->vintrospect[i].type);
			j->usrctx = &def->vintrospect[i];
			return JSON_OK;
		}
	}

	vector_push_back(stk, obj);
	j->usrstat = 1;
	dbg_info("start skipping");
	return JSON_OK;
}

__private err_t jsa_obj_end(json_s* j){
	if( j->usrstat ){
		--j->usrstat;
		return JSON_OK;
	}

	jsonDef_s* def = j->usrctx;
	if( !def || def->type != JSON_DEF_OBJECT ){
		return def ? JP_ERR_DEF : JP_ERR_TOBJ;
	}
	j->usrctx = def->parent;

	dbg_info("");
	void** stk;
	usrval_stack(stk, j);
	--vector_count(stk);

	return JSON_OK;
}

__private err_t jsa_value_int(json_s* j, const char* value, size_t len){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def ){
		return JP_ERR_DEF;
	}

	long val;
	if( json_long_validation(&val, value, len) ){
		return JSON_ERR_NUMBER;	
	}

	void** stk;
	usrval_stack(stk, j);
	void** mem = vector_pull_back(stk);
	dbg_info("get owner   mem %p",  mem);
	dbg_info("get address mem %p", *mem);
	dbg_info("set to val %ld", val);

	switch( def->type ){
		case JSON_DEF_INT:
			*((int*)*mem) = val;
		break;

		case JSON_DEF_UINT:
			*((unsigned int*)*mem) = val;
		break;

		case JSON_DEF_LONG:
			*((long*)*mem) = val;
		break;

		case JSON_DEF_ULONG:
			*((unsigned long*)*mem) = val;
		break;

		default: 
		return JP_ERR_TNUM;
	}

	j->usrctx = def->parent;

	return JSON_OK;
}

__private err_t jsa_value_float(json_s* j, const char* value, size_t len){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def ) return JP_ERR_DEF;

	double val;
	if( json_float_validation(&val, value, len) ) return JSON_ERR_NUMBER;	

	void** stk;
	usrval_stack(stk, j);
	void** mem = vector_pull_back(stk);

	switch( def->type ){
		case JSON_DEF_NUMBER:
			*((double*)*mem) = val;
		break;

		default: 
		return JP_ERR_TDBL;
	}

	j->usrctx = def->parent;

	return JSON_OK;
}

__private err_t jsa_value_string(json_s* j, char** value, size_t len){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def ) return JP_ERR_DEF;

	dbg_info("");
	char* str = json_unescape(j, *value, len);
	if( !str ) return JSON_ERR_STRING;
	gc_add_ref(j->usrgarbage, GTYPE_MEM, str);

	void** stk;
	usrval_stack(stk, j);
	void** mem = vector_pull_back(stk);
	dbg_info("get address &mem %p mem %p", mem, *mem);
	dbg_info("set to val '%s'", str);

	switch( def->type ){
		case JSON_DEF_STRING:
			*mem = str;
		break;

		default: 
		return JP_ERR_TSTR;
	}

	j->usrctx = def->parent;

	return JSON_OK;
}

__private err_t jsa_value_true(json_s* j){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def ) return JP_ERR_DEF;

	void** stk;
	usrval_stack(stk, j);
	void** mem = vector_pull_back(stk);

	switch( def->type ){
		case JSON_DEF_INT:
			*((int*)*mem) = 1;
		break;

		default: 
		return JP_ERR_TBOL;
	}

	j->usrctx = def->parent;

	return JSON_OK;
}

__private err_t jsa_value_false(json_s* j){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def ) return JP_ERR_DEF;

	void** stk;
	usrval_stack(stk, j);
	void** mem = vector_pull_back(stk);

	switch( def->type ){
		case JSON_DEF_INT:
			*((int*)*mem) = 0;
		break;

		default: 
		return JP_ERR_TBOL;
	}

	j->usrctx = def->parent;

	return JSON_OK;
}

__private err_t jsa_value_null(json_s* j){
	if( j->usrstat ) return JSON_OK;

	jsonDef_s* def = j->usrctx;
	if( !def ) return JP_ERR_DEF;
	
	void** stk;
	usrval_stack(stk, j);
	void** mem = vector_pull_back(stk);
	*mem = NULL;
	
	j->usrctx = def->parent;

	return JSON_OK;
}

void* json_parse(jsonDef_s* def, const char* data){
	void* ret = NULL;
	void** stk = vector_new(void*, 4, NULL);
	vector_push_back(stk, &ret);
	garbage_s gc;
	gc.mem = vector_new(gtmem_s, 6, NULL);

	char* errparser[] = {
		"no definition set, propably internal parser error",
		"internal stack error",
		"definition not have setted size",
		"definition is not vector",
		"definition is not object",
		"definition is not number",
		"definition is not double",
		"definition is not string",
		"definition is not boolean(int)",
	};

	json_s json = {
		.objectNew = jsa_obj_new,
		.objectEnd = jsa_obj_end,
		.objectProperties = jsa_obj_property,
		.arrayNew = jsa_vec_new,
		.arrayNext = jsa_vec_next,
		.arrayEnd = jsa_vec_end,
		.valueNull = jsa_value_null,
		.valueTrue = jsa_value_true,
		.valueFalse = jsa_value_false,
		.valueInteger = jsa_value_int,
		.valueFloat = jsa_value_float,
		.valueString = jsa_value_string,
		.usrctx = def,
		.usrstat = 0,
		.usrval = stk,
		.usrit = 0,
		.usrgarbage = &gc,
		.usrError = errparser
	};

	if( json_lexer(&json, data) ){
		json_push_error(&json);
		vector_free(stk);
		gc_free(&gc, 1);
		return NULL;
	}

	if( vector_count(stk) ){
		vector_free(stk);
		gc_free(&gc, 1);
		return NULL;
	}

	vector_free(stk);
	gc_free(&gc, 0);

	return ret;
}


