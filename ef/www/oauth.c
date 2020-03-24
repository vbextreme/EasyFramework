#include <ef/oauth.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>
#include <ef/delay.h>
#include <ef/mth.h>
#include <ef/spawn.h>
#include <ef/socket.h>
#include <ef/file.h>
#include <ef/json.h>
#include <ef/file.h>
#include <ef/config.h>
#include <ef/trie.h>

#define OAUTH2_BUFF         4096
#define OAUTH2_LOCAL        "http://127.0.0.1"
#define OAUTH2_LOCAL_PORT_S "8181"
#define OAUTH2_LOCAL_PORT   8181
#define OAUTH2_LOCAL_URI    OAUTH2_LOCAL ":" OAUTH2_LOCAL_PORT_S

oauth2_s* oauth2_new(wwwProgress_s* prog){
	oauth2_s* oa = mem_new(oauth2_s);
	if( !oa ){
		err_pushno("malloc");
		return NULL;
	}
	oa->codeReply = NULL;
	oa->token = NULL;
	oa->tokenRefresh = NULL;
	oa->scope = NULL;
	oa->expiresTime = 0;
	oa->expires = 0;

	oa->http = http_new(OAUTH2_BUFF, WWW_FLAGS_SSL | WWW_FLAGS_FOLLOW | WWW_FLAGS_BODY | WWW_FLAGS_HEADER | WWW_FLAGS_APP_JSON,  prog);
	if( !oa->http ){
		free(oa);
		return NULL;
	}
	return oa;
}

void oauth2_free(oauth2_s* oa){
	if( oa->codeReply ) free(oa->codeReply);
	if( oa->token ) free(oa->token);
	if( oa->tokenRefresh ) free(oa->tokenRefresh);
	if( oa->scope ) free(oa->scope);
	if( oa->http ) http_free(oa->http);
	free(oa);
}

//https://accounts.google.com/o/oauth2/auth?client_id=[Application Client Id]&redirect_uri=urn:ietf:wg:oauth:2.0:oob&scope=[Scopes]&response_type=code

err_t oauth2_athorization_browser(oauth2_s* oa, oauth2Authorization_s* auth, const char* browser, long timeoutms){
	__www_escape_free char* redir = www_uri_escape(&oa->http->www, OAUTH2_LOCAL_URI, 0);
	
	mth_random_string_azAZ09(oa->rndStr, OAUTH_RND_STR_SIZE);

	__www_escape_free char* scope =  www_uri_escape(&oa->http->www,auth->scope, 0);

	__mem_free char* url = str_printf("%s '%s?client_id=%s&redirect_uri=%s&scope=%s&state=%s%s%s&include_granted_scopes=true&response_type=code'",
			browser,
			auth->authorizationLink,
			auth->appClientId,
			redir,
			scope,
			oa->rndStr,
			auth->accessType?"&access_type=":"",
			auth->accessType?auth->accessType:""
	);

	dbg_info("oauth2.url:%s", url);
	if( url == NULL ){
		err_push("oops str_printf");
		return -1;
	}

	__socket_close socket_s* server = socket_listen( 
		socket_open( 
			socket_new(SOCKET_TYPE_NET4, NULL, NULL, NULL),
			0, 0, 0
		),
		(esport_u){.port = OAUTH2_LOCAL_PORT}
	);
	if( !server ){
		err_push("on create local web server");
		return -1;
	}

	spawn_wait(spawn_shell(url,1), NULL);

	//dbg_info("start webserver");
	__socket_close socket_s* servlet = socket_accept(
		socket_new(SOCKET_TYPE_NET4, NULL, NULL, NULL),
		server, OAUTH2_BUFF, timeoutms, NULL
	);
	if( !servlet ){
		err_push("no connection on local web server");
		return -1;
	}
	//dbg_info("accepted connection");

	err_t err = 1;
	char* request = NULL;
	while( stream_inp_strstr(servlet->stream,&request, "\r\n", 0) > 0 ){
		if( oa->codeReply ){
			continue;
		}

		if( !str_ancmp(request, "GET") ){
			//dbg_info("received GET: %s", request);
			if( !strstr(request, oa->rndStr) ){
				err_push("request fail state");
				break;
			}
			char* code = strstr(request, "&code=");
			if( !code ){
				err_push("no code");
				break;
			}
			code += strlen("&code=");
			char* endcode = strstr(code, "&");
			if( !endcode ){
				err_push("code not end");
				break;
			}
			oa->codeReply = str_dup(code, endcode - code);
			dbg_info("code:'%s'", oa->codeReply);
			err = 0;
		}
	}
	if( err ){
		return -1;
	}

	if( !auth->httpReplyOk ) auth->httpReplyOk =
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
			"<html>\n"
			" <head>\n"
			"  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
			"  <title>authorization get</title>\n"
			" </head>\n"
			" <body>\n"
			"  <h1>authorizated</h1>\n"
			" </body>\n"
			"</html>\n";

	if( !auth->httpReplyError ) auth->httpReplyError =  
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
			"<html>\n"
			" <head>\n"
			"  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
			"  <title>authorization get</title>\n"
			" </head>\n"
			" <body>\n"
			"  <h1>Invalid request</h1>\n"
			" </body>\n"
			"</html>\n";

	__mem_free char* reply = str_printf(
			"HTTP/1.1 200 OK\r\n"
			"Cache-Control: no-cache, private\r\n"
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"\r\n"
			"%s"
			"\r\n",
			err ? auth->httpReplyError : auth->httpReplyOk
	);

	stream_out_string(servlet->stream, reply, 0);
	stream_flush(servlet->stream);
	
	dbg_info("authorization code:'%s'", oa->codeReply);

	return err;
}

/*
__private err_t oauth2_athorization(oauth2_s* oa, oauth2Authorization_s* auth){
	dbg_info("oauth2 authorization");

	if( auth->redirectLink == NULL ){
		auth->redirectLink = "urn:ietf:wg:oauth:2.0:oob";
	}

	mth_random_string_azAZ09(oa->rndStr, OAUTH_RND_STR_SIZE);

	//__www_escape_free char* acid =  www_uri_escape(&oa->http.www,auth->appClientId, 0);
	//__www_escape_free char* reli =  www_uri_escape(&oa->http.www,auth->redirectLink, 0);
	__www_escape_free char* scop =  www_uri_escape(&oa->http.www,auth->scope, 0);

	__mem_free char* url = str_printf("%s?client_id=%s&redirect_uri=%s&scope=%s&state=%s&response_type=code%s",
			auth->authorizationLink,
			auth->appClientId,
			auth->redirectLink,
			scop,
			oa->rndStr,
			auth->accessType?auth->accessType:""
	);
	dbg_info("oauth2.url:%s", url);
	if( url == NULL ){
		err_push("oops str_printf");
		return -1;
	}

	err_t err = http_get(&oa->http, url);
	if( err ) return -1;

	www_dump_buffers(&oa->http.www);

	return 0;
}
*/
/*
curl \
--request POST \
--data "code=[Authentcation code from authorization link]&client_id=[Application Client Id]&client_secret=[Application Client Secret]&redirect_uri=urn:ietf:wg:oauth:2.0:oob&grant_type=authorization_code" \
https://accounts.google.com/o/oauth2/token
*/

__private err_t js_parse_property_err(json_s* ctx, char** name, size_t len){
	if( str_equal(*name, len, "error", strlen("error")) ){
		ctx->usrstat = 1;
	}
	else if( str_equal(*name, len, "error_description", strlen("error_description")) ){
		ctx->usrstat = 2;
	}

	return 0;
}

__private err_t js_parse_value_str_err(json_s* ctx, char** name, __unused size_t len){
	switch( ctx->usrstat ){		
		case 1:
			err_push("oauth2 error: %s", *name);
			ctx->usrstat = 0;
		break;
		case 2:
			err_push("oauth2 error descript: %s", *name);
			ctx->usrstat = 0;
		break;
	}
	return 0;
}

__private void parse_error(const char* str){
	json_begin();
	
	json_s js = {
		.objectNew = NULL,
		.objectNext = NULL,
		.objectProperties = js_parse_property_err,
		.objectEnd = NULL,
		.arrayNew = NULL,
		.arrayNext = NULL,
		.arrayEnd = NULL,
		.valueNull = NULL,
		.valueTrue = NULL,
		.valueFalse = NULL,
		.valueInteger = NULL,
		.valueFloat = NULL,
		.valueString = js_parse_value_str_err,
		.usrctx = NULL,
		.usrstat = 0,
		.usrval = NULL,
		.usrit = 0,
		.usrError = NULL,
	};

	if( json_lexer(&js, str) ){
		json_push_error(&js);
	}

	json_end();
}

__private err_t js_parse_property_tokenget(json_s* ctx, char** name, size_t len){
	oauth2_s* oa = ctx->usrctx;
	ctx->usrval = NULL;
	ctx->usrstat = 0;
	
	if( !str_equal(*name, len, "access_token", strlen("access_token")) ){
		if( oa->token ) mem_free_safe(oa->token);
		ctx->usrval = &oa->token;
		ctx->usrstat = 1;
	}
	else if( !str_equal(*name, len, "expires_in", strlen("expires_in")) ){
		ctx->usrval = &oa->expires;
		ctx->usrstat = 2;
	}
	else if( !str_equal(*name, len, "refresh_token", strlen("refresh_token")) ){
		if( oa->tokenRefresh ) mem_free_safe(oa->tokenRefresh);
		ctx->usrval = &oa->tokenRefresh;
		ctx->usrstat = 1;
	}
	else if( !str_equal(*name, len, "scope", strlen("scope")) ){
		if( oa->scope ) mem_free_safe(oa->scope);
		ctx->usrval = &oa->scope;
		ctx->usrstat = 1;
	}

	return 0;
}

__private err_t js_parse_value_str_tokenget(json_s* ctx, char** name, __unused size_t len){
	if( ctx->usrstat != 1 ) return 0;
	char** out = ctx->usrval;
	*out = *name;
	*name = NULL;
	return 0;
}

__private err_t js_parse_value_int_tokenget(json_s* ctx, const char* name, size_t len){
	if( ctx->usrstat != 2 ) return 0;
	size_t* out = ctx->usrval;
	long ret = 0;
	if( !json_long_validation(&ret, name, len) ){
		*out = ret;
	}
	return 0;
}

__private err_t parse_tokenget(oauth2_s* oa, const char* str){
	json_begin();
	
	json_s js = {
		.objectNew = NULL,
		.objectNext = NULL,
		.objectProperties = js_parse_property_tokenget,
		.objectEnd = NULL,
		.arrayNew = NULL,
		.arrayNext = NULL,
		.arrayEnd = NULL,
		.valueNull = NULL,
		.valueTrue = NULL,
		.valueFalse = NULL,
		.valueInteger = js_parse_value_int_tokenget,
		.valueFloat = NULL,
		.valueString = js_parse_value_str_tokenget,
		.usrctx = oa,
		.usrstat = 0,
		.usrval = NULL,
		.usrit = 0,
		.usrError = NULL,
	};

	if( json_lexer(&js, str) ){
		json_push_error(&js);
		return -1;
	}

	json_end();

	if( oa->token == NULL || oa->tokenRefresh == NULL || oa->expires == 0 ){
		err_push("no token or token refresh or expires time in json reply:%s", str);
		return -1;
	}

	return 0;
}

err_t oauth2_authorization_token_get(oauth2_s* oa, oauth2Authorization_s* auth){
	dbg_info("oauth2 token get");

	http_post_set(oa->http, "code", oa->codeReply);
	http_post_set(oa->http, "client_id", auth->appClientId);
	http_post_set(oa->http, "client_secret", auth->appClientSecret);
	http_post_set(oa->http, "redirect_uri", OAUTH2_LOCAL_URI);
	http_post_set(oa->http, "grant_type", "authorization_code");
	
	err_t err = http_post(oa->http, auth->tokenLink);

	www_dump_buffers(&oa->http->www);

	if( err ){
		if( oa->http->www.body.len > 0 ) parse_error(oa->http->www.body.buf);
		goto ONERR;
	}

	if( parse_tokenget(oa, oa->http->www.body.buf) ){
		goto ONERR;
	}

	oa->expiresTime = time(NULL) + oa->expires;

	dbg_info("token:%s", oa->token);
	dbg_info("refresh:%s", oa->tokenRefresh);
	dbg_info("scope:%s", oa->scope);
	dbg_info("now:%lu expires:%lu timeout:%lu", time(NULL), oa->expires, oa->expiresTime);

	return 0;

ONERR:
	http_post_free(oa->http);
	return err;
}

/*
curl \
--request POST \
--data 'client_id=[Application Client Id]&client_secret=[Application Client Secret]&refresh_token=[Refresh token granted by second step]&grant_type=refresh_token' \
https://accounts.google.com/o/oauth2/token
*/

err_t oauth2_authorization_token_refresh(oauth2_s* oa, oauth2Authorization_s* auth){
	dbg_info("oauth2 refresh token");

	http_post_set(oa->http, "client_id", auth->appClientId);
	http_post_set(oa->http, "client_secret", auth->appClientSecret);
	http_post_set(oa->http, "refresh_token", oa->tokenRefresh);
	http_post_set(oa->http, "grant_type", "refresh_token");
	
	err_t err = http_post(oa->http, auth->refreshTokenLink);
	dbg_info("err:%d", err);
	www_dump_buffers(&oa->http->www);

	if( err ){
		if( oa->http->www.body.len > 0 ) parse_error(oa->http->www.body.buf);
		goto ONERR;
	}

	if( parse_tokenget(oa, oa->http->www.body.buf) ){
		goto ONERR;
	}

	oa->expiresTime = time(NULL) + oa->expires;

	dbg_info("token:%s", oa->token);
	dbg_info("refresh:%s", oa->tokenRefresh);
	dbg_info("scope:%s", oa->scope);
	dbg_info("now:%lu expires:%lu timeout:%lu", time(NULL), oa->expires, oa->expiresTime);

ONERR:
	http_post_free(oa->http);
	return err;
}

long oauth2_authorization_remaining_time(oauth2_s* oa){
	return oa->expiresTime - time(NULL);
}

err_t oauth2_authorization_store(oauth2_s* oa, oauth2Authorization_s* auth, const char* path){
	__stream_close stream_s* out = stream_open(path, "w", 0660, 4096);
	if( !out ) return -1;

	stream_printf(out, "authorizationLink = '%s'\n", auth->authorizationLink ? auth->authorizationLink : "null" );
	stream_printf(out, "tokenLink = '%s'\n", auth->tokenLink ? auth->tokenLink : "null" );
	stream_printf(out, "refreshTokenLink = '%s'\n", auth->refreshTokenLink ? auth->refreshTokenLink : "null" );
	stream_printf(out, "httpReplyOk = '%s'\n", auth->httpReplyOk ? auth->httpReplyOk : "null" );
	stream_printf(out, "httpReplyError = '%s'\n", auth->httpReplyError ? auth->httpReplyError : "null" );
	stream_printf(out, "appClientId = '%s'\n", auth->appClientId ? auth->appClientId : "null" );
	stream_printf(out, "appClientSecret = '%s'\n", auth->appClientSecret ? auth->appClientSecret : "null" );
	stream_printf(out, "scope = '%s'\n", auth->scope ? auth->scope : "null" );
	stream_printf(out, "accessType = '%s'\n", auth->accessType ? auth->accessType : "null" );
	stream_printf(out, "token = '%s'\n", oa->token ? oa->token : "null" );
	stream_printf(out, "tokenRefresh = '%s'\n", oa->tokenRefresh ? oa->tokenRefresh : "null" );
	stream_printf(out, "retscope = '%s'\n", oa->scope ? oa->scope : "null" );
	stream_printf(out, "expires= '%lu'\n", oa->expires );
	stream_printf(out, "expiresTime = '%lu'\n", oa->expiresTime );

	return 0;
}

__private int cf_set(int type, char** value, void* userdata){
	switch( type ){
		case 0:{
			char** usr = userdata;
			if( value ){
				if( !str_ancmp(*value, "null") ){
					*usr = NULL;
				}
				else{
					*usr = *value;
					*value = NULL;
				}
			}
			else{
				*usr = NULL;
			}
		}
		break;

		case 1:{
			size_t* sz = userdata;
			if( value ){
				*sz = strtoul(*value, NULL, 10);
			}
			else{
				*sz = 0;
			}
		}
		break;
	}
	return 0;
}

err_t oauth2_authorization_load(oauth2_s* oa, oauth2Authorization_s* auth, const char* path){
	__stream_close stream_s* in = stream_open(path, "r", 0, 4096);
	if( !in ) return -1;

	const char* cn[] = {
		"authorizationLink",
		"tokenLink",
		"refreshTokenLink",
		"httpReplyOk",
		"httpReplyError",
		"appClientId",
		"appClientSecret",
		"scope",
		"accessType",
		"token",
		"tokenRefresh",
		"retscope",
		"expires",
		"expiresTime",
		NULL
	};

	__trie_free trie_s* kn = trie_new(NULL);
	configTrie_s ct[] = {
		{ .fn = cf_set, .type = 0, .userdata = &auth->authorizationLink },
		{ .fn = cf_set, .type = 0, .userdata = &auth->tokenLink },
		{ .fn = cf_set, .type = 0, .userdata = &auth->refreshTokenLink },
		{ .fn = cf_set, .type = 0, .userdata = &auth->httpReplyOk },
		{ .fn = cf_set, .type = 0, .userdata = &auth->httpReplyError },
		{ .fn = cf_set, .type = 0, .userdata = &auth->appClientId },
		{ .fn = cf_set, .type = 0, .userdata = &auth->appClientSecret },
		{ .fn = cf_set, .type = 0, .userdata = &auth->scope },
		{ .fn = cf_set, .type = 0, .userdata = &auth->accessType },
		{ .fn = cf_set, .type = 0, .userdata = &oa->token },
		{ .fn = cf_set, .type = 0, .userdata = &oa->tokenRefresh },
		{ .fn = cf_set, .type = 0, .userdata = &oa->scope },
		{ .fn = cf_set, .type = 1, .userdata = &oa->expires },
		{ .fn = cf_set, .type = 1, .userdata = &oa->expiresTime },
	};

	for( size_t i = 0; cn[i]; ++i){
		trie_insert(kn, cn[i], &ct[i]);
	}

	return config_parse(kn, in);
}

int oauth2_authorization_ok(oauth2_s* oa){
	return oa->token && oa->tokenRefresh ? 1 : 0;
}
