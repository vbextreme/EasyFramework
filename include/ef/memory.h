#ifndef __EF_MEMORY_H__
#define __EF_MEMORY_H__

#include <ef/type.h>
#include <ef/mth.h>
#include <fcntl.h>

typedef int pkey_t;

#define MEM_PROTECT_DISABLED 0
#ifdef MEM_UNDECLARE_PKEY
	#define MEM_PROTECT_WRITE 1
	#define MEM_PROTECT_RW 2
#else
	#define MEM_PROTECT_WRITE PKEY_DISABLE_WRITE
	#define MEM_PROTECT_RW PKEY_DISABLE_ACCESS
#endif

/****************/
/*** memory.c ***/
/****************/

/** allocate memory
 * @param TYPE is type
 * @return pointer of type
 * @code
 * int* = mem_new(int);
 * @endcode
 */
#define mem_new(TYPE) (TYPE*)malloc(sizeof(TYPE))

/** allocate many memory
 * @param TYPE is type
 * @param COUNT how many type
 * @return pointer of type
 * @code
 * int* = mem_many(int,5);
 * @endcode
 */
#define mem_many(TYPE,COUNT) (TYPE*)malloc(sizeof(TYPE)*(COUNT))

/** allocate memory aligned, raw version
 * @param size is sizeof type
 * @param alignedto is size of aligned
 * @return pointer of memory
 */
void* mem_many_aligned_raw(size_t* size, size_t alignedto);

/** allocate memory aligne, wrap of raw version for use same mem_many
 * @param TYPE is type
 * @param PTRCOUNT how many type
 * @param ALIGNED is size of aligned
 * @return pointer of type
 */
#define mem_many_aligned(TYPE, PTRCOUNT, ALIGNED) ({\
		size_t n = *(PTRCOUNT)*sizeof(TYPE);\
		(TYPE*)mem_many_aligned_raw(&n,ALIGNED);\
	})

/** same mem_new buf set 0 
 * @see mem_new
 */
#define mem_zero(TYPE) (TYPE*)calloc(1,sizeof(TYPE))

/** same mem_many buf set 0 
 * @see mem_many
 */
#define mem_zero_many(TYPE,COUNT) (TYPE*)calloc(COUNT,sizeof(TYPE))

/** allocate memory for flexible structure, do not use clang, this is only for scanbuild fail */
#ifdef __clang__
	#define mem_flexible_structure_new(TYPESTRUCT, SOF, COUNT) (TYPESTRUCT*)malloc(ROUND_UP(sizeof(TYPESTRUCT) + SOF * (COUNT), 8))
#else
	#define mem_flexible_structure_new(TYPESTRUCT, SOF, COUNT) (TYPESTRUCT*)malloc(ROUND_UP(sizeof(TYPESTRUCT) + SOF * (COUNT), sizeof(void*)))
#endif

/** only for even mem*/
#define mem_free(OBJ) free(OBJ)

/** set a NULL pointer after free*/
#define mem_free_safe(OBJ) do{free(OBJ); OBJ=NULL;}while(0)

/** function for cleanup
 * @see __cleanup
 */
void mem_free_auto(void* mem);

/** attribute for auto free memory when exit from scope
 * @code
 * {
 *	__mem_free int* mem = mem_many(int, 5);
 * }
 * @endcode
 * memory is automatic free
 */
#define __mem_free __cleanup(mem_free_auto)

/** same malloc but exit if not allocate memory*/
void* malloc_or_die(size_t sz);

/** same mem_new but die if fail
 * @see mem_new
 * @see malloc_or_die
 */
#define mem_new_or_die(TYPE) (TYPE*)malloc_or_die(sizeof(TYPE))

/** same mem_many but die if fail
 * @see mem_many
 * @see malloc_or_die
 */
#define mem_many_or_die(TYPE) (TYPE*)malloc_or_die(sizeof(TYPE)*(COUNT))

/** allocate matrx
 * @param y is row
 * @param sz id sizeof each row
 * @return ptr memory
 */
void* mem_matrix_new(size_t y, size_t sz);

/** free matrix
 * @param b is ptr to mem
 * @param y is row
 */
void mem_matrix_free(void* b, size_t y); 

/** set memory to 0
 * @param T is type
 * @param M is ptr
 * @code
 * int i;
 * mem_clear(int, &i);
 * @endcode
 */
#define mem_clear(T,M) do{memset(M,0,sizeof(T));}while(0)

/** set memory to 0
 * @param T is type
 * @param M is ptr
 * @param N is count of object
 * @code
 * int i[5];
 * mem_clear_many(int, i, 5);
 * @endcode
 */
#define mem_clear_many(T,M,N) do{memset(M,0,sizeof(T)*(N));}while(0)

/** realloc memory
 * @param mem address of memory, warning address can change
 * @param size new size
 * @return 0 successfull; -1 error, err is pushed errno is setted
 */
err_t mem_resize_raw(void** mem, const size_t size);

#define mem_resize(VAR,TYPE,N) mem_resize_raw((void**)&(VAR), sizeof(TYPE) * (N))

/****************/
/*** shared.c ***/
/****************/
/** create shared memory
 * @param name nameof memory, examples: /mymem
 * @param privilege the memory privileges
 * @param size size of memory
 * @return pointer to mem
 */
void* mem_shared_create(const char* name, int privilege, size_t size);

/** open an exists shared memory
 * @param name nameof memory, examples: /mymem
 * @return pointer to mem
 */
void* mem_shared_open(const char* name);

/** open or create if not exists
 * @param name nameof memory, examples: /mymem
 * @param priv the memory privileges
 * @param size size of memory 
 * @return pointer to mem
 */
void* mem_shared_create_or_map(const char* name, int priv, size_t size);

/** get size of memory
 * @param name nameof memory, examples: /mymem
 */
size_t mem_shared_size(const char* name);

/** close a memory for this process
 * @param mem pointer to memory
 * @param size size of memory
 */
void mem_shared_close(void* mem, size_t size);

/** delete a memory for all
 * @param name nameof memory, examples: /mymem
 */
err_t mem_shared_delete(const char* name);

/** get ptr and increment address of mem
 * @param mem address
 * @param size sizeof memory
 * @return pointer to memory and increment mem
 */
void* mem_shared_alloc_ptr(void** mem, size_t size);

/*** heap.c ***/

/** release heap
 * @param mem pointer of memory
 * @param size size of memory
 */
void mem_heap_close(void* mem, size_t size);

/** allocate memory in heap
 * @param size sizeof memory, return new size rounded
 * @return pointer to mem
 */
void* mem_heap_alloc(size_t* size);

/*** protect.c ***/
/** get new key*/
pkey_t mem_pkey_new(unsigned int mode);
/** protect memory*/
err_t mem_protect(pkey_t* key, void* addr, size_t size, unsigned int mode);
/** changhe memory protection*/
err_t mem_protect_change(pkey_t key, unsigned int mode, void* addr);

#endif
