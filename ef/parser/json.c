#include <ef/json.h>
#include <ef/str.h>
#include <ef/utf8.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/os.h>

__private char* jsonErrStr[JSON_ERR_COUNT] = {
	[JSON_ERR_STRING_END]      = "string not terminated with \"",
	[JSON_ERR_STRING_LEN]      = "string length",
	[JSON_ERR_STRING]          = "string",
	[JSON_ERR_UNICODE]          = "invalid unicode",
	[JSON_ERR_NUMBER]          = "number",
	[JSON_ERR_CONSTANT]        = "not valid constant true/false/null",
	[JSON_ERR_OBJECT_NAME]     = "object not have name",
	[JSON_ERR_OBJECT_COLON]    = "name not separated with colon",
	[JSON_ERR_OBJECT_VALUE]    = "object not have value",
	[JSON_ERR_OBJECT_END]      = "object not terminated with }",
	[JSON_ERR_ARRAY_END]       = "array not terminated with ]",
	[JSON_ERR_NOSPACE]         = "no space",
	[JSON_ERR_UNASPECTED_CHAR] = "unaspected char",
	[JSON_ERR_USER]            = "user error"
};

__private const char* oldlocale;

void json_begin(void){
	oldlocale = os_setlocale(LC_NUMERIC, "C");
}

void json_end(void){
	os_setlocale(LC_NUMERIC, oldlocale);
}

void json_error(json_s* json){
	if( json->err == 0 ){
		fputs("json ok", stderr);
		putchar('\n');
		return;
	}

	fprintf(stderr, "json error");
	
	fprintf(stderr,"(%d):", json->err);

	if( json->err < 0 ){
		fprintf(stderr, "unknow descript error");
	}
	else if( json->err < JSON_ERR_COUNT ){
		fprintf(stderr, "%s", jsonErrStr[json->err]);
	}
	else if( json->err > JSON_ERR_USER && json->usrError ){
		fprintf(stderr, "%s", json->usrError[json->err-(JSON_ERR_USER+1)]);
	}

	fprintf(stderr, " on line %lu\n", json->cline);

	char const* ch = json->beginLine;
	char const* bl = json->beginLine;
	size_t max = 80; 
	if( json->current - json->beginLine > 80 ){
		bl = json->current - 40;
		ch = bl;
	}
	
	while( *ch && *ch != '\n' && max-->0 ) fputc(*ch++, stderr);
	
	fputc('\n',stderr);	
	size_t off = json->current - bl;
	while( off-->0 ) fputc(' ', stderr);

	fputc('^', stderr);	
	fputc('\n',stderr);
}

char* json_error_allocstr(json_s* json){
	if( json->err == 0 ){
		return str_dup("json ok",0);
	}

	const char* descript = "";

	if( json->err < 0 ){
		descript = "unknow descript error";
	}
	else if( json->err < JSON_ERR_COUNT ){
		descript = jsonErrStr[json->err];
	}
	else if( json->err > JSON_ERR_USER && json->usrError ){
		descript = json->usrError[json->err-(JSON_ERR_USER+1)];
	}

	char pline[90];
	size_t il = 0;
	char const* ch = json->beginLine;
	char const* bl = json->beginLine;
	size_t max = 80; 
	if( json->current - json->beginLine > 80 ){
		bl = json->current - 40;
		ch = bl;
	}
	
	while( *ch && *ch != '\n' && max-->0 ) pline[il++] = *ch++;
	pline[il++] = '\n';	
	size_t off = json->current - bl;
	while( off-->0 ) pline[il++] = ' ';
	pline[il++] = '^';
	pline[il] = 0;

	size_t len = snprintf(NULL, 0, "json error:(%d) %s, on line %lu\n%s", json->err, descript, json->cline, pline);
	char* ret = mem_many(char, len+1);
	sprintf(ret, "json error:(%d) %s, on line %lu\n%s", json->err, descript, json->cline, pline);
	return ret;
}

void json_push_error(json_s* json){
	__mem_free char* desc = json_error_allocstr(json);
	err_push("%s", desc);
	scan_build_unknown_cleanup(desc);
}

inline __private void json_token_next(json_s* json){
	while(1){
		while( *json->current && (*json->current == ' ' || *json->current == '\t') ) ++json->current;
		if( *json->current == '\n' ){
			++json->current;
			json->beginLine = json->current;
			++json->cline;
			continue;
		}
		break;
	}
}

__private err_t json_string_size(char const* data, size_t* len){
	iassert( data && len );
	char const* begin = data;
	while( *data && *data != '"' ){
		if( *data == '\\' && *(data+1) == '\"' ){
			data += 2;
		}
		else{
			++data;
		}
	}
	if( *data != '"' ) return JSON_ERR_STRING_END;
	*len = data - begin;
	//dbg_info("string:'%.*s'",(int)*len, begin); 
	return JSON_OK;
}

inline __private int json_number_isdigt(char ch){
	return (ch >= '0' && ch <= '9') ? 1 : 0;
}	

__private const char* json_number_size(char const* data, size_t* len, int* isdouble){
	char const* begin = data;
	int zerointeger = 0;
	int sign = 0;
	*isdouble = 0;
	
	if( *data == '-' ){
		sign = 1;
		++data;
	}
	if( *data == '0' ){
		zerointeger = 1;
		++data;
	}

	while( json_number_isdigt(*data) ) ++data;
	if( *data == '.' ){
		*isdouble = 1;
	   	++data;
		if( !json_number_isdigt(*data) ) goto ONERR;
		while( json_number_isdigt(*data) ) ++data;
	}

	if( *data == 'e' || *data == 'E' ){
		++data;
		*isdouble = 1;
		if( *data == '+' || *data == '-' ) ++data;
		if( !json_number_isdigt(*data) ) goto ONERR;
		while( json_number_isdigt(*data) ) ++data;
	}

	*len = data - begin;
	if( *len == 0 ) goto ONERR;
	if( zerointeger && *len - sign > 1 && !*isdouble ){
		goto ONERR;
	}
	//dbg_info("parsing:%.*s", (int)*len, begin);
	return NULL;
ONERR:
	//dbg_error("on string number:'%.*s'", (int)(data-begin), begin);
	return data;
}

__private int json_lexer_value(json_s* json);

__private int json_lexer_object(json_s* json){
	if( json->objectNew && (json->err=json->objectNew(json)) ) return -1;

	++json->current;
	if( *json->current == '}' ){
		json->err = JSON_ERR_NOSPACE;
		return -1;
	}
	
	json_token_next(json);
	if( *json->current == '}' ){
		if( json->objectEnd && (json->err=json->objectEnd(json)) ) return -1;
		return 0;
	}

	while(*json->current){
		if( json->objectNext && (json->err=json->objectNext(json)) ) return -1;

		if( *json->current != '"' ){
			json->err = JSON_ERR_OBJECT_NAME;
			return -1;
		}
		++json->current;
		
		size_t len;
		if( (json->err=json_string_size(json->current, &len)) ) return -1;
		//dbg_info("json.object:%.*s", (int)len, json->current);
		char* escprop = json_unescape(json, json->current, len);
		if( escprop == NULL ) return -1;
		if( json->objectProperties && (json->err=json->objectProperties(json, &escprop, strlen(escprop)) ) ) return -1;
		if( escprop ) free(escprop);

		json->current += len + 1;
		json_token_next(json);
		if( *json->current != ':' ){
			json->err = JSON_ERR_OBJECT_COLON;
			return -1;
		}

		++json->current;
		json_token_next(json);
		if( json_lexer_value(json) ) return -1;
		json_token_next(json);
		
		if( *json->current == '}' ) break;
		if( *json->current != ',' ){
			json->err = JSON_ERR_OBJECT_END;
			return -1;
		}
		++json->current;
		json_token_next(json);
	}

	if( *json->current != '}' ){
		json->err = JSON_ERR_OBJECT_END;
		return -1;
	}
	
	++json->current;
	if( json->objectEnd && (json->err=json->objectEnd(json)) ) return -1;
	return 0;
}

__private int json_lexer_array(json_s* json){
	if( json->arrayNew && (json->err=json->arrayNew(json)) ) return -1;

	++json->current;
	if( *json->current == ']' ){
		json->err = JSON_ERR_NOSPACE;
		return -1;
	}
	
	json_token_next(json);
	if( *json->current == ']' ){
		if( json->arrayEnd && (json->err=json->arrayEnd(json)) ) return -1;
		return 0;
	}
	if( json->arrayNext && (json->err=json->arrayNext(json)) ) return -1;
	if( json_lexer_value(json) ) return -1;
	
	json_token_next(json);	
	while( *json->current == ',' ){
		++json->current;
		json_token_next(json);
		if( json->arrayNext && (json->err=json->arrayNext(json)) ) return -1;
		if( json_lexer_value(json) ) return -1;
		json_token_next(json);
	}

	if( *json->current != ']' ){
		json->err = JSON_ERR_ARRAY_END;
		return -1;
	}

	++json->current;
	if( json->arrayEnd && (json->err=json->arrayEnd(json)) ) return -1;
	return 0;
}

__private int json_lexer_value(json_s* json){
	switch( *json->current ){
		case 0: return 1;
		
		case '{': return json_lexer_object(json);
		
		case '[': return json_lexer_array(json);

		case '"':{
			size_t len;
			++json->current;
			if( (json->err=json_string_size(json->current, &len)) ) return -1;
			char* esc = json_unescape(json, json->current, len);
			if( esc == NULL ) return -1;
			if( json->valueString && (json->err=json->valueString(json, &esc, strlen(esc))) ) return -1;
			if( esc ) free(esc);
			json->current += len + 1;
		}
		break;

		case '-': case '0' ... '9':{
			size_t len;
			int isdouble;
			const char* errd = json_number_size(json->current, &len, &isdouble);
			if( errd ){
				json->err = JSON_ERR_NUMBER;
				json->current = errd;
				return -1;
			}
			if( isdouble ){
				if( json->valueFloat && (json->err=json->valueFloat(json, json->current, len)) ) return -1;
			}
			else{
				if( json->valueInteger && (json->err=json->valueInteger(json, json->current, len)) ) return -1;
			}
			json->current += len;
		}
		break;

		case 't':
			if( strncmp(json->current, "true", 4) ){
				json->err = JSON_ERR_CONSTANT;
			   	return -1;
			}
			if( json->valueTrue && (json->err=json->valueTrue(json)) ) return -1;
			json->current += 4;
		break;

		case 'f':
			if( strncmp(json->current, "false", 5) ){
				json->err = JSON_ERR_CONSTANT;
			   	return -1;
			}
			if( json->valueFalse && (json->err=json->valueFalse(json)) ) return -1;
			json->current += 5;
		break;

		case 'n':
			if( strncmp(json->current, "null", 4) ){
				json->err = JSON_ERR_CONSTANT;
			   	return -1;
			}
			if( json->valueNull && (json->err=json->valueNull(json)) ) return -1;
			json->current += 4;
		break;

		default: json->err = JSON_ERR_UNASPECTED_CHAR; return -1;
	}

	return 0;
}

err_t json_lexer(json_s* json, char const* data){
	//dbg_info("<JSON>%s</JSON>", data);
	json->cline = 0;
	json->err = JSON_OK;
	json->beginLine = json->text = data;
	json->current = data-1;
	do{
		++json->current;
		json_token_next(json);
		int ret = json_lexer_value(json);
		switch( ret ){	
			case -1: return -1;
			case  1: return  0;
		} 
		json_token_next(json);
	}while(*json->current == ',');
	if( *json->current ) json->err = JSON_ERR_UNASPECTED_CHAR;
	return *json->current;
}

char* json_unescape(json_s* js, const char* str, size_t len){
	//dbg_info("string len %lu", len);
	size_t esz = len + 1;
	size_t el = 0;
	char* esc = mem_many(char, esz);
	if( esc == NULL ) return NULL;

	const char* ends = str + len;
	while( str < ends ){
		if( *str >= 0 && *str < 32 ){
			//dbg_error("control char %d", *str);
			free(esc);
			if( js ){
				js->err = JSON_ERR_STRING;
				js->current = str;
			}
			return NULL;
		}

		if( *str != '\\' ){
			//dbg_info("cp (%d)", *str);
			esc[el++] = *str++;
		}
		else{
			++str;
			//dbg_info("check escape (%c)", *str);
			switch( *str ){
				case '\"':
				case '\\':
				case '/':
					esc[el++] = *str++;
				break;

				case 'b':
					if( el > 0 ) --el;
				break;

				case 'f':
					esc[el++] = '\f';
					++str;
				break;

				case 'n':
					esc[el++] = '\n';
					++str;
				break;

				case 'r':
					esc[el++] = '\r';
					++str;
				break;

				case 't':
					esc[el++] = '\t';
					++str;
				break;

				case 'u':{
					--str;
					if( el + 8 >= esz - 1 ){
						esz += 9; 
						char* tmp = realloc(esc, esz * sizeof(char));
						if( tmp == NULL ){
							err_fail("realloc");
						}
						esc = tmp;
					}
					const char* next = str;
					ssize_t n = utf8_from_seu16((utf8_t*)&esc[el], (esz-1) - el, str, &next);
					if( n < 0 ){
						if( js ){
							js->err = JSON_ERR_UNICODE;
							js->current = str;
						}
						free(esc);
						return NULL;
					}
					else if( n == 1 && esc[el] < 32 ){
						if( js ){
							js->err = JSON_ERR_STRING;
							js->current = str;
						}
						free(esc);
						return NULL;
					}
					str = next;
					el += n;
				}					 
				break;

				default:
					//dbg_error("invalid escape");
					if( js ){
						js->err = JSON_ERR_STRING;
						js->current = str;
					}
					free(esc);
				return NULL;
			}
		}
		
		if( el >= esz - 1 ){
			esz += ends - str < 2 ? 2 : ends - str; 
			char* tmp = realloc(esc, esz * sizeof(char));
			if( tmp == NULL ){
				err_fail("realloc");
			}
			esc = tmp;
		}
	}
	
	esc[el] = 0;
	
	char* unc =(char*) utf_validate_n((utf8_t*)esc, el);
	if( unc ){
		//dbg_error("not valid unicode");
		if( js ){
			js->err = JSON_ERR_STRING;
			js->current = unc;
		}
		free(esc);
		return NULL;
	}
	return esc;
}

err_t json_long_validation(long* ret, const char* number, size_t len){
	char* en;
	errno = 0;
	if( ret ){
		*ret = strtol(number, &en, 10);
	}
	else{
		strtol(number, &en, 10);
	}
	if( errno || !en || en == number || en != number + len) return -1;
	return 0;
}

err_t json_float_validation(double* ret, const char* number, size_t len){
	char* en;
	errno = 0;
	if( ret ){
		*ret = strtod(number, &en);
	}
	else{
		strtod(number, &en);
	}
	if( errno || !en || en == number || en != number + len) return -1;
	return 0;
}



















