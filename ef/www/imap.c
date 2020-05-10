#include <ef/imap.h>
#include <ef/str.h>
#include <ef/err.h>
#include <ef/mth.h>

/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////// EMAIL ///////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// ///////////////////////////////////////////////////////////////// ///
/// "imap://imap.example.com"

imap_s* imap_new(size_t bufferupsize, int flags, wwwProgress_s* prog){
	imap_s* ima = mem_new(imap_s);
	if( !ima ){
		err_pushno("malloc");
		return NULL;
	}
	flags |= WWW_FLAGS_BODY | WWW_FLAGS_HEADER;
	ima->server = NULL;
	if( www_init(&ima->www, bufferupsize, flags, prog) ){
		free(ima);
		return NULL;
	}
	return ima;
}

void imap_free(imap_s* ima){
	if( ima->server ) free(ima->server);
	www_delete(&ima->www);
	free(ima);
}

void imap_email_password(imap_s* ima, const char* email, const char* pass){
	www_user_pass_set(&ima->www, email, pass);
}

err_t imap_server(imap_s* ima, const char* server){
	if( ima->server ) free(ima->server);

	size_t len = strlen(server);
	ima->server = str_dup(server, 0);
	ima->lenServer = len;
	www_url_set(&ima->www, ima->server);

	//dbg_info("email:%s",ima->server);
	return 0;
}

__private char** imap_list_parse(const char* body){
	char** ret = vector_new(char*, 8, free);
	
	while( *body ){
		while( *body && *body != '*' ){
			body = str_next_line(body);
		}
		body = str_chr(body, '\"');
		if( !*body ) break;
		++body;
		if( *body != '/' ){
			vector_free(ret);
			return NULL;
		}
		++body;
		if( *body != '"' ){
			vector_free(ret);
			return NULL;
		}
		body = str_skip_h(body+1);
		vector_push_back(ret, str_dup_ch(body, '\r'));
	}
	return ret;
}

char** imap_ls(imap_s* ima){
	if( www_perform(&ima->www, 0) ) return NULL;
	return imap_list_parse(ima->www.body.buf);
}

err_t imap_examine(imapExamine_s* out, imap_s* ima, const char* dir){
	www_custom_reqest(&ima->www, "EXAMINE %s", dir);
	if( www_perform(&ima->www, 0) ) return -1;
	//printf("<HEADER>%s</HEADER>\n<BODY>%s</BODY>\n<CODE %d>\n", ima->www.header.buf, ima->www.body.buf,0);

	const char* parse = strstr(ima->www.body.buf, "EXISTS");
	if( !parse ){
		//dbg_error("not find exists");
		goto ONERR;
	}
	//dbg_info("<PARSE>%s</PARSE>", parse);

	while( parse > ima->www.body.buf && *parse != '*' ) --parse;
	if( parse == ima->www.body.buf ){
		//dbg_error("line exists not start with *");
		goto ONERR;
	}
	++parse;
	parse = str_skip_h(parse);
	out->exists = strtoul(parse, NULL, 10);

	parse = strstr(parse, "RECENT");
	if( !parse ){
		//dbg_error("not find recent");
		goto ONERR;
	}
	while( parse > ima->www.body.buf && *parse != '*' ) --parse;
	if( parse == ima->www.body.buf ){
		//dbg_error("line recent not start with *");
		goto ONERR;
	}
	++parse;
	parse = str_skip_h(parse);
	out->recent = strtoul(parse, NULL, 10);

	parse = strstr(parse, "UNSEEN");
	if( !parse ){
		//dbg_error("not find unseen");
		goto ONERR;
	}
	parse += strlen("UNSEEN");
	parse = str_skip_h(parse);
	out->unseeid = strtoul(parse, NULL, 10);

	return 0;
ONERR:
	err_push("invalid body");
	return -1;
}

__private imapFetchFlags_e imap_fetch_to_flag(const char* flag){
	static const char* flags[] = {
		"Seen",
		"Answered",
		"Flagged",
		"Deleted",
	   	"Draft",
		NULL
	};
	for(size_t i = 0; flags[i]; ++i){
		if( !strncmp(flag, flags[i], strlen(flags[i])) ){
			return i;
		}
	}
	return IMAP_FETCH_FLAGS_ERROR;
}

__private char* imap_fetch_next_line(char* line){
	while( (line=strchr(line, '\r')) && *(line+1) != '\n' );
	return line ? line+2 : NULL;
}

imapFetch_s* imap_fetch(imap_s* ima, const char* dir, int nobody, size_t from, size_t to){
	__mem_free char* url = str_printf("%s/%s/", ima->server, dir);
	www_url_set(&ima->www, url);
	if( nobody ){
		www_custom_reqest(&ima->www, "FETCH %lu:%lu BODY[HEADER]", from, to);
	}
	else{
		www_custom_reqest(&ima->www, "FETCH %lu:%lu BODY[]", from, to);
	}

	if( www_perform(&ima->www, 0) ){
		www_url_set(&ima->www, url);
		return NULL;
	}
	www_url_set(&ima->www, ima->server);

	//printf("<HEADER>%s</HEADER>\n<BODY>%s</BODY>\n<CODE %d>\n", ima->www.header.buf, ima->www.body.buf,0);

	size_t count = (to - from) + 1;
	imapFetch_s* fetch = mem_many(imapFetch_s, count);
	if( !fetch ){
		err_push("eom");
		return NULL;
	}
	char* parse = ima->www.header.buf;

	for( size_t i = from, id = 0; i <= to && id < count; ++i, ++id ){
		//dbg_info("parse %lu", id);

		char find[1024];
		char* next;

		fetch[id].id = i;
		sprintf(find, "* %lu FETCH (", i);
		
		//dbg_info("find begin email:%s", find);
		parse = strstr(parse, find);
		if( !parse ) goto ONERR;

		parse = strchr(parse, '{');
		if( !parse ) goto ONERR;
		++parse;
		fetch[id].size = strtoul(parse, &next, 10);
		if( !next ) goto ONERR;
		dbg_info("size of email:%lu", fetch[id].size);
		
		parse = imap_fetch_next_line(next);
		if( !parse ) goto ONERR;
		fetch[id].mime = parse;
		
		while( parse ){
			//dbg_info("search empty line '%.16s'", parse);
			while( parse && str_ancmp(parse, " FLAGS (") ){
				parse = imap_fetch_next_line(parse);
				//dbg_info("next line '%.32s'", parse);
			}
			if( !parse ) goto ONERR;
			//dbg_info("end mime");
			*parse = 0;
			++parse;
			
			//dbg_info("set flag");
			parse = strchr(parse, '\\');
			if( !parse ) goto ONERR;
			++parse;

			fetch[id].flag = imap_fetch_to_flag(parse);

			parse = imap_fetch_next_line(parse);	
			break;
		}
	}
	//dbg_info("fetch complete");
	return fetch;

ONERR:
	err_push("wrong fetch id");
	free(fetch);
	return NULL;
}

typedef struct mmph{
	const char* name;
	void* dup;
	int type;
}mmph_s;

#define MIME_HEADER_COUNT 9

__private const char* mime_charset(int* out, const char* line){
	if( strncmp(line, "=?", 2) ){
		//dbg_info("ascii encoded");
		*out = 0;
		return NULL;
	}
	line+=2;
	//skip charset
	*out = 1;
	//dbg_info("charset encoded");
	line = strchr(line, '?');
	if( !line ){
		err_push("no charset");
		return NULL;
	}
	return line+1;
}

__private const char* mime_encoding(int* out, const char* line){
	switch( *line ){
		case 'Q': *out = 0; break;
		case 'B': *out = 1; break;
		default: err_push("invalid encoding"); return NULL;
	}
	++line;
	if( *line != '?' ){
		err_push("invalid end encoding");
		return NULL;
	}
	return line + 1;
}

__private const char* mime_unfolding_charset(char** out, const char* line){
	const char* end = strstr(line, "?=");
	if( end == NULL ){
		err_push("unterminated encoded");
		return NULL;
	}
	size_t count = end-line;
	end += 2;

	char* str = mem_many(char, count + 1);
	if( !str ){
		err_fail("eom");
	}

	memcpy(str, line, count);
	str[count] = 0;
	*out = str;

	return end;
}

__private const char* mime_encoded_word(char** out, size_t* size, const char* line){
	int charset;
	int encoding;
	if( !(line = mime_charset(&charset, line)) ) return NULL;
	if( !charset ) return NULL;
	
	if( !(line = mime_encoding(&encoding, line)) ) return NULL;
		
	char* encoded;
	char* pdec;
	size_t szpdec;
	

	if( !(line = mime_unfolding_charset(&encoded, line)) ) return NULL;
	if( encoding == 0 ){
		pdec = quote_printable_decode(&szpdec, encoded);	
	}
	else{
		pdec = base64_decode(&szpdec, encoded);
	}
	free(encoded);
	*out = realloc(*out, sizeof(char) * (*size + szpdec));
	if( !*out ) err_fail("realloc");
	memcpy(&((*out)[*size]), pdec, szpdec);
	*size += szpdec;
	free(pdec);
	
	return line;
}

__private const char* mime_unfolding(char** out, const char* line){
	char* str = NULL;
	size_t size = 0;

	while( *line ){
		if( !strncmp(line, "=?", 2) ){
			line = mime_encoded_word(&str, &size, line);
			if( !line ){
				if( str ) free(str);
				return NULL;
			}
		}	
		else if( !strncmp(line, "\r\n", 2 ) ){
			line += 2;
			if( *line != ' ' && *line != '\t' ) break;
			++line;
		}
		else{
			++size;
			str = realloc(str, sizeof(char) * size); 
			if( !str ) err_fail("realloc");
			str[size-1] = *line++;
		}
	}
	
	++size;
	str = realloc(str, sizeof(char) * size); 
	if( !str ) err_fail("realloc");
	str[size-1] = 0;
	*out = str;
	return line;
}

__private int mime_pass_fail(const char* line){
	line = strpbrk(line, ":\r");
	if( !line || *line != ':' ) return -1;
	line = str_skip_h(line+1);
	return !strncmp(line, "PASS", strlen("PASS"));
}

__private void mime_content_free(imapMimeContent_s* content){
	if( content->data ) free(content->data);
	if( content->disposition ) free(content->disposition);
	if( content->filename ) free(content->filename);
	if( content->type ) free(content->type);
	free(content);
}

#define MIME_CONTENT_ENCODING_8BIT            0
#define MIME_CONTENT_ENCODING_QUOTEDPRINTABLE 1
#define MIME_CONTENT_ENCODING_BASE64          2
#define MIME_CONTENT_DATA_SIZE                128

__private const char* mime_content_base64_unfolding(size_t* size, void** out, const char* line){
	size_t bsize = MIME_CONTENT_DATA_SIZE;
	size_t len = 0;
	__mem_free char* data = mem_many(char, bsize);
	if( !data ){
		err_pushno("eom");
		return NULL;
	}
	while( *line && str_ancmp(line, "--") && str_ancmp(line, "\r\n") ){
		while( *line && str_ancmp(line, "\r\n") ){
			data[len++] = *line++;
			if( len >= bsize - 1 ){
				bsize += MIME_CONTENT_DATA_SIZE;
				data = realloc(data, bsize);
				if( !data ) err_fail("realloc");
			}
		}
		if( *line ) line += 2;
	}
	
	*out = base64_decode(size, data);
	scan_build_unknown_cleanup(data);
	return line;
}

__private const char* mime_content_quotedprintable_unfolding(size_t* size, char** out, const char* line){
	const char* begin = line;
	while( *line && str_ancmp(line, "--") && str_ancmp(line, "\r\n") ){
		line = imap_fetch_next_line((char*)line);
		if( !line ) return NULL;
	}
	size_t len = line - begin;

	*out = quote_printable_decode(&len, begin);
	*size = strlen(*out);

	return line;	
}

__private const char* mime_content_8bit_unfolding(size_t* size, void** out, const char* line){
	const char* begin = line;

	while( *line && str_ancmp(line, "\r\n--") ) ++line;
	*size = line - begin;

	char* data = mem_many(char, *size+1);
	if( !data ){
		err_pushno("eom");
		return NULL;
	}
	memcpy(data, line, *size);
	
	data[*size-1] = 0;
	*out = data;

	return line+2;
}
__private const char* mime_content_data_encoding(size_t* size, void** out, int encoding, const char* line){
	switch( encoding ){
		case MIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
			line = mime_content_base64_unfolding(size, out, line);
		break;

		case MIME_CONTENT_ENCODING_BASE64:
			line = mime_content_quotedprintable_unfolding(size, (char**)out, line);
		break;

		case MIME_CONTENT_ENCODING_8BIT:
			line = mime_content_8bit_unfolding(size, out, line);
		break;
	}
	return str_ancmp(line, "\r\n")? line : imap_fetch_next_line((char*)line);
}

__private const char* mime_content(imapMimeContent_s** head, const char* boundary, const char* line){
	if( strncmp(line, boundary, strlen(boundary)) ){
		//dbg_info("no boundary");
		return imap_fetch_next_line((char*)line);
	}
	line = imap_fetch_next_line((char*)line);
	//dbg_info("content next line:%.16s", line);
	if( !line ) return NULL;
	if( !(*line) ) return line;

	//dbg_info("content find");

	imapMimeContent_s* content = mem_zero_many(imapMimeContent_s, 1);
	if( !content ){
		err_pushno("eom");
		return NULL;
	}
	int encoding = -1;

	while( line && *line && str_ancmp(line, "\r\n") ){
		if( !str_ancmp(line, "Content-Type:") ){
			line = str_skip_h(line+strlen("Content-Type:"));
			if( !(line = mime_unfolding(&content->type, line)) ){
				mime_content_free(content);
				//dbg_error("parsing content type");
				return NULL;
			}
			//dbg_info("content-type::%s", content->type);
		}
		else if( !str_ancmp(line, "Content-Transfer-Encoding:") ){
			line = str_skip_h(line+strlen("Content-Transfer-Encoding:"));
			if( !str_ancmp(line, "base64") ){
				encoding = MIME_CONTENT_ENCODING_BASE64;
				//dbg_info("encoding base 64");
			}
			else if( !str_ancmp(line, "quoted-printable") ){
				encoding = MIME_CONTENT_ENCODING_QUOTEDPRINTABLE;
				//dbg_info("encoding quoted printable");
			}
			else if( !str_ancmp(line, "8bit") ){
				encoding = MIME_CONTENT_ENCODING_8BIT;
				//dbg_info("encoding 8bit");
			}
			else{
				err_push("unknow encoding type:%.32s", line);
				mime_content_free(content);
				return NULL;	
			}
			line = imap_fetch_next_line((char*)line);
			if( !line ){
				mime_content_free(content);
				return NULL;
			}
		}
		else if( !str_ancmp(line, "Content-Disposition:") ){
			line = str_skip_h(line+strlen("Content-Disposition:"));	
			if( !(line = mime_unfolding(&content->disposition, line)) ){
				mime_content_free(content);
				//dbg_error("parsing content disposition");
				return NULL;
			}
			//dbg_info("content-disposition:: %s", content->disposition);
		}
		else{
			//dbg_info("unparsed content:%.16s", line);
			line = imap_fetch_next_line((char*)line);
		}
	}
	//dbg_info("end parse header content");

	const char* searchfilename;
	if( content->disposition && (searchfilename = strstr(content->disposition, "filename=\"")) ){
		searchfilename += strlen("filename=\"");
		content->filename = str_dup_ch(searchfilename,'"');
	}
	else if( content->type && (searchfilename = strstr(content->type, "name=\"")) ){
		searchfilename += strlen("name=\"");
		content->filename = str_dup_ch(searchfilename,'"');
	}
	//dbg_info("content-filename:: %s", content->filename);

	if( line && *line ){
		line = imap_fetch_next_line((char*)line);
		if( !line ){
			mime_content_free(content);
			return NULL;
		}
		//dbg_info("decoding content");
		line = mime_content_data_encoding(&content->dataSize, &content->data, encoding, line);
		if( !line ){
			mime_content_free(content);
			err_push("decoding content");
			return NULL;
		}
	}
	
	content->next = *head;
	*head = content;
	//dbg_info("end parse content");
	return line;
}

__private err_t mime_parse(imapMime_s* mime, const char* msg){
	static mmph_s header[] = {
		{"X-AUTH-Result:",           NULL, 0},
		{"X-SID-Result:",            NULL, 0},
		{"To:",                      NULL, 1},
		{"Subject:",                 NULL, 1},
		{"Date:",                    NULL, 1},
		{"From:",                    NULL, 1},
		{"Reply-To:",                NULL, 1},
		{"Content-Type: multipart/", NULL, 2},
		{"--",                       NULL, 3}
	};

	mime->to = NULL;
	mime->subject = NULL;
	mime->date = NULL;
	mime->from = NULL;
	mime->replyto = NULL;
	mime->boundary = NULL;
	mime->content = NULL;

	header[0].dup = &mime->xauth;
	header[1].dup = &mime->xsid;
	header[2].dup = &mime->to;
	header[3].dup = &mime->subject;
	header[4].dup = &mime->date;
	header[5].dup = &mime->from;
	header[6].dup = &mime->replyto;
	header[7].dup = &mime->boundary;
	header[8].dup = &mime->content;

	//dbg_info("");
	//dbg_info("new mime");

	size_t nassign = 0;
	while( msg && *msg && nassign < MIME_HEADER_COUNT ){
		size_t i;
		for( i = 0; i < MIME_HEADER_COUNT; ++i ){
			if( !strncmp(header[i].name, msg, strlen(header[i].name)) ){
				//dbg_info("assign %s", header[i].name);
				switch( header[i].type){
					case 0:{
						int* v = header[i].dup;
						*v = mime_pass_fail(msg);
						if( *v < 0 ){
							err_push("wrong %s value", header[i].name);
							nassign = MIME_HEADER_COUNT + 1;
							break;
						}
						//dbg_info("value: %d", *v);
						msg = imap_fetch_next_line((char*)msg);
						if( !msg ){
							err_push("next line");
							nassign = MIME_HEADER_COUNT + 1;
							break;
						}
						++nassign;
					}
					break;
					
					case 1:{
						char** v = header[i].dup;
						if( *v ){
							err_push("reassign data %s",header[i].name);
							nassign = MIME_HEADER_COUNT + 2;
							break;
						}
						msg += strlen(header[i].name);
						msg = str_skip_h(msg);
						msg = mime_unfolding(v, msg);
						if( msg == NULL ){
							err_push("wrong %s value", header[i].name);
							nassign = MIME_HEADER_COUNT + 1;
						}
						//dbg_info("value: %s", *v);
						++nassign;
					}
					break;

#define BOUNDARY "boundary=\""
					case 2:{
						size_t szb = strlen(BOUNDARY);
						char** v = header[i].dup;
						if( *v ){
							err_push("reassign data %s",header[i].name);
							nassign = MIME_HEADER_COUNT + 2;
							break;
						}
						__mem_free char* ct = NULL;
						msg = mime_unfolding(&ct, msg);
						if( !msg || !ct ){
							err_push("error unfolding content-type");
							nassign = MIME_HEADER_COUNT + 2;
							break;
						}
						const char* pb = strstr(ct, BOUNDARY);
						if( !pb ){
							err_push("wrong %s value", header[i].name);
							nassign = MIME_HEADER_COUNT + 2;
							break;
						}
						pb += szb;
						const char* begin = pb;
						while( *pb && *pb != '\r' && *pb != '"' ) ++pb;
						*v = str_dup(begin, pb-begin);
						//dbg_info("value: %s", *v);
						++nassign;
					}
					break;

					case 3:{
						char** boundary = header[7].dup;
						if( !*boundary ){
							msg = imap_fetch_next_line((char*)msg);
							break;
						}
						//dbg_info("check content boundary: %s", *boundary);
						msg = mime_content(header[8].dup, *boundary, msg+2);
						if( !msg ){
							err_push("error content");
							nassign = MIME_HEADER_COUNT + 3;
							break;
						}
					}
					break;
				}
				break;
			}
		}
		if( i >= MIME_HEADER_COUNT ) msg = imap_fetch_next_line((char*)msg);
	}

	if( !msg || nassign > MIME_HEADER_COUNT ){
		err_push("wrong fetch header, err %ld", nassign - MIME_HEADER_COUNT);
		return -1;
	}	

	return 0;
}

void imap_mime_delete(imapMime_s* mime){
	if( mime->to ) free(mime->to);
	if( mime->subject ) free(mime->subject);
	if( mime->date ) free(mime->date);
	if( mime->from ) free(mime->from);
	if( mime->replyto ) free(mime->replyto);
	if( mime->boundary ) free(mime->boundary);
	imapMimeContent_s* next;
	while( mime->content ){
		next = mime->content->next;
		mime_content_free(mime->content);
		mime->content = next;
	}
}

void imap_mime_free(imapMime_s* mime, size_t count){
	for( size_t i = 0; i < count; ++i ){
		imap_mime_delete(&mime[i]);
	}
	free(mime);
}

imapMime_s* imap_fetch_to_mime(imapFetch_s* fetch, size_t count){
	imapMime_s* mime = mem_zero_many(imapMime_s, count);
	if( mime == NULL ){
		err_pushno("eom");
		return NULL;
	}

	for( size_t i = 0; i < count; ++i ){
		if( mime_parse(&mime[i], fetch[i].mime) ){
			imap_mime_free(mime, count);
			err_push("on parsing %lu fetched message id %lu", i, fetch[i] .id);
			return NULL;
		}
	}

	return mime;
}

err_t imap_flags(imap_s* ima, const char* dir, size_t id, char mode, imapFetchFlags_e flag){
	static char* flagsname[] = {
			"Seen", 
			"Answered", 
			"Flagged", 
			"Deleted", 
			"Draft"
	};

	if( flag > IMAP_FETCH_FLAGS_DRAFT || flag < 0 ){
		err_pushno("unknow flag %d", flag);
		return -1;
	}

	__mem_free char* url = str_printf("%s/%s/", ima->server, dir);
	www_url_set(&ima->www, url);
	www_custom_reqest(&ima->www, "STORE %lu %cFLAGS \\%s", id, mode, flagsname[flag]);

	if( www_perform(&ima->www, 0) ){
		www_url_set(&ima->www, url);
		return -1;
	}
	www_url_set(&ima->www, ima->server);

	//printf("<HEADER>%s</HEADER>\n<BODY>%s</BODY>\n<CODE %d>\n", ima->www.header.buf, ima->www.body.buf,0);

	return 0;
}

