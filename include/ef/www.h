#ifndef __EF_WWW_H__
#define __EF_WWW_H__

#include <ef/type.h>
#include <curl/curl.h> 

/** secure connection flags*/
#define WWW_FLAGS_SSL      0x01
/** reply in json format*/
#define WWW_FLAGS_APP_JSON 0x02
/** get body as data */
#define WWW_FLAGS_BODY     0x04
/** get header as data */
#define WWW_FLAGS_HEADER   0x08
/** follow location */
#define WWW_FLAGS_FOLLOW   0x10
/** write on fd */
#define WWW_FLAGS_FD       0x20
/** read on buffers */
#define WWW_FLAGS_SENDS    0x40
/** read on fd */
#define WWW_FLAGS_SEND_FD  0x80


/** enable www, call before use anyof www function */
void www_begin(void);

/** terminate www session */
void www_end(void);

#define www_errno_str() curl_easy_strerror(errno)

typedef struct wwwBuffer_s{
	char* buf;
	size_t size;
	size_t len;
	size_t offset;
	size_t upto;
}wwwBuffer_s;

/** for static initialization */
#define WWW_BUFFER_NEW(UPSIZE) { .buf = NULL, .size = 0, .len = 0, .upto = UPSIZE }

/** init buffer, same of BUFFER_NEW */
void www_buffer_init(wwwBuffer_s* buf, size_t upto);

/** delete buffer */
void www_buffer_delete(wwwBuffer_s* buf);

/**cleanup function*/
#define __www_buffer_delete __cleanup(www_buffer_delete)

/** increase size of buffer*/
void www_buffer_upsize(wwwBuffer_s* buf, size_t addsize);

typedef void(*wwwprogress_f)(const char* name, double current, double total, double speed);

typedef struct wwwProgress{
	wwwprogress_f upload;   /**< callback upload*/
	wwwprogress_f download; /**< callback dowload*/
	const char* name;       /**< name passed to callback */
	double time;            /**< private remember to set 0 */
	double down;            /**< private remember to set 0 */
	double up;              /**< private remember to set 0 */
}wwwProgress_s;

typedef struct www{
	CURL* h;
	wwwBuffer_s header;
	wwwBuffer_s body;
	struct curl_slist* hlist;
	char* customRQ;
}www_s;

/** set ssl*/
void www_set_ssl(www_s* www, int flags);

/** set reply as json*/
void www_set_app_json(www_s* www, int flags);

/** set recv mode*/
void www_set_recv_mode(www_s* www, int flags );

/** set send mode*/
void www_set_send_mode(www_s* www, int flags );

/** set follow*/
void www_set_follow(www_s* www, int flags );

/** set progress*/
void www_set_progress(www_s* www, wwwProgress_s* prog);

/** init www object*/
err_t www_init(www_s* www, size_t bufferupsize, int flags, wwwProgress_s* prog);

/** delete www object*/
void www_delete(www_s* www);

/** cleanup*/
#define __www_delete __cleanup(www_delete)

/** curl cleanup */
#define __www_escape_free __cleanup(www_curl_free)
void www_curl_free(void* mem);

/** escape data in uri */
#define www_uri_escape(WWW, STR, LEN) curl_easy_escape((WWW)->h, STR, LEN)

/** url set*/
void www_url_set(www_s* www, const char* url);

/** fd set*/
void www_fd_set(www_s* www, int* fd);

/** send fd set*/
void www_send_fd_set(www_s* www, int* fd);

/** set buffer to send*/
void www_send_set(www_s* www, wwwBuffer_s* buf);

/** set post*/
void www_post_set(www_s* www, struct curl_httppost* post);

/** set user/pass*/
void www_user_pass_set(www_s* www, const char* user, const char* psw);

/** create custom request*/
__printf(2,3) void www_custom_reqest(www_s* www, const char* format, ...);

/** add in header*/
void www_header_list_append(www_s* www, const char* str);

/** delete header*/
void www_header_list_delete(www_s* www);

/** resume mode*/
void www_resume(www_s* www, ssize_t resume);

/** performe request*/
err_t www_perform(www_s* www, int retcode);

/** dump buffer */
void www_dump_buffers(www_s* www);

#endif 
