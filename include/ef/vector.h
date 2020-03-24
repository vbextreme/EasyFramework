#ifndef __EF_VECTOR_H__
#define __EF_VECTOR_H__

#include <ef/type.h>
#include <ef/memory.h>

typedef struct vector{
	size_t size;    /**< size of allocated mem*/
	size_t count;   /**< used object*/
	size_t minimal; /**< minimal element to allocate*/
	size_t sof;     /**< sizeof type*/
	void* element[0];  /**< pointer to mem*/
}vector_s;

#define VECTOR(V) ((vector_s*)(ADDR(V)-sizeof(vector_s)) )

/** create new vector
 * @param sof sizeof element
 * @param size begine element size 
 * @param minimal minimal count of elements
 * @return memory or NULL for error
 */
void* vector_new_raw(size_t sof, size_t size, size_t minimal);

/** create new vector
 * @param T type vector
 * @param S begine element size 
 * @param M minimal count of elements
 * @return memory or NULL for error
 */
#define vector_new(T, S, M) (T*)vector_new_raw(sizeof(T), S, M)

/** get vector count */
#define vector_count(V) VECTOR(V)->count

/** get vector size */
#define vector_size(V) VECTOR(V)->size

/** get vector minimal*/
#define vector_minimal(V) VECTOR(V)->minimal

/** get sizeof */
#define vector_sizeof(V) VECTOR(V)->sof

/** free memory allocate inside vector
 * @param v vector
 */
void vector_free(void* v);

/** for cleanup
 * @see __cleanup
 */
void vector_free_auto(void* v);

/** for cleanup
* @see __cleanup
*/
#define __vector_free __cleanup(vector_free_auto)

/** resize of vector
 * @param mem vector
 * @param size elements
 * @return NULL error, or new mem addr 
 */
void* vector_resize(void* mem, size_t size);

/** increase size of vector of count element if need
 * @param ptrmem pointer to vector mem
 * @param count elements to upsize
 * @return -1 error 0 ok
 */
err_t vector_upsize(void* ptrmem, size_t count);

/** clear vector, set count to 0
 * @param M mem of vector
 */
#define vector_clear(M) (vector_count(M) = 0)

/** check id vector is empty
 * @param M mem of vector
 * 1 empty 0 no
 */ 
#define vector_isempty(M) (vector_count(M)==0)

/** remove element from index
 * @param m vector mem
 * @param index element to remove
 */
void vector_remove(void* m, const size_t index);

/** add space in index position for setting new value
 * @param ptrmem pointer to mem of vector
 * @param index position where add new space
 */
err_t vector_add_raw(void* ptrmem, const size_t index);

/** add element in position I, warning address of M can change
 * @param M mem of vector
 * @param I index
 * @param E element
 */
#define vector_add(M, I, E) do{\
	if( !vector_add_raw(&(M), I) ){\
		(M)[I] = E;\
	}\
}while(0)

/** get a ptr of new last element of vector
 * @param M mem of vector
 * @return prt of new element
 */
#define vector_get_push_back(M) ({\
	void* _ret_ = NULL;\
	if( !vector_upsize(&(M), 1) )\
	   	_ret_ = &(M)[vector_count(M)++];\
	_ret_;\
})

/** copy element to new last element in vector
 * @param M mem of vector
 * @param ELEMENT element to copy
 */
#define vector_push_back(M, ELEMENT) do{\
	if( !vector_upsize(&M, 1) )\
	   	(M)[vector_count(M)++] = ELEMENT;\
}while(0)

/** extract last element of vector, warning not check if vector is empty
 * @param M mem of vector
 * @return ELEMENT element to copy
 */
#define vector_pull_back(M) (M)[--vector_count(M)]

/** foreach element of vector
 * @param M mem of vector
 * @param I iterator name
 */
#define vector_foreach(M, I) for(size_t I = 0; I < vector_count(M); ++I )

/** qsort vector
 * @param M mem of vector
 * @param CMPFN compare function
 */
#define vector_qsort(M, CMPFN) qsort(M, vector_count(M), vector_sizeof(M), CMPFN)

/** resize vector to count object
 * @param ptrmem pointer to mem of vector
 * @return -1 error 0 successfull
 */
err_t vector_fitting(void* ptrmem);



#endif 
