#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "easylist.h"

char* itoa(int value, char* result, int base)
{
		// check that the base if valid
		if (base < 2 || base > 36) { *result = '\0'; return result; }

		char* ptr = result, *ptr1 = result, tmp_char;
		int tmp_value;

		do {
			tmp_value = value;
			value /= base;
			*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
		} while ( value );

		// Apply negative sign
		if (tmp_value < 0) *ptr++ = '-';
		*ptr-- = '\0';
		while(ptr1 < ptr) {
			tmp_char = *ptr;
			*ptr--= *ptr1;
			*ptr1++ = tmp_char;
		}
		return result;
	}

int main()
{

    LHS l;

    lhs_init(&l,100,TRUE,NULL);

    int i;
    char* ch;

    for (i = 0; i < 10; i++)
    {
        ch = malloc(80);
        itoa(i,ch,10);
        lhs_add( &l, ele_new(ch), lhs_hash( &l, ch, strlen(ch)) );
    }



    lhs_debug(&l);

    lhs_free(&l);




    return 0;
}

#endif
