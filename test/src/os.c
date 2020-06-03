#include "test.h"
#include <ef/sys.h>
#include <ef/proc.h>
#include <ef/spawn.h>
#include <ef/os.h>

/*@test -o --os 'test os'*/

__private void printps(const char* name, size_t val){
	sip_e si;
	double v = mth_si_prefix_base(&si, val);
	printf("%s:%7.3f%s\n", name, v, mth_si_prefix_translate_short_string(si));
}

typedef struct iima{
	char* doc;
	char* type;
	char* name;
	sdbusIIMArg_s* arg;
}iima_s;

typedef struct iim{
	iima_s* va;
	char* doc;
	char* fname;
	sdbusIIProto_s* m;
}iim_s;

typedef struct iip{
	iima_s argget;
	iima_s argset;
	char* docget;
	char* fnameget;
	char* docset;
	char* fnameset;
	sdbusIIProperty_s* p;
}iip_s;

__private char* introspect_type(const char* type, int in){
	const char* errtype = type;
	char* mtype[256] = { 0 };
	mtype['y'] = "unsigned char";
	mtype['b'] = "int";
	mtype['n'] = "short int";
	mtype['q'] = "unsigned short int";
	mtype['i'] = "int";
	mtype['u'] = "unsigned int";
	mtype['x'] = "long";
	mtype['t'] = "unsigned long";
	mtype['d'] = "double";
	mtype['h'] = "int";
	mtype['o'] = mtype['g'] = mtype['s'] = "char*";
	mtype['v'] = "sdbusVariant_s";
	mtype['{'] = "sdbusKV_s";
	mtype['('] = "/*TODO*/ void*";

	char array[10];
	unsigned ia = 0;
	while( *type == 'a' ){
		++type;
		array[ia++] = '*';
	}
	array[ia] = 0;

	if( !mtype[(unsigned)*type] ) err_fail("unknow type '%c' in '%s'", *type, errtype);

	return str_printf("%s%s%s", mtype[(unsigned)*type], array, in ? "" : "*");
}

__private void gencode_introspect_interface_proto_arg(iima_s* ea, sdbusIIMArg_s* sa){
	ea->arg  = sa;
	ea->type = introspect_type(sa->type, sa->in);
	ea->name = str_printf("%s_%s", sa->in ? "in" : "out", sa->name) ;
	ea->doc  = str_printf(" * @param %s systemd type %s", ea->name, sa->type);
}

__private void gencode_introspect_interface_proto(const char* namespace, iim_s* em, sdbusIIProto_s* pro, const char* docPre, const char* codePre){
	em->m     = pro;
	em->fname = str_printf("%s%s_%s", codePre, namespace, pro->name);
	em->doc   = str_printf("/** %s%s", docPre, em->fname);
	em->va    = vector_new(iima_s, 4, NULL);
	vector_foreach( pro->vargs, i ){
		gencode_introspect_interface_proto_arg(vector_get_push_back(em->va), &pro->vargs[i]);
	}
}

__private void gencode_introspect_interface_property(const char* namespace, iip_s* ep, sdbusIIProperty_s* pro){
	ep->p        = pro;
	if( pro->access & SDBUS_PROPERTY_ACCESS_READ ){
		ep->argget.type = introspect_type(pro->type, 1);
		ep->argget.name = str_printf("%s", pro->name) ;
		ep->fnameget = str_printf("%s_%s_get", namespace, pro->name);
		ep->docget   = str_printf("/** %s", ep->fnameget);
		ep->argget.doc  = str_printf(" * @param %s systemd type %s", pro->name, pro->type);
	}
	else{
		ep->argget.type = NULL;
		ep->argget.doc  = NULL;
		ep->argget.name = NULL;
		ep->fnameget = NULL;
		ep->docset   = NULL;
	}
	if( pro->access & SDBUS_PROPERTY_ACCESS_WRITE ){
		ep->argset.type = introspect_type(pro->type, 1);
		ep->argset.name = str_printf("%s", pro->name) ;
		ep->fnameset = str_printf("%s_%s_set", namespace, pro->name);
		ep->docget   = str_printf("/** %s", ep->fnameset);
		ep->argset.doc  = str_printf(" * @return %s systemd type %s", pro->name, pro->type);
	}
	else{
		ep->argset.type = NULL;
		ep->argset.doc  = NULL;
		ep->argset.name = NULL;
		ep->fnameset = NULL;
		ep->docset   = NULL;
	}
}

__private char* gencode_introspect_interface_namespace(const char* name){
	if( name == NULL ) name = "NULL_NAMESPACE";
	char* ns = str_dup(name, 0);
	char* rp = ns;
	while( (rp=strchr(rp,'.')) ) *rp = '_';
	return ns;
}

__private void iim_free(iim_s* iim){
	free(iim->doc);
	free(iim->fname);
	vector_foreach(iim->va, i){
		free(iim->va[i].doc);
		free(iim->va[i].name);
		free(iim->va[i].type);
	}
	vector_free(iim->va);
}

__private void iip_free(iip_s* iip){
	if( iip->docget ) free(iip->docget);
	if( iip->docset ) free(iip->docset);
	if( iip->fnameget ) free(iip->fnameget);
	if( iip->fnameset ) free(iip->fnameset);
	if( iip->argget.doc ) free(iip->argget.doc);
	if( iip->argget.name ) free(iip->argget.name);
	if( iip->argget.type ) free(iip->argget.type);
	if( iip->argset.doc ) free(iip->argset.doc);
	if( iip->argset.name ) free(iip->argset.name);
	if( iip->argset.type ) free(iip->argset.type);
}

__private void gencode_arg_in_parse(const char* name, const char* type){
	if( *type == 'a' ){
		++type;
		printf(",\n\t\tvector_count(%s)", name);
	}
}

__private int basicType[256] = {
	['b'] = 1,
	['y'] = 1,
	['n'] = 1,
	['q'] = 1,
	['i'] = 1,
	['u'] = 1,
	['x'] = 1,
	['t'] = 1,
	['d'] = 1,
	['h'] = 1,
	['o'] = 1
};

__private int arg_is_basic(char type){
	return basicType[(unsigned)type];	
}

__private int arg_is_container(char* type){
	if( *type == '(' ) return 1;
	return 0;
}

__private int arg_is_array(char type){
	if( type == 'a' ) return 1;
	return 0;
}

__private int arg_is_variant(char type){
	if( type == 'v' ) return 1;
	return 0;
}

__private void gencode_test_error(const char* errstr){
	puts("\tif( ret < 0){");
	printf("\t\terr_puts(\"%s\");\n", errstr);
	puts("\t\tgoto onerr;");
	puts("\t}");
}

__private void gencode_arg_in(iima_s* va){
	vector_foreach(va, i){
		if( !va[i].arg->in ) continue;
		char* type = va[i].arg->type;
		while( *type ){
			if( arg_is_basic(*type) ){
				printf("ret = sd_bus_message_append_basic(msg, %c, %s);\n", *type, va[i].arg->name);
				gencode_test_error("append basic");
			}
			else if( arg_is_variant(*type) ){
				printf("ret = sd_bus_open_container(msg, 'v', %s.type");
			}
			else if( arg_is_array(*type) ){
				
			}
		}
	}
}

__private void gencode_method_fn(const char* services, const char* object, const char* interface, iim_s* m){
	puts("\tsdbusMessage_h msg = NULL;");
	puts("\tint ret = 0;");
	puts("\tchar* vtype = NULL;");
	printf("err_t %s(sdbus_h* sd", m->fname);
	vector_foreach(m->va, j){
		printf(", %s %s", m->va[j].type, m->va[j].name);
	}
	puts("){");
	printf("\tret = sd_bus_message_new_method_call(sd, \"%s\", \"%s\",\n\t\t\"%s\", \"%s\");\n", services, object, interface, m->m->name);
	gencode_test_error("on new method");
	
	
	
	
	
	if( vector_count(m->va) ){
		putchar('"');
		vector_foreach(m->va, i){
			if( !m->va[i].arg->in ) continue;
			fputs(m->va[i].arg->type, stdout);
		}
		fputs("\"", stdout);
		
		vector_foreach(m->va, i){
			if( !m->va[i].arg->in ) continue;
			char* t = m->va[i].arg->type;
			if( *t == 'a' ){
				printf(",\n\t\tvector_count(%s)", m->va[i].name);
				++t;
			}
			if( *t == '{' ){
				printf(",\n\t\t%s.", m->va[i].name);
				++t;
				while(*t == 'a'){
					putchar('a');
					++t;
				}
				putchar(*t);
				printf(",\n\t\t%s.", m->va[i].name);
				++t;
				while(*t == 'a'){
					putchar('a');
					++t;
				}
				putchar(*t);
			}
			else{
				printf(",\n\t\t%s", m->va[i].name);
			}
		}
		putchar('\n');
	}
	else{
		puts("NULL");
	}
	puts("\t);");
	
	puts("\tif( !msg ) return -1;\n");

	puts("\treturn 0;");
	puts("}\n");
}

__private void gencode_introspect_interface(const char* services, const char* object, sdbusIInterface_s* ii, int header){
	__mem_free char* namespace = gencode_introspect_interface_namespace(ii->name);
	printf("/");
	int l = strlen(namespace) + 8;
	for( int i = 0; i < l; ++i) putchar('*');
	printf("/\n");
	printf("/*** %s ***/\n", namespace);
	printf("/");
	for( int i = 0; i < l; ++i) putchar('*');
	printf("/\n\n");

	vector_foreach(ii->vmethods, i){
		iim_s m;
		gencode_introspect_interface_proto(namespace, &m, &ii->vmethods[i], "", "");
		if( header ){
			puts(m.doc);
			puts(" * @param sd sdbus object");
			vector_foreach(m.va, j){
				puts(m.va[j].doc);
			}
			puts("*/");
			printf("err_t %s(sdbus_s* sd", m.fname);
			vector_foreach(m.va, j){
				printf(", %s %s", m.va[j].type, m.va[j].name);
			}
			puts(");\n");
		}
		else{
			gencode_method_fn(services, object, ii->name, &m);
		}
		iim_free(&m);
	}

	vector_foreach(ii->vsignal, i){
		iim_s m;
		gencode_introspect_interface_proto(namespace, &m, &ii->vsignal[i], "", "signal_");
		if( header ){
			puts(m.doc);
			puts(" * @param sd sdbus object");
			vector_foreach(m.va, j){
				puts(m.va[j].doc);
			}
			puts("*/");
			printf("err_t %s(sdbus_s* sd", m.fname);
			vector_foreach(m.va, j){
				printf(", %s %s", m.va[j].type, m.va[j].name);
			}
			puts(");\n");
		}
		else{

		}
		iim_free(&m);
	}
	vector_foreach(ii->vproperty, i){
		iip_s p;
		gencode_introspect_interface_property(namespace, &p, &ii->vproperty[i]);
		if( header ){
			if( p.docget ){
				puts(p.docget);
				puts(" * @param sd sdbus object");
				puts(p.argget.doc);
				puts("*/");
				printf("err_t %s(sdbus_s* sd, %s %s);\n\n", p.fnameget, p.argget.type, p.argget.name);
			}
			if( p.docset ){
				puts(p.docset);
				puts(" * @param sd sdbus object");
				puts(p.argset.doc);
				puts("*/");
				printf("%s %s(sdbus_s* sd);\n\n", p.argset.type, p.fnameget);
			}
		}
		else{

		}
		iip_free(&p);
	}
}

__private void gencode_introspect(const char* services, const char* object, sdbusIntrospect_s* intro, int header){
	vector_foreach(intro->vinterfaces, i){
		gencode_introspect_interface(services, object, &intro->vinterfaces[i], header);
	}
	vector_foreach(intro->vchild, i){
		gencode_introspect(services, object, intro->vchild[i], header);
	}
}

/*@fn*/
void test_os(__unused const char* argA, __unused const char* argB){
	sdbus_s* sd = sdbus_new();
	if( !sd ) err_fail("sdbus");

	sdbusBusNames_s* bn = sdbus_bus_names(sd);
	vector_foreach(bn->acquired,i){
		printf("acquired:'%s'\n", bn->acquired[i]);
	}
	vector_foreach(bn->activable, i){
		printf("activable:'%s'\n", bn->activable[i]);
	}
	sdbus_bus_names_free(bn);
	
	/*
	char* xml = sdbus_introspect_raw(sd, "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager");
	if( !xml ) err_fail("xml");
	printf("<XML>%s</XML>\n", xml);
	*/
	
	const char* services = "org.freedesktop.NetworkManager";
	const char* object   = "/org/freedesktop/NetworkManager";
	sdbusIntrospect_s* introspect = sdbus_introspect(sd, services, object);
	//introspect_gencode(introspect);
	//vector_foreach(introspect->vchild, i){
	//	introspect_gencode(introspect->vchild[i]);
	//}
	gencode_introspect(services, object, introspect, 1);
	sdbus_introspect_free(introspect);
	

	sdbus_free(sd);
	return;
	
	cpu_auto_load_average_begin();	
	printf("core count:%d\n", cpu_core_count());
	
	double avg = cpu_auto_load_average(0);
	printf("load average:%f%%\n",avg);
	

	powerstate_begin();
	powerstate_s ps;
	if( powerstate_get(&ps, "BAT0") ){
		err_fail("read BAT0");
	}

	printps("capacity  ", ps.capacity);
	printps("energyFull", ps.energyFull);
	printps("energyNow ", ps.energyNow);
	printps("powerNow  ", ps.powerNow);
	printps("voltageMin", ps.voltageMin);
	printps("voltageNow", ps.voltageNow);
	printf("timeleft  :%f\n", ps.timeleft);
	printf("status    :%s\n", ps.status);

	powerstate_end();

	cpufreq_begin();
	size_t ncore = cpu_core_count();
	long v;
	for(size_t i = 0; i < ncore; ++i){
		cpufreq_property_get(&v, 0, SYS_CPUFREQ_SCALING_CURFQ);
		printps("cpu", v);
	}

	avg = cpu_auto_load_average(0);
	printf("load average:%f%%\n",avg);
	
	cpufreq_end();
	cpu_auto_load_average_end();

	printf("temp:%ld\n", thermal_get(0));
	printf("crit:%ld\n", thermal_critic_get(0,1));

	//spawn_disable_zombie();
	printf("<LS>\n");
	pid_t pid = spawn_shell("ls", 0);
	if( pid == -1 ){
		err_fail("spawn shell");
	}
	int ret = -1;
	if( spawn_wait(pid, &ret) ){
		dbg_fail("spawn wait");
	}
	printf("</LS return:%d>\n", ret);

	printf("<LS>\n");
	char* out;
	char* err;
	if( spawn_shell_slurp(&out, &err, &ret, "ls") ){
		err_fail("spawn shell");
	}
	printf("<OUT>\n");
	if( out ) puts(out);
	printf("</OUT>\n");
	printf("<ERR>\n");
	if( err ) puts(err);
	printf("</ERR>\n");
	printf("</LS return:%d>\n", ret);



	spawn_end();
}
