#ifndef __EF_JSON_H__
#define __EF_JSON_H__

/* if not build ef with -O2 compiler not use tail recursion and json parser can stack overflow cause callback depth */

#include <ef/type.h>

typedef enum {
	JSON_OK, 
	JSON_ERR_STRING_END, 
	JSON_ERR_STRING_LEN,
	JSON_ERR_STRING,
	JSON_ERR_UNICODE,
	JSON_ERR_NUMBER,
	JSON_ERR_CONSTANT,
	JSON_ERR_OBJECT_NAME,
	JSON_ERR_OBJECT_COLON,
	JSON_ERR_OBJECT_VALUE,
	JSON_ERR_OBJECT_END,
	JSON_ERR_ARRAY_END,
	JSON_ERR_UNASPECTED_CHAR,
	JSON_ERR_NOSPACE,
	JSON_ERR_USER,
	JSON_ERR_COUNT
}jsonError_e;

typedef struct json json_s;

typedef err_t(*jsonEvent_f)(json_s* ctx);
/** set ptr name to null for not free variable*/
typedef err_t(*jsonEventName_f)(json_s* ctx, char** name, size_t len);
typedef err_t(*jsonEventValue_f)(json_s* ctx, const char* name, size_t len);

typedef struct json{
	jsonEvent_f objectNew;            /**< event when object is created*/
	jsonEvent_f objectNext;           /**< event when have next object*/
	jsonEventName_f objectProperties; /**< property of object*/
	jsonEvent_f objectEnd;            /**< end object*/
	jsonEvent_f arrayNew;             /**< array is created*/
	jsonEvent_f arrayNext;            /**< next element on array*/
	jsonEvent_f arrayEnd;             /**< array end*/
	jsonEvent_f valueNull;            /**< NULL */
	jsonEvent_f valueTrue;            /**< TRUE */
	jsonEvent_f valueFalse;           /**< FALSE */
	jsonEventValue_f valueInteger;    /**< integer value*/
	jsonEventValue_f valueFloat;      /**< float value*/
	jsonEventName_f valueString;      /**< string value*/
	void* usrctx;                     /**< user data ctx*/
	long usrstat;                     /**< user status*/
	void* usrval;                     /**< user value*/
	size_t usrit;                     /**< user iterator*/
	void* usrgarbage;                 /**< user garbage*/
	size_t cline;                     /**< private numbers of current line*/
	char const* text;                 /**< private text*/
	char const* beginLine;            /**< private begin line*/
	char const* current;              /**< private current line*/
	jsonError_e err;                  /**< error */
	char** usrError;                  /**< user error*/
}json_s;

typedef enum {JSON_DEF_INT, JSON_DEF_UINT, JSON_DEF_LONG, JSON_DEF_ULONG, JSON_DEF_STRING, JSON_DEF_NUMBER, JSON_DEF_VECTOR, JSON_DEF_OBJECT} jsonDef_e;

typedef struct jsonDef{
	struct jsonDef* vintrospect;
	struct jsonDef* parent;
	const char* name;
	jsonDef_e type;
	size_t offof;
	size_t size;
}jsonDef_s;


/**before use json, disable locale for correct parsing double value*/
void json_begin(void);

/**after end use json, restore old locale*/
void json_end(void);

/** lexing json 
 * @param json structure
 * @param data text to parse
 * @return -1 for error 0 successfull
 */
err_t json_lexer(json_s* json, char const* data);

/** print error on stderr*/
void json_error(json_s* json);

/** return string allocated, remember to free, contains error descript*/
char* json_error_allocstr(json_s* json);

/** err_push json error*/
void json_push_error(json_s* json);

/** unescape json string, utf_begin required
 * @param js optional
 * @param str to escape
 * @param len len of str
 * @return new unescaped string , remember to free, or null and set json error
 */
char* json_unescape(json_s* js, const char* str, size_t len);

/** validate integer value
 * @param ret optional return number
 * @param number string number to validate
 * @param len len of number
 * @return 0 successfull, -1 error
 */
err_t json_long_validation(long* ret, const char* number, size_t len);

/** validate float value
 * @param ret optional return number
 * @param number string number to validate
 * @param len len of number
 * @return 0 successfull, -1 error
 */
err_t json_float_validation(double* ret, const char* number, size_t len);

/** create new definition begin with object
 * @param size sizeof struct
 * @return new def
 */
jsonDef_s* json_parse_new_object(size_t size);

/** create new definition begin with vector, next declare is type of vector
 * @param size sizeof struct
 * @return new def
 */
jsonDef_s* json_parse_new_vector(void);

/** declare type as num*/
jsonDef_s* json_parse_declare_number(jsonDef_s* def, const char* name, size_t offof, size_t size, jsonDef_e typeNum);

/** declare type int
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 */
jsonDef_s* json_parse_declare_int(jsonDef_s* def, const char* name, size_t offof);

/** declare type uint
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 */
jsonDef_s* json_parse_declare_uint(jsonDef_s* def, const char* name, size_t offof);

/** declare type long
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 */
jsonDef_s* json_parse_declare_long(jsonDef_s* def, const char* name, size_t offof);

/** declare type ulong
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 */
jsonDef_s* json_parse_declare_ulong(jsonDef_s* def, const char* name, size_t offof);

/** declare type double
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 */
jsonDef_s* json_parse_declare_double(jsonDef_s* def, const char* name, size_t offof);

/** declare type string
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 */
jsonDef_s* json_parse_declare_string(jsonDef_s* def, const char* name, size_t offof);

/** declare type vector, need child element than declare type of vector
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 */
jsonDef_s* json_parse_declare_vector(jsonDef_s* def, const char* name, size_t offof);

/** declare type object
 * @param def parent
 * @param name name of property if is object
 * @param offof offsetof of var if is a struct
 * @param size sizeof struct
 */
jsonDef_s* json_parse_declare_object(jsonDef_s* def, const char* name, size_t offof, size_t size);

/** add previous definition in def*/
jsonDef_s* json_parse_declare_def(jsonDef_s* def, jsonDef_s* sub);

/** free def*/
void json_def_free(jsonDef_s* def);

/** parse json, each element is a pointer to type, but not char*
 * @param def definition, free after parse
 * @param data json string
 * @return NULL for error otherwise object or vector
 */
void* json_parse(jsonDef_s* def, const char* data);

















#endif

