#include "test.h"
#include <ef/utf8.h>

/*@test -s --str 'test string'*/

__private err_t cast16escape(utf8_t* out, const char* str){
	while( *str ){
		if( *str == '\\' ){
			const char* next;
			ssize_t nch;
			if( (nch=utf8_from_seu16(out, 8, str, &next)) < 0 ) return -1;
			str = next;
			out += nch;
		}
		else{
			*out++ = *str++;
		}
	}
	*out = 0;

	return 0;
}

/*@fn*/
void test_str(__unused const char* argA, __unused const char* argB){
	char a[128] = "password";
	char b[128] = "hello";
	char c[128] = "123456789";
	char d[128] = "qwertyui";

	str_swap(a,b);
	TESTF("swap a > b", str_equal(b, strlen(b), "password", strlen("password")));
	TESTF("swap a > b", str_equal(a, strlen(a), "hello", strlen("hello")));
	str_swap(b,c);
	TESTF("swap a < b", str_equal(c, strlen(c), "password", strlen("password")));
	TESTF("swap a < b", str_equal(b, strlen(b), "123456789", strlen("123456789")));
	str_swap(c,d);
	TESTF("swap a == b", str_equal(d, strlen(d), "password", strlen("password")));
	TESTF("swap a == b", str_equal(c, strlen(c), "qwertyui", strlen("qwertyui")));


	char* testvu16 = "hello \\u02B4";
	utf8_t out[4096];
	if( cast16escape(out, testvu16) ) printf("fail cast u16 %s\n", testvu16);
	printf("cast to u8:%s\n", out);

	char* unvalid16 = "hello \\ud83c";
	if( !cast16escape(out, unvalid16) ) printf("fail cast invalid surrugate u16 %s\n", unvalid16);

	char* surrugate16 = "hello \\ud83c\\udf09";
	if( cast16escape(out, surrugate16) ) printf("fail cast surrugate u16 %s\n", surrugate16);
	printf("cast to u8:%s\n", out);


	printf("%d\n", strcmp("ciaobella", "ciao"));

}
