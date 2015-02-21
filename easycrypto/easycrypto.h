#ifndef EASYCRYPTO_H_INCLUDED
#define EASYCRYPTO_H_INCLUDED

#include <easytype.h>

#define CRY_MD5_OUTSIZE 33

#define CRY_AES_KEY_EASY    16 //128bit
#define CRY_AES_KEY_NORMAL  24 //192bit
#define CRY_AES_KEY_HARD    32 //256bit

/// HASH MD5
/// SIMMETRICA AES messaggio multiplo di 16

typedef struct _CRYMD5
{
    unsigned long int total[2];
    unsigned long int state[4];
    unsigned char buffer[64];
}CRYMD5;

typedef struct CRYAES
{
    unsigned long int erk[64];
    unsigned long int drk[64];
    int nr;
}CRYAES;

UINT32 cry_fasthash(CHAR* data, INT32 len);
UINT32 cry_hash(CHAR* val, INT32 len, UINT32 maxmap);

//inizializzare ogni volta che si vuole generare una nuova chiave
void cry_md5_init(CRYMD5* ctx);
//chiamare la funzione per ogni blocco di dati
void cry_md5_calcolate(CRYMD5 *ctx, unsigned char* input, unsigned long int length );
//alla fine out restituisce in d[33] la chiave md5
void cry_md5_out( CRYMD5 *ctx ,char* d);

//create a aeskey from secret key size and tkey is CRY_AES_KEY_
int  cry_aes_setkey( CRYAES* ctx, unsigned char* secretkey, int tkey );
//attenzione se s non è un multiplo di 16 verrà usata realloc per ridimensionare il vettore
void cry_aes_encrypt( CRYAES* ctx, unsigned char *s, unsigned long int len );
//decripta s
void cry_aes_decrypt( CRYAES* ctx, unsigned char *s, unsigned long int len );

#endif // EASYCRYPTO_H_INCLUDED
