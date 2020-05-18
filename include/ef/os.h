#ifndef __EF_OS_H__
#define __EF_OS_H__

#include <ef/type.h>

typedef struct sdbus sdbus_s;

typedef void* sdbusMessage_h;

typedef struct sdbusBusNames{
	char** acquired;
	char** activable;
}sdbusBusNames_s;

sdbus_s* sdbus_new(void);
void sdbus_free(sdbus_s* sd);
sdbusBusNames_s* sdbus_bus_names(sdbus_s* sd);
void sdbus_bus_names_free(sdbusBusNames_s* bn);
sdbusMessage_h sdbus_method(sdbus_s* sd, const char* service, const char* object, const char* interface, const char* member);
char* sdbus_reply_string(sdbusMessage_h msg);
char* sdbus_introspect_raw(sdbus_s* sd, const char* service, const char* object);



/** set locale and return previous setting*/
const char* os_setlocale(int category, const char* locale);

/** get current locale */
#define os_getlocale() locale_charset()

/** init cpu feature*/
void os_begin(void);

#endif

