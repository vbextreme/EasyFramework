#ifndef EASYTYPE_H_INCLUDED
#define EASYTYPE_H_INCLUDED

#define EASYTYPE_V a

#include <inttypes.h>
#include <string.h>

#define PPC_MULTILINE_START do{
#define PPC_MULTILINE_END }while(0)
#define __VA_COUNT__(...) __VA_COUNT_IMPL__(__VA_ARGS__, 7,6,5,4,3,2,1)
#define __VA_COUNT_IMPL__(_1,_2,_3,_4,_5,_6,_7,N,...) N

#define forever() for(;;)

typedef void VOID;
typedef void* VARIANT;
typedef unsigned char BYTE;
typedef char CHAR;
typedef short int INT16;
typedef unsigned short int UINT16;
typedef int INT32;
typedef unsigned int UINT32;

typedef int64_t INT64;
typedef uint64_t UINT64;
#define S64(V) INT64_C(V)
#define lld PRId64

typedef float FLOAT32;
typedef double FLOAT64;

typedef UINT16  WORD;
typedef UINT32 DWORD;
typedef INT64  QWORD;
typedef DWORD FLAGS;

typedef enum { FALSE, TRUE } BOOL;

typedef size_t SIZET;

typedef struct _DATE
{
    INT16 y;
    BYTE m;
    BYTE d;
    BYTE hh;
    BYTE mm;
    BYTE ss;
}DATE;

#define DWBYTE0(VAL) ((VAL) & 0xFF)
#define DWBYTE1(VAL) (((VAL) >> 8) & 0xFF)
#define DWBYTE2(VAL) (((VAL) >> 16) & 0xFF)
#define DWBYTE3(VAL) ((VAL) >> 24)

#define UNIONBYTE(A,B) (((UINT32)(A)<<8) | (BYTE)(B))

#define HIDWORD(l) *((UINT32*)&(l))
#define LODWORD(l) *((UINT32*)&(l) + 1)

typedef void* TUPLE;



#define _CAT(A,B) A##B
#define CAT(A,B) _CAT(A,B)

#define __TUPLE_SIZEOF2(T1,T2) sizeof(T1) + sizeof(T2)
#define __TUPLE_SIZEOF3(T1,T2,T3) __TUPLE_SIZEOF2(T1,T2) + sizeof(T3)
#define __TUPLE_SIZEOF4(T1,T2,T3,T4) __TUPLE_SIZEOF3(T1,T2,T3) + sizeof(T4)
#define __TUPLE_SIZEOF5(T1,T2,T3,T4,T5) __TUPLE_SIZEOF4(T1,T2,T3,T4) + sizeof(T5)
#define __TUPLE_SIZEOF6(T1,T2,T3,T4,T5,T6) __TUPLE_SIZEOF5(T1,T2,T3,T4,T5) + sizeof(T6)
#define __TUPLE_SIZEOF7(T1,T2,T3,T4,T5,T6,T7) __TUPLE_SIZEOF6(T1,T2,T3,T4,T5,T6) + sizeof(T7)

#define __TUPLE2(T,T1,T2) ((UINT16*)(T))[2]  = sizeof(T1);

#define __TUPLE3(T,T1,T2,T3) __TUPLE2(T,T1,T2)\
                             ((UINT16*)(T))[3]  = __TUPLE_SIZEOF2(T1,T2);

#define __TUPLE4(T,T1,T2,T3,T4) __TUPLE3(T,T1,T2,T3)\
                                ((UINT16*)(T))[4]  = __TUPLE_SIZEOF3(T1,T2,T3);

#define __TUPLE5(T,T1,T2,T3,T4,T5) __TUPLE4(T,T1,T2,T3,T4)\
                                   ((UINT16*)(T))[5]  = __TUPLE_SIZEOF4(T1,T2,T3,T4);

#define __TUPLE6(T,T1,T2,T3,T4,T5,T6) __TUPLE5(T,T1,T2,T3,T4,T5)\
                                      ((UINT16*)(T))[6]  = __TUPLE_SIZEOF5(T1,T2,T3,T4,T5);

#define __TUPLE7(T,T1,T2,T3,T4,T5,T6,T7) __TUPLE5(T,T1,T2,T3,T4,T5,T6)\
                                         ((UINT16*)(T))[7]  = __TUPLE_SIZEOF6(T1,T2,T3,T4,T5,T6);

#define __TUPLE_BASE(T,...) T = malloc(sizeof(UINT16) * (__VA_COUNT__(__VA_ARGS__) + 1 ) + CAT(__TUPLE_SIZEOF,__VA_COUNT__(__VA_ARGS__))(__VA_ARGS__) );\
                           ((UINT16*)(T))[0]  = sizeof(UINT16) * (__VA_COUNT__(__VA_ARGS__) + 1 );\
                           ((UINT16*)(T))[1]  = 0;\
                           CAT(__TUPLE,__VA_COUNT__(__VA_ARGS__))(T,__VA_ARGS__)

#define tuple_new(T,...) PPC_MULTILINE_START\
                         __TUPLE_BASE(T,__VA_ARGS__)\
                         PPC_MULTILINE_END

#define tuple_free(T) free(T);

#define tuple(T,TYPE,P) *((TYPE*)( (BYTE*)(T) + ((UINT16*)(T))[0] + ((UINT16*)(T))[(P)+1]))
#define ptrtuple(T,TYPE,P) ((TYPE*)( (BYTE*)(T) + ((UINT16*)(T))[0] + ((UINT16*)(T))[(P)+1]))

typedef TUPLE EERR;

#define err_new(E) tuple_new( E, UINT32, CHAR[80], CHAR[80], CHAR[80], EERR)
#define err_number(E) tuple(E,UINT32,0)
#define err_file(E) ptrtuple(E,CHAR,1)
#define err_function(E) ptrtuple(E,CHAR,2)
#define err_description(E) ptrtuple(E,CHAR,3)
#define err_up(E) tuple(E,EERR,4)
#define err_allset(E,N,FN,DE,UP) err_number(E) = N;\
                                 strcpy(err_file(E),__FILE__);\
                                 strcpy(err_function(E),FN);\
                                 strcpy(err_description(E),DE);\
                                 err_up(E) = UP

#define err_print(E) printf("error %s:%s{%d=%s}\n",err_file(E),err_function(E),err_number(E),err_description(E))
#define err_printup(E) PPC_MULTILINE_START\
                        EERR __scan;\
                        for ( __scan = E; __scan; __scan = err_up(__scan))\
                           err_print(__scan);\
                       PPC_MULTILINE_END

#define err_free(E) tuple_free(E);
#define err_freeup(E)  PPC_MULTILINE_START\
                       EERR __scan;\
                       while(E)\
                       {\
                        __scan = err_up(E);\
                        err_free(E);\
                        E = __scan;\
                       }\
                       E = NULL;\
                       PPC_MULTILINE_END


#define return_err(N,FNC,DESC,UP)  PPC_MULTILINE_START\
                                   EERR __e;\
                                   err_new(__e);\
                                   err_allset(__e,N,FNC,DESC,UP);\
                                   return __e;\
                                   PPC_MULTILINE_END

# define errorblock(expr)							\
  ((expr)								\
   ? __ASSERT_VOID_CAST (0)						\
   : __assert_fail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION))

#endif // EASYTYPE_H_INCLUDED
