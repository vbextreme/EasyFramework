#ifndef __EF_PHQ_H__
#define __EF_PHQ_H__

#include <ef/type.h>

/** callback compare priority funcion*/
typedef int (*phqCompare_f)(size_t a, size_t b);
/** callback free userdata*/
typedef void (*phqFree_f)(void* data);

typedef struct phqElement{
	size_t priority; /**< priority*/
	size_t index;    /**< index*/
	phqFree_f free;  /**< callback function*/
	void* data;      /**< userdata*/
}phqElement_s;

typedef struct phq{
	size_t size;            /**< size of elements*/
	size_t count;           /**< count elements*/
	size_t resize;          /**< resize*/
	phqCompare_f cmp;       /**< compare function*/
	phqElement_s** elements;/**< array of elements*/
}phq_s;

/** compare descend function*/
int phq_cmp_des(size_t a, size_t b);

/** compare ascend function*/
int phq_cmp_asc(size_t a, size_t b);

/** create new priority heap queue
 * @param size initial size
 * @param resize resize 
 * @param cmp compare function
 * @return phq or NULL, err is pushed errno is setted
 */
phq_s* phq_new(size_t size, size_t resize, phqCompare_f cmp);

/** free an element of queue
 * @param el element
 */
void phq_element_free(phqElement_s *el);

/** free a queue
 * @param q queue
 */
void phq_free(phq_s *q);

/** cleanup */
void phq_free_auto(phq_s** q);

/** cleanup*/
#define __phq_free __cleanup(phq_free_auto)

/** get size of array elements
 * @param q
 * @return size
 */
size_t phq_size(phq_s *q);

/** get count of array elements
 * @param q
 * @return count
 */
size_t phq_count(phq_s *q);

/** create a new element
 * @param priority priority of element
 * @param data userdata
 * @param pfree callback for free userdata
 * @return new element, or NULL for error
 */
phqElement_s* phq_element_new(size_t priority, void* data, phqFree_f pfree);

/** insert element in queue
 * @param q
 * @param el element
 * @return 0 successfull -1 error
 */
err_t phq_insert(phq_s *q, phqElement_s* el);

/** change priority of element in queue
 * @param q
 * @param newpri new priority
 * @param el element in a queue
 */
void phq_change_priority(phq_s *q, size_t newpri, phqElement_s* el);

/** remove element from queue, element is not free
 * @param q
 * @param el element to remove
 */
void phq_remove(phq_s *q, phqElement_s* el);

/** pop a element;
 * @param q
 * @return element or NULL if no element
 */
phqElement_s* phq_pop(phq_s *q);

/** get element without extract from queue
 * @param q
 * @return element or NULL if not have element
 */
phqElement_s* phq_peek(phq_s *q);

//void phq_dump(phq_s *q);

#endif
