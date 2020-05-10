#include <ef/ftp.h>
#include <ef/err.h>
#include <ef/str.h>
#include <ef/file.h>
#include <ef/memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>
#include <sys/stat.h>

ftp_s* ftp_new(size_t bufferupsize, int flags, wwwProgress_s* prog){
	ftp_s* ftp = mem_new(ftp_s);
	if( !ftp ){
		err_pushno("malloc");
		return NULL;
	}
	flags |= WWW_FLAGS_FOLLOW | WWW_FLAGS_BODY | WWW_FLAGS_HEADER;
	ftp->site = NULL;
	if( www_init(&ftp->www, bufferupsize, flags, prog) ){
		free(ftp);
		return NULL;
	}
	return ftp;
}

void ftp_free(ftp_s* ftp){
	if( ftp->site ) free(ftp->site);
	www_delete(&ftp->www);
	free(ftp);
}

void ftp_user_password(ftp_s* ftp, const char* user, const char* pass){
	www_user_pass_set(&ftp->www, user, pass);
}

err_t ftp_site(ftp_s* ftp, const char* url){
	if( ftp->site ) free(ftp->site);

	size_t len = strlen(url);
	if( url[len-1] != '/' ){
		ftp->site = str_printf("%s/", url);
	}
	else{	
		ftp->site = str_dup(url, 0);
	}
	ftp->lenSite = strlen(ftp->site);
	
	dbg_info("site:%s",ftp->site);
	return 0;
}

void ftp_cd(ftp_s* ftp, const char* path){
	dbg_info("");
	dbg_info("curretn:%s",&ftp->site[ftp->lenSite-1]);
	__mem_free char* res = path_resolve_custom(path, NULL, &ftp->site[ftp->lenSite-1]);
	dbg_info("pathresolve:%s",res);
	iassert(res);
	size_t len = strlen(res);
	char* newsite;
	if( res[len-1] == '/' ){
		newsite = str_printf("%.*s%s",(int)ftp->lenSite-1, ftp->site, res);
	}
	else{
		newsite = str_printf("%.*s%s/",(int)ftp->lenSite-1, ftp->site, res);
	}
	iassert(newsite);
	free(ftp->site);
	ftp->site = newsite;
	dbg_info("cwd:%s",ftp->site);
}

__private int ftp_list_priv(const char* line){
	int ret = 0;
	switch( *line++ ){
		case 'r': ret |= S_IROTH;
		case '-': break;
		default: return -1;
	}
	switch( *line++ ){
		case 'w': ret |= S_IWOTH;
		case '-': break;
		default: return -1;
	}
	switch( *line ){
		case 'x': ret |= S_IXOTH;
		case '-': break;
		default: return -1;
	}
	return ret;
}

__private const char* ftp_list_skip_numeric(const char* line){
	while( *line && (*line != ' ' && *line != '\t') ) ++line;
	return str_skip_h(line);
}

__private int ftp_list_month_to_n(const char* line){
	static char* month[] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
		NULL
	};
	for( int i = 0; month[i]; ++i){
		if( !strncmp(line,month[i],3) ) return i;
	}
	return -1;
}

__private const char* ftp_list_parse_line(ftpStat_s* out, const char* line){
	int tp;
	out->name = NULL;
	out->link = NULL;

	switch( *line++ ){
		case 'l': out->mode = S_IFLNK; break;
		case 'd': out->mode = S_IFDIR; break;
		case '-': out->mode = S_IFREG; break;
		default: return NULL;
	}
	
	tp = ftp_list_priv(line);
	if( tp == -1 ) return NULL;
	out->mode |= tp << 8;
	line += 3;

	tp = ftp_list_priv(line);
	if( tp == -1 ) return NULL;
	out->mode |= tp << 4;
	line += 3;

	tp = ftp_list_priv(line);
	if( tp == -1 ) return NULL;
	out->mode |= tp;
	line += 3;

	line = str_skip_h(line);
	line = ftp_list_skip_numeric(line);
	line = ftp_list_skip_numeric(line);
	line = ftp_list_skip_numeric(line);

	errno = 0;
	char* en;
	out->size = strtoul(line, &en, 10);
	if( !en || errno ) return NULL;
	line = en;
	line = str_skip_h(line);
	
	int month = ftp_list_month_to_n(line);
	if( month == -1 ) return NULL;
	line += 3;
	line = str_skip_h(line);

	int day = strtoul(line, &en, 10);
	if( !en || errno ) return NULL;
	line = en;
	line = str_skip_h(line);

	int year = 0;
	int hh = 0;
	int mm = 0;
	if( line[1] == ':' || line[2] == ':' ){
		time_t now = time(NULL);
		struct tm* tm = localtime(&now);
		year = tm->tm_year + 1900;
		hh = strtoul(line, &en, 10);
		if( !en || errno ) return NULL;
		line = en+1;
		mm = strtoul(line, &en, 10);
		if( !en || errno ) return NULL;
		line = en;

	}
	else{
		year = strtoul(line, &en, 10);
		if( !en || errno ) return NULL;
		line = en;
	}
	line = str_skip_h(line);

	struct tm tm = {0};
	tm.tm_year = year - 1900;
	tm.tm_mon  = month;
	tm.tm_mday = day;
	tm.tm_min = mm;
	tm.tm_hour = hh;
	out->time = mktime(&tm);
	
	if( S_ISLNK(out->mode) ){
		const char* begin = line;
		while( *line && strncmp(line, " -> ", 4) ) ++line;
		out->name = str_dup(begin, line-begin);
		line += 4;
		begin = line;
		while( *line && *line != '\n' ) ++line;
		out->link = str_dup(begin, line-begin);
	}
	else{
		const char* begin = line;
		while( *line && *line != '\n' ) ++line;
		out->name = str_dup(begin, line-begin);
		out->link = NULL;
	}
	++line;
	return line;
}

__private ftpStat_s* ftp_gnu_parse(const char* line){
	ftpStat_s* ret = vector_new(ftpStat_s, 8, ftp_stat_free);
	if( !ret ) return NULL;

	while( *line ){
		ftpStat_s* fs = vector_get_push_back(ret);
		line = ftp_list_parse_line(fs, line);
		if( line == NULL ){
			dbg_error("line == NULL");
			vector_free(ret);
			return NULL;
		}
	}
	return ret;
}

ftpStat_s* ftp_list(ftp_s* ftp){
	www_custom_reqest(&ftp->www, "LIST");
	www_url_set(&ftp->www, ftp->site);

	int res = www_perform(&ftp->www, 1);

	if( res < 200 || res > 299 ){
		err_push("ftp result:%d", res);
		return NULL;
	}

	//printf("<HEADER>%s</HEADER>\n", ftp->www.header.buf);
	//printf("<BODY>%s</BODY>\n", ftp->www.body.buf);

	if ( !strncmp(ftp->www.header.buf,"220 Microsoft",13) ){
		return NULL;
	}
	else if ( !strncmp(ftp->www.header.buf,"220 GNU",7) ){
		return ftp_gnu_parse(ftp->www.body.buf);
	}
	
	return ftp_gnu_parse(ftp->www.body.buf);
}

void ftp_stat_free(void* arg){
	ftpStat_s* v = arg;
	if( v->name ) free(v->name);
	if( v->link ) free(v->link);
}

err_t ftp_rename(ftp_s* ftp, const char* from, const char* to){
	www_custom_reqest(&ftp->www, "LIST");
	www_url_set(&ftp->www, ftp->site);

	__mem_free char* fromr = path_resolve_custom(from, NULL, &ftp->site[ftp->lenSite-1]);
	__mem_free char* rnfr  = str_printf("RNFR %.*s%s", (int)ftp->lenSite-1, ftp->site, fromr);
	__mem_free char* tor   = path_resolve_custom(to, NULL, &ftp->site[ftp->lenSite-1]);
	__mem_free char* rnto  = str_printf("RNTO %.*s%s", (int)ftp->lenSite-1, ftp->site, tor);
	www_header_list_append(&ftp->www, rnfr);
	www_header_list_append(&ftp->www, rnto);

	int res = www_perform(&ftp->www, 1);

	if( res < 200 || res > 299 ){
		err_push("ftp result:%d", res);
		www_header_list_delete(&ftp->www);
		return -1;
	}

	www_header_list_delete(&ftp->www);
    return 0;
}

err_t ftp_unlink(ftp_s* ftp, const char* fname){
	__mem_free char* fnamer = path_resolve_custom(fname, NULL, &ftp->site[ftp->lenSite-1]);
	www_custom_reqest(&ftp->www, "DELETE %s", fnamer);
	www_url_set(&ftp->www, ftp->site);

	int res = www_perform(&ftp->www, 1);

	if( res < 200 || res > 299 ){
		err_push("ftp result:%d", res);
		return -1;
	}
	
	return 0;
}

err_t ftp_mkdir(ftp_s* ftp, const char* name){
	__mem_free char* namer = path_resolve_custom(name, NULL, &ftp->site[ftp->lenSite-1]);
	www_custom_reqest(&ftp->www, "MKD %s", namer);
	www_url_set(&ftp->www, ftp->site);

	int res = www_perform(&ftp->www, 1);

	if( res < 200 || res > 299 ){
		err_push("ftp result:%d", res);
		return -1;
	}
	
	return 0;
}

err_t ftp_rmdir(ftp_s* ftp, const char* name){
	__mem_free char* namer = path_resolve_custom(name, NULL, &ftp->site[ftp->lenSite-1]);
	www_custom_reqest(&ftp->www, "RMD %s", namer);
	www_url_set(&ftp->www, ftp->site);

	int res = www_perform(&ftp->www, 1);

	if( res < 200 || res > 299 ){
		err_push("ftp result:%d", res);
		return -1;
	}
	
	return 0;
}

err_t ftp_download(int fd, ftp_s* ftp, const char* fname, ssize_t resume){
	www_set_recv_mode(&ftp->www, WWW_FLAGS_FD);
	www_fd_set(&ftp->www, &fd);
	www_resume(&ftp->www, resume);
	
	__mem_free char* fnamer = path_resolve_custom(fname, NULL, &ftp->site[ftp->lenSite-1]);
	www_url_set(&ftp->www, fnamer);

	int res = www_perform(&ftp->www, 1);

	if( res < 200 || res > 299 ){
		www_set_recv_mode(&ftp->www,  WWW_FLAGS_BODY | WWW_FLAGS_HEADER);
		err_push("ftp result:%d", res);
		return -1;
	}
	
	www_set_recv_mode(&ftp->www,  WWW_FLAGS_BODY | WWW_FLAGS_HEADER);
	return 0;
}

err_t ftp_upload(ftp_s* ftp, int fd, const char* fname){
	www_set_send_mode(&ftp->www, WWW_FLAGS_SEND_FD);
	www_send_fd_set(&ftp->www, &fd);
	
	__mem_free char* fnamer = path_resolve_custom(fname, NULL, &ftp->site[ftp->lenSite-1]);
	www_url_set(&ftp->www, fnamer);

	int res = www_perform(&ftp->www, 1);

	if( res < 200 || res > 299 ){
		err_push("ftp result:%d", res);
		www_send_set(&ftp->www, 0);
		return -1;
	}

	www_send_set(&ftp->www, 0);
	return 0;	
}

