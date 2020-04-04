#ifndef __EF_ERR_H__
#define __EF_ERR_H__

#include <ef/type.h>
#include <threads.h>

typedef struct eerr{
	char* descript;
	size_t size;
	size_t len;
	size_t flags;
	size_t* restore;
	size_t rc;
	size_t rsz;
}eerr_s;

#define ERR_REALLOC 32
#define ERR_ENABLED    0x01
#define ERR_RESTORE_STACK_SIZE 32

#ifdef __clang__
	#define thread_local 
#endif

#ifndef ERR_DEC_OBJ
	extern thread_local eerr_s gerr;
#endif

/** call before use error */
void err_begin(void);

/** call after end of use error */
void err_end(void);

/** push new error descript */
__printf(1,2) void err_pushf(const char* format, ...);

/** push new error descript, call err_pushf with 'error: your error nl'*/
#define err_push(F, arg...) do{\
   	if( gerr.descript && (gerr.flags & ERR_ENABLED) ){\
		err_pushf("error %s->%s(%u):: ", __FILE__, __FUNCTION__, __LINE__);\
		err_pushf(F, ## arg);\
		err_pushf("\n");\
	}\
}while(0)

/** push new error with errno descript, call err_pushf with 'error: your error nl errno(N): descript nl'*/
#define err_pushno(F, arg...)  do{\
	if( gerr.descript && (gerr.flags & ERR_ENABLED) ){\
	   	err_pushf("error %s->%s(%u):: ", __FILE__, __FUNCTION__, __LINE__);\
		err_pushf(F, ## arg);\
		err_pushf("\nerrno(%d): %s\n", errno, strerror(errno));\
	}\
}while(0)

/** push error, print and exit */
#define err_fail(F, arg...)  do{\
	if( gerr.descript && (gerr.flags & ERR_ENABLED) ){\
		err_pushf("fail %s->%s(%u):: ", __FILE__, __FUNCTION__, __LINE__);\
		err_pushf(F, ## arg);\
	   	err_pushf("\n");\
		err_print();\
	}\
	exit(1);\
}while(0)

/** disable error, automatic store*/
void err_disable(void);
/** enable error, automatic store*/
void err_enable(void);
/** store enable/disable error*/
void err_store(void);
/** restore enable/disable error*/
void err_restore(void);
/** get string of error*/
const char* err_descript(void);
/** print error on stderr*/
void err_print(void);
/** clear all error*/
void err_clear(void);

#endif 
