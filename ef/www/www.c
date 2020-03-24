#include <ef/www.h>
#include <ef/memory.h>
#include <ef/mth.h>
#include <ef/err.h>
#include <ef/delay.h>
#include <ef/str.h>

#include <stdarg.h>

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/************************************ WWW OBJECT ******************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

__private struct curl_slist *HAPPJSON = NULL;

void www_begin(void){
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl_slist_append(HAPPJSON, "Accept: application/json");
	curl_slist_append(HAPPJSON, "Content-Type: application/json");
	curl_slist_append(HAPPJSON, "charsets: utf-8");
}

void www_end(void){
	curl_slist_free_all(HAPPJSON);
	curl_global_cleanup();
}

__private int www_curl_progress(void* userp, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded){
	wwwProgress_s* prog = userp;
	const double time = time_dbls();
	const size_t ellapseTime = time - prog->time;
	const double advDownload = NowDownloaded - prog->down;
	const double advUpload = NowUploaded - prog->up;
	double speeddw = 0.0;
	double speedup = 0.0;

	if( prog->time > 0.0 ){
		if( advDownload > 0.0 && ellapseTime > 0.0 ) speeddw = advDownload / ellapseTime;
		if( advDownload > 0.0 && ellapseTime > 0.0 ) speedup = advDownload / ellapseTime;
	}

	if( prog->download && advDownload > 0.0 ) prog->download(prog->name, NowDownloaded, TotalToDownload, speeddw);
	if( prog->upload && advUpload > 0.0 ) prog->upload(prog->name, NowUploaded, TotalToUpload, speedup);

	prog->time = time;
	prog->down = advDownload;
	prog->up = advUpload;

	return 0;
}

__private size_t www_curl_buffer_recv(void* ptr, size_t size, size_t nmemb, void* userp){
	wwwBuffer_s* buf = (wwwBuffer_s*)userp;
	size_t maxs = size * nmemb;
	
	www_buffer_upsize(buf, maxs);
	memcpy(&buf->buf[buf->len], ptr, maxs);
	buf->len += maxs;
	return maxs;
}

__private size_t www_curl_file_recv(void* ptr, size_t size, size_t nmemb, void* userp) {
	int* f = (int*)userp;
	size_t ret = write(*f, ptr, size * nmemb);
	return ret;
}

__private size_t www_curl_buffer_send(void* ptr, size_t size, size_t nmemb, void* userp){
	wwwBuffer_s* buf = (wwwBuffer_s*)userp;
	size_t maxs = size * nmemb;
	if( !maxs ) return 0;
	if( maxs > buf->len - buf->offset ){
		maxs = buf->len - buf->offset;
	}	
	memcpy(&buf->buf[buf->offset], ptr, maxs);
	buf->offset += maxs;
	return maxs;
}

__private size_t www_curl_file_send(void* ptr, size_t size, size_t nmemb, void* userp) {
	int* f = (int*)userp;
	size_t ret = read(*f, ptr, size * nmemb);
	return ret;
}

void www_set_ssl(www_s* www, int flags){
	if ( flags & WWW_FLAGS_SSL ){
		dbg_info("enable ssl");
		curl_easy_setopt(www->h, CURLOPT_SSL_VERIFYPEER, 1L);
	}
	else{
		curl_easy_setopt(www->h, CURLOPT_SSL_VERIFYPEER, 0L);
	}
}

void www_set_app_json(www_s* www, int flags){
	if( flags & WWW_FLAGS_APP_JSON ){
		dbg_info("enable appjson");
		curl_easy_setopt(www->h, CURLOPT_HTTPHEADER, HAPPJSON);
	}
}

void www_set_recv_mode(www_s* www, int flags ){
	if( flags & WWW_FLAGS_FD ){
		dbg_info("recv fd");
		curl_easy_setopt(www->h, CURLOPT_WRITEFUNCTION, www_curl_file_recv);
	}
	else if( flags & WWW_FLAGS_BODY || flags & WWW_FLAGS_HEADER ){
		curl_easy_setopt(www->h, CURLOPT_WRITEFUNCTION, www_curl_buffer_recv);
		if( flags & WWW_FLAGS_BODY ){
			dbg_info("recv body");
			curl_easy_setopt(www->h, CURLOPT_WRITEDATA, &www->body);
		}
		if( flags & WWW_FLAGS_HEADER ){
			dbg_info("recv header");
			curl_easy_setopt(www->h, CURLOPT_HEADERDATA, &www->header);
		}
	}
}

void www_set_send_mode(www_s* www, int flags ){
	if( flags & WWW_FLAGS_SEND_FD ){
		dbg_info("send fd");
		curl_easy_setopt(www->h, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(www->h, CURLOPT_READFUNCTION, www_curl_file_send);
	}
	else if( flags & WWW_FLAGS_SENDS ){
		curl_easy_setopt(www->h, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(www->h, CURLOPT_READFUNCTION, www_curl_buffer_send);
	}
	else{
		curl_easy_setopt(www->h, CURLOPT_UPLOAD, 0L);
	}
}

void www_set_follow(www_s* www, int flags ){
	if( flags & WWW_FLAGS_FOLLOW ){
		dbg_info("follow");
		curl_easy_setopt(www->h, CURLOPT_FOLLOWLOCATION, 1L);
	}
}

void www_set_progress(www_s* www, wwwProgress_s* prog){
	if( prog == NULL ){
		curl_easy_setopt(www->h, CURLOPT_NOPROGRESS, TRUE);
	}
	else{
		dbg_info("set progress");
		curl_easy_setopt(www->h, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(www->h, CURLOPT_PROGRESSFUNCTION, www_curl_progress);
		curl_easy_setopt(www->h, CURLOPT_PROGRESSDATA, prog);
	}
}

err_t www_init(www_s* www, size_t bufferupsize, int flags, wwwProgress_s* prog){
	www_buffer_init(&www->body, bufferupsize);
	www_buffer_init(&www->header, bufferupsize);
	www->customRQ = NULL;
	www->hlist = NULL;

	if( !(www->h = curl_easy_init()) ){
		err_push("curl init(%d): %s", errno, www_errno_str());
		return -1;
	}

	www_set_ssl(www, flags);
	www_set_app_json(www, flags);	
	www_set_recv_mode(www, flags);
	www_set_send_mode(www, flags);
	www_set_follow(www, flags);
	www_set_progress(www, prog);

	curl_easy_setopt(www->h, CURLOPT_NOSIGNAL, 1L);	
	curl_easy_setopt(www->h, CURLOPT_VERBOSE, 0L);	

	return 0;
}

void www_delete(www_s* www){
	if( www->hlist ) curl_slist_free_all(www->hlist);
	if( www->h ) curl_easy_cleanup(www->h);
	www_buffer_delete(&www->body);
	www_buffer_delete(&www->header);
	if( www->customRQ ) free(www->customRQ);
}

void www_url_set(www_s* www, const char* url){
	dbg_info("set url:%s",url);
	curl_easy_setopt(www->h, CURLOPT_URL, url);
}

void www_fd_set(www_s* www, int* fd){
	 curl_easy_setopt(www->h, CURLOPT_WRITEDATA, fd);
}

void www_send_fd_set(www_s* www, int* fd){
	 curl_easy_setopt(www->h, CURLOPT_READDATA, fd);
}

void www_send_set(www_s* www, wwwBuffer_s* buf){
	buf->offset = 0;
	curl_easy_setopt(www->h, CURLOPT_READDATA, buf);
}

void www_post_set(www_s* www, struct curl_httppost* post){
	curl_easy_setopt(www->h, CURLOPT_HTTPPOST, post);
}

void www_user_pass_set(www_s* www, const char* user, const char* psw){
	dbg_info("user:%s pass:<*>", user);
	if ( user ) curl_easy_setopt(www->h, CURLOPT_USERNAME, user);
	if ( psw ) curl_easy_setopt(www->h, CURLOPT_PASSWORD, psw);
}

void www_custom_reqest(www_s* www, const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	
	if( www->customRQ ) free(www->customRQ);
	www->customRQ = str_vprintf(format, va1, va2);
	if( www->customRQ ){
		dbg_info("custom request:%s", www->customRQ);
		curl_easy_setopt(www->h, CURLOPT_CUSTOMREQUEST, www->customRQ);
	}	

	va_end(va1);
	va_end(va2);
}

void www_header_list_append(www_s* www, const char* str){
	curl_slist_append(www->hlist, str);
}

void www_header_list_delete(www_s* www){
	if( www->hlist ) curl_slist_free_all(www->hlist);
	www->hlist = NULL;
}

void www_resume(www_s* www, ssize_t resume){
	if( resume != -1 ){
		 curl_easy_setopt(www->h, CURLOPT_RESUME_FROM , resume);
	}
}

err_t www_perform(www_s* www, int retcode){
	CURLcode res;
	res = curl_easy_perform(www->h);

	if ( res != CURLE_OK && res != CURLE_FTP_COULDNT_RETR_FILE ){
		err_push("curl perform return(%d): %s", res, curl_easy_strerror(res));
		errno = res;
		return -1;
	}
	
	long resCode;
	curl_easy_getinfo(www->h, CURLINFO_RESPONSE_CODE, &resCode);
    if( !retcode && resCode != 200L && resCode != 0 ) {
        err_push("response code(%ld): %s", resCode, curl_easy_strerror(res)); 
		errno = resCode;
        return -2;
    }
	
	if( www->body.len > 0 ){
		www_buffer_upsize(&www->body, 1);
		www->body.buf[www->body.len] = 0;
	}
	if( www->header.len > 0 ){
		www_buffer_upsize(&www->header, 1);
		www->header.buf[www->header.len] = 0;
	}

	return retcode ? resCode : 0;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/********************************* WWW BUFFER OBJECT **************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

void www_buffer_init(wwwBuffer_s* buf, size_t upto){
	buf->buf = NULL;
	buf->len = 0;
	buf->size = 0;
	buf->offset = 0;
	buf->upto = upto;
}

void www_buffer_delete(wwwBuffer_s* buf){
	free(buf->buf);
	buf->buf = NULL;
	buf->len = 0;
	buf->size = 0;
}

void www_buffer_upsize(wwwBuffer_s* buf, size_t addsize){
	if( buf->buf == NULL ){
		buf->buf = mem_many(char, addsize);
		iassert(buf->buf);
		buf->size = addsize;
	}
	else if( buf->size - buf->len < addsize ){
		size_t inc = addsize - (buf->size - buf->len);
		addsize = ROUND_UP(inc, buf->upto);
		buf->buf = realloc(buf->buf, sizeof(char) * (addsize + buf->size));
		if( !buf->buf ) err_fail("realloc");
		buf->size += addsize;
	}
}

void www_dump_buffers(www_s* www){
	printf("<HEADER>%s</HEADER>\n", www->header.buf);
	printf("<BODY>%s</BODY>\n", www->body.buf);
}

void www_curl_free(void* mem){
	void** addr = (void**)mem;
	if( *addr ){
		curl_free(*addr);
	}
}
