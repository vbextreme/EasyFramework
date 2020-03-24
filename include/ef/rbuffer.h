#ifndef __EF_RBUFFER_H__
#define __EF_RBUFFER_H__

#include <ef/type.h>

/** generic ring buffer*/
typedef struct rbuffer{
	size_t sof;       /**< sizeof type*/
	size_t size;      /**< size of memory*/
	size_t r;         /**< read id*/
	size_t w;         /**< write id*/
	void* element[0]; /**< memory */
}rbuffer_s;

/** cleanup function
 * @see __cleanup
 */
#define __rbuffer_free __cleanup(rbuffer_free_auto)

/** create new ring buffer
 * @param sof sizeof object
 * @param size size of memory
 * @return rbuffer or NULL for error
 */
rbuffer_s* rbuffer_new(size_t sof, size_t size);

/** free ring buffer
 * @param cb ring buffer
 */
void rbuffer_free(rbuffer_s* cb);

/** free for cleanup ring buffer
 * @param cb ring buffer
 * @see __cleanup
 */
void rbuffer_free_auto(rbuffer_s** cb);

/** chek if ring buffer is empty
 * @param cb ring buffer
 * @return 1 for empty 0 otherwise
 */
int rbuffer_isempty(rbuffer_s* cb);

/** chek if ring buffer is full, no more data can be write
 * @param cb ring buffer
 * @return 1 for full 0 otherwise
 */
int rbuffer_isfull(rbuffer_s* cb);

/** write data
 * @param cb ring buffer
 * @param data data with same size of rbuffer element
 * @return 0 ok -1 for error
 */
err_t rbuffer_write(rbuffer_s* cb, void* data);

/** read data
 * @param cb ring buffer
 * @param out out data with same size of rbuffer element
 * @return 0 ok -1 for error
 */
err_t rbuffer_read(rbuffer_s* cb, void* out);

/** available size for write
 * @param cb ring buffer
 * @return size
 */
size_t rbuffer_available_write(rbuffer_s* cb);

/** available size for read
 * @param cb ring buffer
 * @return size
 */
size_t rbuffer_available_read(rbuffer_s* cb);

/** available linear size for write with memcpy, for examples
 * @param cb ring buffer
 * @return size
 */
size_t rbuffer_available_linear_write(rbuffer_s* cb);

/** available linear size for read with memcpy, for examples
 * @param cb ring buffer
 * @return size
 */
size_t rbuffer_available_linear_read(rbuffer_s* cb);

/** sync w, call thin with value returned from linear size*/
void rbuffer_sync_write(rbuffer_s* cb, size_t n);

/** sync w, call thin with value returned from linear size*/
void rbuffer_sync_read(rbuffer_s* cb, size_t n);

/** push back readed element*/
err_t rbuffer_unread(rbuffer_s* cb);

/** puch back writed element*/
err_t rbuffer_unwrite(rbuffer_s* cb);

/** get address of r*/
void* rbuffer_addr_r(rbuffer_s* cb);

/** get address of w*/
void* rbuffer_addr_w(rbuffer_s* cb);

#endif 
