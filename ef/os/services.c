#include <ef/os.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/err.h>

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


sdbusMessage_h sdbus_method(sdbus_s* sd, const char* service, const char* object, const char* interface, const char* member){
	sd_bus_message* reply = NULL;
	sd_bus_error serr = SD_BUS_ERROR_NULL;

	int ret = sd_bus_call_method(sd->bus, service, object, interface, member, &serr, &reply, "");
	if(	ret < 0 ){
		err_push("sdbus(%d):'%s'", ret, strerror(-ret));
		err_push("sdbus.err.name: %s", serr.name);
		err_push("sdbus.err.message: %s", serr.message);
		sd_bus_error_free(&serr);
		return NULL;
	}
	sd_bus_error_free(&serr);

	return reply;
}

char* sdbus_reply_string(sdbusMessage_h msg){
	char* ret;
	int err = sd_bus_message_read(msg, "s", &ret);
	if( err < 0 ){
		err_push("sdbus(%d):'%s'", err, strerror(-err));
		return NULL;
	}
	sd_bus_message_unref(msg);
	return ret;
}

char* sdbus_introspect_raw(sdbus_s* sd, const char* service, const char* object){
	sdbusMessage_h msg = sdbus_method(sd, service, object, "org.freedesktop.DBus.Introspectable", "Introspect");
	if(	!msg ) return NULL;	
	return sdbus_reply_string(msg);
}
























