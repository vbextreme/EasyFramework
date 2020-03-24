#ifndef __EF_HTTP_H__
#define __EF_HTTP_H__

#include <ef/type.h>
#include <ef/www.h>

#define HTTP_DOWNLOAD_BUFFER 4096

typedef struct httpPost{
	struct curl_httppost* post;
	struct curl_httppost* last;
}httpPost_s;

typedef struct http{
	www_s www;
	httpPost_s post;
}http_s;

/** free post, is auto called from http_delete*/
void http_post_free(http_s* http);

/** set post */
void http_post_set(http_s* http, const char* cmp, const char* value);

/** set post */
__printf(3,4) void http_post_setf(http_s* http, const char* cmp, const char* format, ...);

/** set file post*/
void http_post_file_set(http_s* http, const char* cmp, const char* value);

/** create new http
 * @param bufferupsize the upsize of www_buffer
 * @param flags WWW_FLAGS
 * @param prog set callback fro progress or NULL if not used
 * @return obj or NULL for error
 */
http_s* http_new(size_t bufferupsize, int flags, wwwProgress_s* prog);

/** free http */
void http_free(http_s* http);

/** perform an http get */
err_t http_get(http_s* http, const char* url);

/** perform an http post */
err_t http_post(http_s* http, const char* url);

/** download to a fd
 * @param fd destination
 * @param url ubication file
 * @param flags WWW_FLAGS, WWW_FLAGS_FD are automatic set, WWW_FLAGS_BODY & WWW_FLAGS_HEADER not have effect
 * @param prog set callback fro progress or NULL if not used
 * @param resume position to resume download, -1 no resume, assume fd is in a position
 * @return 0 successfull -1/-2 error
 */
err_t http_download(int fd, const char* url, int flags, wwwProgress_s* prog, ssize_t resume);

#endif 
