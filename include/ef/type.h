#ifndef __EF_TYPE_H__
#define __EF_TYPE_H__

/*
 * 1.2.6 -----
 * 1.3.0 broken previous version
 *		gui:
 *			themes
 *			bar:
 *				circle
 *			option
 *			round
 *			simple
 *			composite layers
 *			msgbox
 *			combo
 *
 *		g2d:
 *			arc
 *			supersampling
 *
 *		svg:
 *			optimize
 *
 *		vector:
 *			exponential
 *			reduce
 *			autofree
 *
 *		thr:
 *			remove FUTEX_FD
 *			add eventfd
 *			change qmessages
 *
*/


#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif
#ifndef _XOPEN_SPURCE
	#define _XOPEN_SPURCE 600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

/** compare function for qsort */
typedef int (*qsort_f)(const void* a, const void* b);

/** type for byte*/
typedef char byte_t;

/** type for word*/
typedef uint16_t word_t;

/** type for double word*/
typedef uint32_t dword_t;

/** type for quad word*/
typedef uint64_t qword_t;

/** type for flags*/
typedef dword_t flags_t;

/** type for error*/
typedef int err_t;

/** type for boolean*/
typedef enum { FALSE, TRUE } bool_t;

/** get size of array allocated in same scope and in stack
 * @param V is an array
 */
#define lenght_stack_vector(V) (sizeof(V)/sizeof(V[0]))

#ifndef _Y_ 
/** macro value yes*/
	#define _Y_ 1
#endif
#ifndef _N_
/** macro value no*/
	#define _N_ 0
#endif

/** macro is empty*/
#define EMPTY_MACRO do{}while(0)

/** swap two variable
 * @param A
 * @param B
 */
#define SWAP(A,B) ({ \
		__auto_type __tmp__ = A;\
		A = B;\
		B = __tmp__;\
	})

/** use address for move to data*/
#define ADDR(VAR) ((char*)(VAR))

/** to address */
#define ADDRTO(VAR, SO, I) ( ADDR(VAR) + ((SO)*(I)))

#define __VA_COUNT__(...) __VA_COUNT_IMPL__(foo, ##__VA_ARGS__,9,8,7,6,5,4,3,2,1,0)
#define __VA_COUNT_IMPL__(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,N,...) N
#define __CONCAT__(A,B) A##B
#define __CONCAT_EXPAND__(A,B) __CONCAT__(A,B)
//#define myfnc(...) __CONCAT_EXPAND__(myfnc_n, __VA_COUNT(__VA_ARGS__))(##__VA_ARGS__)

/** forever loop */
#define forever() for(;;)
/** private is more elegant? */
#define __private static
/** unused value */
#define __unused __attribute__((unused))
/** cleanup function called when exit from scope
 * @param FNC function to call
 */
#define __cleanup(FNC) __attribute__((__cleanup__(FNC)))
/** is printf
 * @param FRMT int argument where stored format, start from 1
 * @param VA int argument where stored ..., start from 1
 */
#define __printf(FRMT,VA) __attribute__((format (printf, FRMT, VA)))
/** is const */
#define __const __attribute__((const))
/** is packed */
#define __packed __attribute__((packed))
/** target for runtime optimization */
#define __target(T) __attribute__((target(T)))
/** target for runtime optimization */
#define __target_clone(arg...) __attribute__((target_clones(## arg)))
/** target for runtime optimization */
#define __target_default __target("default")
/** target for runtime optimization */
#define __target_popcount __target("popcnt")
/** target for runtime optimization */
#define __target_vectorization __attribute__((target_clones("default","mmx","sse","sse2","sse3","ssse3","sse4.1","sse4.2","avx","avx2")))
/** target for runtime optimization */
#define __target_default_popcount __target_clone("default","popcnt")
/** target for runtime optimization */
#define __target_default_vectorization __target_clone("default","mmx","sse","sse2","sse3","ssse3","sse4.1","sse4.2","avx","avx2")
/** target for runtime optimization */
#define __target_all __target_clone("default","popcnt","mmx","sse","sse2","sse3","ssse3","sse4.1","sse4.2","avx","avx2")
/** init cpu before use runtime optimizations */
#define __cpu_init() __builtin_cpu_init()
/** test is support bitcount */
#define __cpu_supports_popcount() __builtin_cpu_supports ("popcnt")
/** test is support vectorization */
#define __cpu_supports_vectorization() (__builtin_cpu_supports("mmx") || __builtin_cpu_supports("sse") || __builtin_cpu_supports("sse2") || __builtin_cpu_supports("sse3") || __builtin_cpu_supports("ssse3") || __builtin_cpu_supports("sse4.1") || __builtin_cpu_supports("sse4.2") || __builtin_cpu_supports("avx") || __builtin_cpu_supports("avx2"))

#define __mul_overflow(R,A,B) __builtin_mul_overflow(A,B,R)

/** KiB value */
#define KiB (1024UL)
/** MiB value */
#define MiB (KiB*KiB)
/** GiB value */
#define GiB (MiB*MiB)

/** to call pragma*/
#define DO_PRAGMA(DOP) _Pragma(#DOP)
/** suppress compiler warning */
#define UNSAFE_BEGIN(FLAGS) DO_PRAGMA(GCC diagnostic push); DO_PRAGMA(GCC diagnostic ignored FLAGS)
#define UNSAFE_END DO_PRAGMA(GCC diagnostic pop)

/*disable scan build error*/
#if defined OMP_ENABLE && !defined __clang__
/** omp parallel for*/
	#define __parallef DO_PRAGMA(omp parallel for)
/** omp parallel for collaps Z */
	#define __parallefc(Z) DO_PRAGMA(omp parallel for collapse Z)
#else
	#define __parallef
	#define __parallefc(Z) 
#endif

#ifndef DBG_OUTPUT
	#define DBG_OUTPUT stderr
#endif

#ifdef DEBUG_COLOR
	#define DBG_COLOR_INFO    "\033[36m"
	#define DBG_COLOR_WARNING "\033[93m"
	#define DBG_COLOR_ERROR   "\033[31m"
	#define DBG_COLOR_FAIL    "\033[91m"
	#define DBG_COLOR_RESET   "\033[m"
#else
	#define DBG_COLOR_INFO    ""
	#define DBG_COLOR_WARNING ""
	#define DBG_COLOR_ERROR   ""
	#define DBG_COLOR_FAIL    ""
	#define DBG_COLOR_RESET   ""
#endif

#ifndef DBG_INFO
	#define DBG_INFO    "info"
#endif
#ifndef DBG_WARNING
	#define DBG_WARNING "warning"
#endif
#ifndef DBG_ERROR
	#define DBG_ERROR   "error"
#endif
#ifndef DBG_FAIL
	#define DBG_FAIL    "fail"
#endif
#ifndef DBG_ERRNO
	#define DBG_ERRNO   "errno"
#endif

#ifndef DBG_LVL_FAIL
	#define DBG_LVL_FAIL    1
#endif
#ifndef DBG_LVL_ERROR
	#define DBG_LVL_ERROR   2
#endif
#ifndef DBG_LVL_WARNING
	#define DBG_LVL_WARNING 3
#endif
#ifndef DBG_LVL_INFO
	#define DBG_LVL_INFO    4
#endif

#if DEBUG_ENABLE >= 1
	#define dbg(TYPE, COLOR, FORMAT, arg...) do{\
										fprintf(DBG_OUTPUT, "%s[%u]:{%ld} %s(): %s%s" DBG_COLOR_RESET "::" FORMAT "\n",\
										__FILE__,\
									   	__LINE__,\
									   	pthread_self(),\
										__FUNCTION__,\
										COLOR, TYPE,\
										## arg); \
										fflush(DBG_OUTPUT);\
									}while(0)
/** print message fail and exit from application*/
	#define dbg_fail(FORMAT, arg...) do{ \
										dbg(DBG_FAIL, DBG_COLOR_FAIL, FORMAT, ## arg);\
										exit(1);\
									 }while(0)
/** print errno message*/
	#define dbg_errno() dbg(DBG_ERRNO, DBG_COLOR_ERROR, " %d descript: %s", errno, strerror(errno)) 
#else
	#define dbg(TYPE, FORMAT, arg...) EMPTY_MACRO
	#define dbg_fail(FORMAT, arg...) do{exit(1);}while(0)
	#define dbg_errno() EMPTY_MACRO
#endif

#if DEBUG_ENABLE > DBG_LVL_FAIL
/** print error message */
	#define dbg_error(FORMAT, arg...) dbg(DBG_ERROR, DBG_COLOR_ERROR, FORMAT, ## arg)
#else
	#define dbg_error(FORMAT, arg...) EMPTY_MACRO
#endif

#if DEBUG_ENABLE > DBG_LVL_ERROR
/** print warning message */
	#define dbg_warning(FORMAT, arg...) dbg(DBG_WARNING, DBG_COLOR_WARNING, FORMAT, ## arg)
#else
	#define dbg_warning(FORMAT, arg...) EMPTY_MACRO
#endif

#if DEBUG_ENABLE > DBG_LVL_WARNING
/** print infoe */
	#define dbg_info(FORMAT, arg...) dbg(DBG_INFO, DBG_COLOR_INFO, FORMAT, ## arg)
#else
	#define dbg_info(FORMAT, arg...) EMPTY_MACRO
#endif
	
#if ASSERT_ENABLE == _Y_
/** assert */
	#define iassert(C) do{ if ( !(C) ){fprintf(stderr,"assertion fail %s[%u]: %s(%s)\n", __FILE__, __LINE__, __FUNCTION__, #C); exit(0);}}while(0)
#else
	#define iassert(C) EMPTY_MACRO
#endif

/**static assert*/
#define iassert_static(EX, MSG) _Static_assert(EX, MSG)

#ifdef __clang__
	#define scan_build_unknown_cleanup(E) free(E)
#else
	#define scan_build_unknown_cleanup(E)
#endif

#endif 
