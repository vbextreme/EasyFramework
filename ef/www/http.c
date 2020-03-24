#include <ef/http.h>
#include <ef/memory.h>
#include <ef/mth.h>
#include <ef/err.h>
#include <ef/str.h>
#include <stdarg.h>

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/*********************************** HTTP OBJECT ******************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

http_s* http_new(size_t bufferupsize, int flags, wwwProgress_s* prog){
	http_s* http = mem_new(http_s);
	if( !http ){
		err_pushno("malloc");
		return NULL;
	}

	http->post.last = NULL;
	http->post.post = NULL;

	if( www_init(&http->www, bufferupsize, flags, prog) ){
		free(http);
		return NULL;
	}
		
	return http;
}

void http_free(http_s* http){
	http_post_free(http);
	www_delete(&http->www);
	free(http);
}

err_t http_get(http_s* http, const char* url){
	www_url_set(&http->www, url);
	return www_perform(&http->www, 0);
}

err_t http_post(http_s* http, const char* url){
	www_url_set(&http->www, url);
	www_post_set(&http->www, http->post.post);
	return www_perform(&http->www, 0);
}

void http_post_free(http_s* http){
	if( http->post.post ){	
		curl_formfree(http->post.post); 
		http->post.post = http->post.last = NULL; 
	}
}

void http_post_set(http_s* http, const char* cmp, const char* value){
	curl_formadd( &http->post.post, &http->post.last, CURLFORM_COPYNAME, cmp, CURLFORM_COPYCONTENTS, value, CURLFORM_END);
}

void http_post_setf(http_s* http, const char* cmp, const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	
	__mem_free char* val = str_vprintf(format, va1, va2);
	if( val ){
		curl_formadd( &http->post.post, &http->post.last, CURLFORM_COPYNAME, cmp, CURLFORM_COPYCONTENTS, val, CURLFORM_END);
	}	

	va_end(va1);
	va_end(va2);
}

void http_post_file_set(http_s* http, const char* cmp, const char* value){
	curl_formadd( &http->post.post, &http->post.last, CURLFORM_COPYNAME, cmp, CURLFORM_FILE, value, CURLFORM_END);
}

err_t http_download(int fd, const char* url, int flags, wwwProgress_s* prog, ssize_t resume){
	__www_delete www_s www;
	flags |= WWW_FLAGS_FD;
	www_init(&www, HTTP_DOWNLOAD_BUFFER, flags , prog );
	www_fd_set(&www, &fd);
	www_url_set(&www, url);
	www_resume(&www, resume);
	return www_perform(&www, 0);
}


