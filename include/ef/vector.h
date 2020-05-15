#ifndef __EF_VECTOR_H__
#define __EF_VECTOR_H__

#include <ef/type.h>
#include <ef/memory.h>

typedef void (*vfree_f)(void*);

typedef struct vector{
	size_t size;      /**< size of allocated mem*/
	size_t count;     /**< used object*/
	size_t sof;       /**< sizeof type*/
	vfree_f fn;       /**< cleanup function*/
	void* element[0]; /**< pointer to mem*/
}vector_s;

#define VECTOR(V) ((vector_s*)(ADDR(V)-sizeof(vector_s)) )

/** create new vector
 * @param sof sizeof element
 * @param size begine element size 
 * @param fn cleanup function
 * @return memory or fail
 */
void* vector_new_raw(size_t sof, size_t size, vfree_f fn);

/** create new vector
 * @param T type vector
 * @param S begine element size 
 * @param F free function
 * @return memory or NULL for error
 */
#define vector_new(T, S, F) (T*)vector_new_raw(sizeof(T), S, F)

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
 * @return new mem addr or fail
 */
void* vector_resize(void* mem, size_t size);

/** increase size of vector of count element if need
 * @param ptrmem pointer to vector mem
 * @param count elements to upsize
 */
void vector_upsize(void* ptrmem, size_t count);

/** decrease size of vector of count element if need
 * @param ptrmem pointer to vector mem
 */
void vector_downsize(void* ptrmem);

/** clear vector, set count to 0 and call free function
 * @param m mem of vector
 */
void vector_clear(void* m);

/** check id vector is empty
 * @param M mem of vector
 * 1 empty 0 no
 */ 
#define vector_isempty(M) (vector_count(M)==0)

/** remove element from index
 * @param ptrmem address of vector mem, can be change
 * @param index element to remove
 */
void vector_remove_raw(void** ptrmem, const size_t index);

/** remove element from index
 * @param M vector, address can change
 * @param I element to remove
 */
#define vector_remove(M, I) vector_remove_raw((void**)&(M), I);

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
	vector_upsize(&(M), 1);\
	_ret_ = &(M)[vector_count(M)++];\
	_ret_;\
})

/** copy element to new last element in vector
 * @param M mem of vector
 * @param ELEMENT element to copy
 */
#define vector_push_back(M, ELEMENT) do{\
	vector_upsize(&M, 1);\
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

/** shuffle a vector, call mth_random_begin() before use
 * @param ptrmem pointer to vector
 * @param begin begin index
 * @param end end index
 */
void vector_shuffle(void* ptrmem, size_t begin, size_t end);

#define vector_shuffle_all(V) vector_shuffle(V, 0, vector_count(V)-1)


#endif 
