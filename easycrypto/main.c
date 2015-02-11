#ifdef _APP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "easycrypto.h"

static char *msg[] =
{
    "",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789012" \
        "345678901234567890"
};

static char *val[] =
{
    "d41d8cd98f00b204e9800998ecf8427e",
    "0cc175b9c0f1b6a831c399e269772661",
    "900150983cd24fb0d6963f7d28e17f72",
    "f96b697d7cb7938d525a2f31aaf161d0",
    "c3fcd3d76192e4007dfb496cca67e13b",
    "d174ab98d277d9f5a5611c2c9f419d9f",
    "57edf4a22be3c955ac49da2e2107b67a"
};

int main()
{
    ///MD5 USED
    char output[CRY_MD5_OUTSIZE];
    CRYMD5 ctx;

    printf( "\n MD5 Validation Tests:\n\n" );

    int i;
    for( i = 0; i < 7; i++ )
    {
        cry_md5_init(&ctx);
        cry_md5_calcolate(&ctx,(unsigned char*)msg[i],strlen(msg[i]));
        cry_md5_out(&ctx,output);
        printf(" %s ",output);

        if( memcmp( output, val[i], 32 ) )
        {
            printf( "failed!\n" );
            return( 1 );
        }

        printf( "passed.\n" );
    }

    printf( "\n" );
    printf( "\n" );

    ///AES USED

    unsigned char secretkey[CRY_AES_KEY_EASY] = {'a','z','9','q','1','r','g','h',
                                                 'j','A','P','k','M','0','O','v',
                                                };
    CRYAES act;
    cry_aes_setkey(&act,secretkey,CRY_AES_KEY_EASY);


    char* msg = malloc (10);
    strcpy(msg,"ciao");


    printf("Test AES:\n");
    printf("cript:%s\n",msg);

    cry_aes_encrypt(&act,(unsigned char*)msg,strlen(msg)+1);
    printf("->%s\n",msg);

    cry_aes_decrypt(&act,(unsigned char*)msg,strlen(msg)+1);
    printf("->%s\n",msg);

    return 0;
}
#endif
