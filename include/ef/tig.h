#ifndef __EF_TIG_H__
#define __EF_TIG_H__

#include <ef/type.h>

typedef struct tigProgress tigProgress_s;

/** progress callback */
typedef void (*tigProgress_f)(tigProgress_s* p);

typedef enum { 
	TIG_STATUS_FETCH,
	TIG_STATUS_CHECKOUT,
	TIG_STATUS_DELTA,
	TIG_STATUS_NEW,
	TIG_STATUS_UPDATE,
	TIG_STATUS_OBJECTS,
	TIG_STATUS_COMPRESS,
	TIG_STATUS_END
}tigStatusProgress_e;

typedef struct tigProgress{
	tigStatusProgress_e status; /**< status of progress */
	size_t num;                 /**< current value */
	size_t numTotal;            /**< total data */
	const char* str;            /**< string contains info */
	tigProgress_f progress;     /**< private */
	void* userdata;             /**< userdata for callback*/
}tigProgress_s;

/** before use tig call this*/
void tig_begin(void);

/** when end of use tig call this */
void tig_end(void);

/** get error message and klass
 * @param klass output klass
 * @return error message
 */
const char* tig_error_get(int* klass);

/** clone project 
 * @param url url where get project
 * @param path destination dir
 * @param progress optional progress callback
 * @param userdata optional userdata for callback
 * @return 0 successfull otherwise error
 */
err_t tig_clone(const char* url, const char* path, tigProgress_f progress, void* userdata);

/** pull project 
 * @param nothing optional, setted to 1 if nothig append otherwise 0
 * @param path of repo
 * @param origin origin
 * @param progress optional progress callback
 * @param userdata optional userdata for callback
 * @return 0 successfull otherwise error
 */
err_t tig_pull(int* nothing, const char* path, const char* origin, tigProgress_f progress, void* userdata);

#endif 
