#ifndef __EF_OS_H__
#define __EF_OS_H__

#include <ef/type.h>
#include <ef/xml.h>

/*TODO FUNCTION HELPER FOR VARIANT AND KV*/

typedef struct sdbus sdbus_s;

typedef void* sdbusMessage_h;
typedef void* sdbusError_h;

typedef int (*sdbusCallback_f)(sdbusMessage_h m, void *userdata, sdbusError_h reterr);

typedef struct sdbusBusNames{
	char** acquired;
	char** activable;
}sdbusBusNames_s;

/*
DBus Type	signature	D mapping	Description
BYTE	y	ubyte	8-bit unsigned integer
BOOLEAN	b	bool	Boolean value
INT16	n	short	16-bit signed integer
UINT16	q	ushort	16-bit unsigned integer
INT32	i	int	32-bit signed integer
UINT32	u	uint	32-bit unsigned integer
INT64	x	long	64-bit signed integer
UINT64	t	ulong	64-bit unsigned integer
DOUBLE	d	double	IEEE 754 double
STRING	s	char[]	UTF-8 string (must be valid UTF-8).
OBJECT_PATH	o	char[]	Name of an object instance
SIGNATURE	g	char[]	A type signature
FD      h   int      fd type
ARRAY	a	type[]	Array //argument is count array, array
STRUCT	()	Struct!()	Struct
VARIANT	v	Variant!()	Variant type (the type of the value is part of the value itself)
DICT_ENTRY	{}	type1[ type2 ]	A key-value map
*/

#define SDBUS_PROPERTY_ACCESS_READ  0x01
#define SDBUS_PROPERTY_ACCESS_WRITE 0x02

typedef union sdbusVariantType{
	uint8_t y;  /**< byte */
	int b;      /**< bool */
	int16_t n;  /**< short */
	uint16_t q; /**< unsigned short*/
	int i;      /**< int*/
	int32_t u;  /**< unsigned*/
	int64_t x;  /**< long*/
	uint64_t t; /**< unsigned long*/
	double d;   /**< double*/
	int h;      /**< fd*/
	char* s;    /**< string*/
	char* o;    /**< object interface name*/
	char* g;    /**< signature*/
	void* v;    /**< if struct v is vector of elements, or sdbusVariant_s, or sdbusKV_s*/
}sdbusVariantType_s;

typedef union sdbusVariant{
	char* type;
	sdbusVariantType_s var;
}sdbusVariant_s;
	
typedef struct sdbusKV{
	sdbusVariant_s key;
	sdbusVariant_s value;
}sdbusKV_s;

typedef struct sdbusIIMArg{
	char* name;
	char* type;
	int in;
}sdbusIIMArg_s;

typedef struct sdbusIIProto{
	char* name;
	sdbusIIMArg_s* vargs;
}sdbusIIProto_s;

typedef struct sdbusIIProperty{
	char* name;
	char* type;
	int access;
}sdbusIIProperty_s;

typedef struct sdbusIInterface{
	char* name;
	sdbusIIProto_s* vmethods;
	sdbusIIProto_s* vsignal;
	sdbusIIProperty_s* vproperty;
}sdbusIInterface_s;

typedef struct sdbusIntrospect{
	sdbusIInterface_s* vinterfaces;
	struct sdbusIntrospect** vchild;
	char* str;
	xml_s* xml;
}sdbusIntrospect_s;

sdbus_s* sdbus_new(void);
void sdbus_free(sdbus_s* sd);
sdbusBusNames_s* sdbus_bus_names(sdbus_s* sd);
void sdbus_bus_names_free(sdbusBusNames_s* bn);
sdbusMessage_h sdbus_methodv(sdbus_s* sd, const char* service, const char* object, const char* interface, const char *member, const char *types, va_list ap);
sdbusMessage_h sdbus_method(sdbus_s* sd, const char* service, const char* object, const char* interface, const char* member, const char* types, ...);
err_t sdbus_property_set(sdbus_s* sd, const char* service, const char* object, const char* interface, const char *member, sdbusVariant_s* v);
sdbusMessage_h sdbus_property_get(sdbus_s* sd, const char* service, const char* object, const char* interface, const char *member, sdbusVariant_s* v);
err_t sdbus_on_signal(sdbus_s* sd, const char* serviceSender, const char* object, const char* interface, const char* member, sdbusCallback_f fn, void* userdata);
int sdbus_fd_get(sdbus_s* sd);
int sdbus_fd_eventfd(sdbus_s* sd);
uint64_t sdbus_fd_get_timeout(sdbus_s* sd);
void sdbus_message_free(sdbusMessage_h msg);
void sdbus_message_autofree(void* m);
#define __sdbus_message_free __cleanup(sdbus_message_autofree)
void sdbus_read_free_variant_content(sdbusVariant_s* v);
void sdbus_read_free_kv_content(sdbusKV_s* kv);
void sdbus_read_free_vstruct(const char* type, void* v);
void sdbus_read_free_array(const char* type, void* v);
err_t sdbus_message_read_kv(sdbusMessage_h msg, const char* type, sdbusKV_s* v);
err_t sdbus_message_read_variant(sdbusMessage_h msg, sdbusVariant_s* v);
err_t sdbus_message_read_basic(sdbusMessage_h msg, const char type, void* ret);
err_t sdbus_message_read_array(sdbusMessage_h msg, const char* type, void* vec);
err_t sdbus_message_readv(sdbusMessage_h msg, const char* type, va_list ap);
err_t sdbus_message_read(sdbusMessage_h msg, const char* types, ...);

void sdbus_introspect_free(sdbusIntrospect_s* i);
char* sdbus_introspect_raw(sdbus_s* sd, const char* service, const char* object);
sdbusIntrospect_s* sdbus_introspect(sdbus_s* sd, const char* service, const char* object);

/** set locale and return previous setting*/
const char* os_setlocale(int category, const char* locale);

/** get current locale */
#define os_getlocale() locale_charset()

/** init cpu feature*/
void os_begin(void);

#endif

