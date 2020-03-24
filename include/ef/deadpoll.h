#ifndef __EF_DEADPOLL_H__
#define __EF_DEADPOLL_H__

#include <ef/type.h>
#include <ef/list.h>

/** an error is occured*/
#define DEADPOLL_ERROR -1
/** timeout */
#define DEADPOLL_TIMEOUT 1
/** event*/
#define DEADPOLL_EVENT 0

typedef struct deadpoll deadpoll_s;

/** callback function when raised events
 * @param 1 deadpoll
 * @param 2 event
 * @param 3 user data
 * @return 0 ok otherwise deadpoll_event return DEADPOLL_ERROR
 */
typedef err_t(*pollCbk_f)(deadpoll_s*, int, void*);

typedef struct pollEvent{
	int fd;
	int event;
	pollCbk_f callback;
	void* arg;
}pollEvent_s;

typedef struct deadpoll{
	pollEvent_s* events;
	int pollfd;
	size_t nevents;
	pollCbk_f timeout;
	void* timeoutarg;
}deadpoll_s;

/** unregister fd from epoll
 * @param dp deadpoll
 * @param fd fd to remove
 * @return 0 ok, -1 if fd not exists or error
 */
err_t deadpoll_unregister(deadpoll_s* dp, int fd);

/** register fd to epoll
 * @param dp deadpoll
 * @param fd fd to add
 * @param cbk callback function
 * @param arg userdata pass to callback
 * @param onevents  if 0 set EPOLLIN | EPOLLET | EPOLLPRI
 * @param cleanup called cleanup function with argument pollEvent_s, use this for if need to clean void* arg
 * @return 0 successfull -1 error
 */
err_t deadpoll_register(deadpoll_s* dp, int fd, pollCbk_f cbk, void* arg, int onevents, listFree_f cleanup);

/** register timeout function event
 * @param dp deadpoll
 * @param cbk callback function
 * @param arg userdata pass to callback
 */
void deadpoll_register_timeout(deadpoll_s* dp, pollCbk_f cbk, void* arg);

/** create new epoll
 * @return deadpoll or null for error
 */
deadpoll_s* deadpoll_new(void);

/** free epoll, release all resource of deadpoll
 * @param dp deadpoll
 */
void deadpoll_free(deadpoll_s* dp);

/** for cleanup
 * @see __cleanup
 */
void deadpoll_free_auto(deadpoll_s** dp);

/** for cleanup
 * @see __cleanup
 */
#define __deadpoll_free __cleanup(deadpoll_free_auto)

/** waiting for an events, call callback function if event occurred
 * @param dp deadpoll
 * @param timems if NULL is passed, deadpoll infinite wait event, same if timems == -1, if timems == 0 return immediately, if timems > 0 wait time, if time ellapsed execute timeout callback if exists and timems is setted to 0. if event is occurred timems is setted to remaining time
 * @return DEADPOLL_ERROR for error, DEADPOLL_EVENT if one or more events is parsed, DEADPOLL_TIMEOUT if timeout is generate
 */
int deadpoll_event(deadpoll_s* dp, long* timems);

/** infinite loop on event, timems -1 infinite wait*/
int deadpoll_loop(deadpoll_s* dp, long timems);

#endif
