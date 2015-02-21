#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "easycrypto.h"

/// //// ///
/// HASH ///
/// //// ///

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
    #define g16b(d) (*((const UINT16 *) (d)))
#endif

#if !defined (g16b)
    #define g16b(d) ((((UINT32)(((const BYTE *)(d))[1])) << 8) + (UINT32)(((const BYTE *)(d))[0]) )
#endif

UINT32 cry_fasthash(CHAR* data, INT32 len)
{
    UINT32 hash = len;
    UINT32 tmp;
    INT32 rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--)
    {
        hash  += g16b(data);
        tmp    = (g16b(data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2 * sizeof(INT16);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += g16b (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += g16b(data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

UINT32 cry_hash(CHAR* val, INT32 len, UINT32 maxmap)
{
    return cry_fasthash(val,len) % maxmap;
}



/// ///////////////////// ///
/// CRITTOGRAFIA HASH MD5 ///
/// ///////////////////// ///

#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (unsigned long int) (b)[(i)    ]       )       \
        | ( (unsigned long int) (b)[(i) + 1] <<  8 )       \
        | ( (unsigned long int) (b)[(i) + 2] << 16 )       \
        | ( (unsigned long int) (b)[(i) + 3] << 24 );      \
}

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (unsigned char) ( (n)       );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 3] = (unsigned char) ( (n) >> 24 );       \
}


void cry_md5_init(CRYMD5* ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

void md5_process( CRYMD5 *ctx, unsigned char data[64] )
{
    unsigned long int X[16], A, B, C, D;

    GET_UINT32( X[0],  data,  0 );
    GET_UINT32( X[1],  data,  4 );
    GET_UINT32( X[2],  data,  8 );
    GET_UINT32( X[3],  data, 12 );
    GET_UINT32( X[4],  data, 16 );
    GET_UINT32( X[5],  data, 20 );
    GET_UINT32( X[6],  data, 24 );
    GET_UINT32( X[7],  data, 28 );
    GET_UINT32( X[8],  data, 32 );
    GET_UINT32( X[9],  data, 36 );
    GET_UINT32( X[10], data, 40 );
    GET_UINT32( X[11], data, 44 );
    GET_UINT32( X[12], data, 48 );
    GET_UINT32( X[13], data, 52 );
    GET_UINT32( X[14], data, 56 );
    GET_UINT32( X[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a,b,c,d,k,s,t)                                \
{                                                       \
    a += F(b,c,d) + X[k] + t; a = S(a,s) + b;           \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];

#define F(x,y,z) (z ^ (x & (y ^ z)))

    P( A, B, C, D,  0,  7, 0xD76AA478 );
    P( D, A, B, C,  1, 12, 0xE8C7B756 );
    P( C, D, A, B,  2, 17, 0x242070DB );
    P( B, C, D, A,  3, 22, 0xC1BDCEEE );
    P( A, B, C, D,  4,  7, 0xF57C0FAF );
    P( D, A, B, C,  5, 12, 0x4787C62A );
    P( C, D, A, B,  6, 17, 0xA8304613 );
    P( B, C, D, A,  7, 22, 0xFD469501 );
    P( A, B, C, D,  8,  7, 0x698098D8 );
    P( D, A, B, C,  9, 12, 0x8B44F7AF );
    P( C, D, A, B, 10, 17, 0xFFFF5BB1 );
    P( B, C, D, A, 11, 22, 0x895CD7BE );
    P( A, B, C, D, 12,  7, 0x6B901122 );
    P( D, A, B, C, 13, 12, 0xFD987193 );
    P( C, D, A, B, 14, 17, 0xA679438E );
    P( B, C, D, A, 15, 22, 0x49B40821 );

#undef F

#define F(x,y,z) (y ^ (z & (x ^ y)))

    P( A, B, C, D,  1,  5, 0xF61E2562 );
    P( D, A, B, C,  6,  9, 0xC040B340 );
    P( C, D, A, B, 11, 14, 0x265E5A51 );
    P( B, C, D, A,  0, 20, 0xE9B6C7AA );
    P( A, B, C, D,  5,  5, 0xD62F105D );
    P( D, A, B, C, 10,  9, 0x02441453 );
    P( C, D, A, B, 15, 14, 0xD8A1E681 );
    P( B, C, D, A,  4, 20, 0xE7D3FBC8 );
    P( A, B, C, D,  9,  5, 0x21E1CDE6 );
    P( D, A, B, C, 14,  9, 0xC33707D6 );
    P( C, D, A, B,  3, 14, 0xF4D50D87 );
    P( B, C, D, A,  8, 20, 0x455A14ED );
    P( A, B, C, D, 13,  5, 0xA9E3E905 );
    P( D, A, B, C,  2,  9, 0xFCEFA3F8 );
    P( C, D, A, B,  7, 14, 0x676F02D9 );
    P( B, C, D, A, 12, 20, 0x8D2A4C8A );

#undef F

#define F(x,y,z) (x ^ y ^ z)

    P( A, B, C, D,  5,  4, 0xFFFA3942 );
    P( D, A, B, C,  8, 11, 0x8771F681 );
    P( C, D, A, B, 11, 16, 0x6D9D6122 );
    P( B, C, D, A, 14, 23, 0xFDE5380C );
    P( A, B, C, D,  1,  4, 0xA4BEEA44 );
    P( D, A, B, C,  4, 11, 0x4BDECFA9 );
    P( C, D, A, B,  7, 16, 0xF6BB4B60 );
    P( B, C, D, A, 10, 23, 0xBEBFBC70 );
    P( A, B, C, D, 13,  4, 0x289B7EC6 );
    P( D, A, B, C,  0, 11, 0xEAA127FA );
    P( C, D, A, B,  3, 16, 0xD4EF3085 );
    P( B, C, D, A,  6, 23, 0x04881D05 );
    P( A, B, C, D,  9,  4, 0xD9D4D039 );
    P( D, A, B, C, 12, 11, 0xE6DB99E5 );
    P( C, D, A, B, 15, 16, 0x1FA27CF8 );
    P( B, C, D, A,  2, 23, 0xC4AC5665 );

#undef F

#define F(x,y,z) (y ^ (x | ~z))

    P( A, B, C, D,  0,  6, 0xF4292244 );
    P( D, A, B, C,  7, 10, 0x432AFF97 );
    P( C, D, A, B, 14, 15, 0xAB9423A7 );
    P( B, C, D, A,  5, 21, 0xFC93A039 );
    P( A, B, C, D, 12,  6, 0x655B59C3 );
    P( D, A, B, C,  3, 10, 0x8F0CCC92 );
    P( C, D, A, B, 10, 15, 0xFFEFF47D );
    P( B, C, D, A,  1, 21, 0x85845DD1 );
    P( A, B, C, D,  8,  6, 0x6FA87E4F );
    P( D, A, B, C, 15, 10, 0xFE2CE6E0 );
    P( C, D, A, B,  6, 15, 0xA3014314 );
    P( B, C, D, A, 13, 21, 0x4E0811A1 );
    P( A, B, C, D,  4,  6, 0xF7537E82 );
    P( D, A, B, C, 11, 10, 0xBD3AF235 );
    P( C, D, A, B,  2, 15, 0x2AD7D2BB );
    P( B, C, D, A,  9, 21, 0xEB86D391 );

#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
}

void cry_md5_calcolate(CRYMD5 *ctx, unsigned char* input, unsigned long int length )
{
    unsigned long int left, fill;

    if( ! length ) return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += length;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < length )
        ctx->total[1]++;

    if( left && length >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        md5_process( ctx, ctx->buffer );
        length -= fill;
        input  += fill;
        left = 0;
    }

    while( length >= 64 )
    {
        md5_process( ctx, input );
        length -= 64;
        input  += 64;
    }

    if( length )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, length );
    }
}

static unsigned char md5_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void cry_md5_out( CRYMD5 *ctx ,char* d)
{
    unsigned char digest[16];
    unsigned long int last, padn;
    unsigned long int high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_UINT32( low,  msglen, 0 );
    PUT_UINT32( high, msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    cry_md5_calcolate( ctx, md5_padding, padn );
    cry_md5_calcolate( ctx, msglen, 8 );

    PUT_UINT32( ctx->state[0], digest,  0 );
    PUT_UINT32( ctx->state[1], digest,  4 );
    PUT_UINT32( ctx->state[2], digest,  8 );
    PUT_UINT32( ctx->state[3], digest, 12 );

    int j;
    for( j = 0; j < 16; j++ )
    {
        sprintf( d + j * 2, "%02x", digest[j] );
    }
}

/// /////////////////////////// ///
/// CRITTOGRAFIA SIMMETRICA AES ///
/// /////////////////////////// ///

unsigned long int FSb[256];
unsigned long int FT0[256];
unsigned long int FT1[256];
unsigned long int FT2[256];
unsigned long int FT3[256];

/* reverse S-box & tables */

unsigned long int RSb[256];
unsigned long int RT0[256];
unsigned long int RT1[256];
unsigned long int RT2[256];
unsigned long int RT3[256];

/* round constants */

unsigned long int RCON[10];

/* tables generation flag */

int do_init = 1;

/* tables generation routine */

#define ROTR8(x) ( ( ( x << 24 ) & 0xFFFFFFFF ) | \
                   ( ( x & 0xFFFFFFFF ) >>  8 ) )

#define XTIME(x) ( ( x <<  1 ) ^ ( ( x & 0x80 ) ? 0x1B : 0x00 ) )
#define MUL(x,y) ( ( x &&  y ) ? pow[(log[x] + log[y]) % 255] : 0 )

void aes_gen_tables( void )
{
    int i;
    unsigned char x, y;
    unsigned char pow[256];
    unsigned char log[256];

    /* compute pow and log tables over GF(2^8) */

    for( i = 0, x = 1; i < 256; i++, x ^= XTIME( x ) )
    {
        pow[i] = x;
        log[x] = i;
    }

    /* calculate the round constants */

    for( i = 0, x = 1; i < 10; i++, x = XTIME( x ) )
    {
        RCON[i] = (unsigned long int) x << 24;
    }

    /* generate the forward and reverse S-boxes */

    FSb[0x00] = 0x63;
    RSb[0x63] = 0x00;

    for( i = 1; i < 256; i++ )
    {
        x = pow[255 - log[i]];

        y = x;  y = ( y << 1 ) | ( y >> 7 );
        x ^= y; y = ( y << 1 ) | ( y >> 7 );
        x ^= y; y = ( y << 1 ) | ( y >> 7 );
        x ^= y; y = ( y << 1 ) | ( y >> 7 );
        x ^= y ^ 0x63;

        FSb[i] = x;
        RSb[x] = i;
    }

    /* generate the forward and reverse tables */

    for( i = 0; i < 256; i++ )
    {
        x = (unsigned char) FSb[i]; y = XTIME( x );

        FT0[i] =   (unsigned long int) ( x ^ y ) ^
                 ( (unsigned long int) x <<  8 ) ^
                 ( (unsigned long int) x << 16 ) ^
                 ( (unsigned long int) y << 24 );

        FT0[i] &= 0xFFFFFFFF;

        FT1[i] = ROTR8( FT0[i] );
        FT2[i] = ROTR8( FT1[i] );
        FT3[i] = ROTR8( FT2[i] );

        y = (unsigned char) RSb[i];

        RT0[i] = ( (unsigned long int) MUL( 0x0B, y )       ) ^
                 ( (unsigned long int) MUL( 0x0D, y ) <<  8 ) ^
                 ( (unsigned long int) MUL( 0x09, y ) << 16 ) ^
                 ( (unsigned long int) MUL( 0x0E, y ) << 24 );

        RT0[i] &= 0xFFFFFFFF;

        RT1[i] = ROTR8( RT0[i] );
        RT2[i] = ROTR8( RT1[i] );
        RT3[i] = ROTR8( RT2[i] );
    }
}

/* decryption key schedule tables */

int KT_init = 1;

unsigned long int KT0[256];
unsigned long int KT1[256];
unsigned long int KT2[256];
unsigned long int KT3[256];

/* AES key scheduling routine */

int  cry_aes_setkey( CRYAES* ctx, unsigned char* secretkey, int tkey )
{
    unsigned char key[32];
    int nbits = tkey * 8;
    memcpy(key,secretkey,tkey);

    int i;
    unsigned long int *RK, *SK;

    if( do_init )
    {
        aes_gen_tables();

        do_init = 0;
    }

    switch( nbits )
    {
        case 128: ctx->nr = 10; break;
        case 192: ctx->nr = 12; break;
        case 256: ctx->nr = 14; break;
        default : return( 1 );
    }

    RK = ctx->erk;

    for( i = 0; i < (nbits >> 5); i++ )
    {
        GET_UINT32( RK[i], key, i * 4 );
    }

    /* setup encryption round keys */

    switch( nbits )
    {
    case 128:

        for( i = 0; i < 10; i++, RK += 4 )
        {
            RK[4]  = RK[0] ^ RCON[i] ^
                        ( FSb[ (unsigned char) ( RK[3] >> 16 ) ] << 24 ) ^
                        ( FSb[ (unsigned char) ( RK[3] >>  8 ) ] << 16 ) ^
                        ( FSb[ (unsigned char) ( RK[3]       ) ] <<  8 ) ^
                        ( FSb[ (unsigned char) ( RK[3] >> 24 ) ]       );

            RK[5]  = RK[1] ^ RK[4];
            RK[6]  = RK[2] ^ RK[5];
            RK[7]  = RK[3] ^ RK[6];
        }
        break;

    case 192:

        for( i = 0; i < 8; i++, RK += 6 )
        {
            RK[6]  = RK[0] ^ RCON[i] ^
                        ( FSb[ (unsigned char) ( RK[5] >> 16 ) ] << 24 ) ^
                        ( FSb[ (unsigned char) ( RK[5] >>  8 ) ] << 16 ) ^
                        ( FSb[ (unsigned char) ( RK[5]       ) ] <<  8 ) ^
                        ( FSb[ (unsigned char) ( RK[5] >> 24 ) ]       );

            RK[7]  = RK[1] ^ RK[6];
            RK[8]  = RK[2] ^ RK[7];
            RK[9]  = RK[3] ^ RK[8];
            RK[10] = RK[4] ^ RK[9];
            RK[11] = RK[5] ^ RK[10];
        }
        break;

    case 256:

        for( i = 0; i < 7; i++, RK += 8 )
        {
            RK[8]  = RK[0] ^ RCON[i] ^
                        ( FSb[ (unsigned char) ( RK[7] >> 16 ) ] << 24 ) ^
                        ( FSb[ (unsigned char) ( RK[7] >>  8 ) ] << 16 ) ^
                        ( FSb[ (unsigned char) ( RK[7]       ) ] <<  8 ) ^
                        ( FSb[ (unsigned char) ( RK[7] >> 24 ) ]       );

            RK[9]  = RK[1] ^ RK[8];
            RK[10] = RK[2] ^ RK[9];
            RK[11] = RK[3] ^ RK[10];

            RK[12] = RK[4] ^
                        ( FSb[ (unsigned char) ( RK[11] >> 24 ) ] << 24 ) ^
                        ( FSb[ (unsigned char) ( RK[11] >> 16 ) ] << 16 ) ^
                        ( FSb[ (unsigned char) ( RK[11] >>  8 ) ] <<  8 ) ^
                        ( FSb[ (unsigned char) ( RK[11]       ) ]       );

            RK[13] = RK[5] ^ RK[12];
            RK[14] = RK[6] ^ RK[13];
            RK[15] = RK[7] ^ RK[14];
        }
        break;
    }

    /* setup decryption round keys */

    if( KT_init )
    {
        for( i = 0; i < 256; i++ )
        {
            KT0[i] = RT0[ FSb[i] ];
            KT1[i] = RT1[ FSb[i] ];
            KT2[i] = RT2[ FSb[i] ];
            KT3[i] = RT3[ FSb[i] ];
        }

        KT_init = 0;
    }

    SK = ctx->drk;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;

    for( i = 1; i < ctx->nr; i++ )
    {
        RK -= 8;

        *SK++ = KT0[ (unsigned char) ( *RK >> 24 ) ] ^
                KT1[ (unsigned char) ( *RK >> 16 ) ] ^
                KT2[ (unsigned char) ( *RK >>  8 ) ] ^
                KT3[ (unsigned char) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (unsigned char) ( *RK >> 24 ) ] ^
                KT1[ (unsigned char) ( *RK >> 16 ) ] ^
                KT2[ (unsigned char) ( *RK >>  8 ) ] ^
                KT3[ (unsigned char) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (unsigned char) ( *RK >> 24 ) ] ^
                KT1[ (unsigned char) ( *RK >> 16 ) ] ^
                KT2[ (unsigned char) ( *RK >>  8 ) ] ^
                KT3[ (unsigned char) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (unsigned char) ( *RK >> 24 ) ] ^
                KT1[ (unsigned char) ( *RK >> 16 ) ] ^
                KT2[ (unsigned char) ( *RK >>  8 ) ] ^
                KT3[ (unsigned char) ( *RK       ) ]; RK++;
    }

    RK -= 8;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;

    return( 0 );
}

/* AES 128-bit block encryption routine */

void _cry_aes_encrypt( CRYAES* ctx, unsigned char input[16], unsigned char output[16] )
{
    unsigned long int *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->erk;

    GET_UINT32( X0, input,  0 ); X0 ^= RK[0];
    GET_UINT32( X1, input,  4 ); X1 ^= RK[1];
    GET_UINT32( X2, input,  8 ); X2 ^= RK[2];
    GET_UINT32( X3, input, 12 ); X3 ^= RK[3];

#define AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
    RK += 4;                                    \
                                                \
    X0 = RK[0] ^ FT0[ (unsigned char) ( Y0 >> 24 ) ] ^  \
                 FT1[ (unsigned char) ( Y1 >> 16 ) ] ^  \
                 FT2[ (unsigned char) ( Y2 >>  8 ) ] ^  \
                 FT3[ (unsigned char) ( Y3       ) ];   \
                                                \
    X1 = RK[1] ^ FT0[ (unsigned char) ( Y1 >> 24 ) ] ^  \
                 FT1[ (unsigned char) ( Y2 >> 16 ) ] ^  \
                 FT2[ (unsigned char) ( Y3 >>  8 ) ] ^  \
                 FT3[ (unsigned char) ( Y0       ) ];   \
                                                \
    X2 = RK[2] ^ FT0[ (unsigned char) ( Y2 >> 24 ) ] ^  \
                 FT1[ (unsigned char) ( Y3 >> 16 ) ] ^  \
                 FT2[ (unsigned char) ( Y0 >>  8 ) ] ^  \
                 FT3[ (unsigned char) ( Y1       ) ];   \
                                                \
    X3 = RK[3] ^ FT0[ (unsigned char) ( Y3 >> 24 ) ] ^  \
                 FT1[ (unsigned char) ( Y0 >> 16 ) ] ^  \
                 FT2[ (unsigned char) ( Y1 >>  8 ) ] ^  \
                 FT3[ (unsigned char) ( Y2       ) ];   \
}

    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 1 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 2 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 3 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 4 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 5 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 6 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 7 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 8 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 9 */

    if( ctx->nr > 10 )
    {
        AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 10 */
        AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 11 */
    }

    if( ctx->nr > 12 )
    {
        AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 12 */
        AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 13 */
    }

    /* last round */

    RK += 4;

    X0 = RK[0] ^ ( FSb[ (unsigned char) ( Y0 >> 24 ) ] << 24 ) ^
                 ( FSb[ (unsigned char) ( Y1 >> 16 ) ] << 16 ) ^
                 ( FSb[ (unsigned char) ( Y2 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (unsigned char) ( Y3       ) ]       );

    X1 = RK[1] ^ ( FSb[ (unsigned char) ( Y1 >> 24 ) ] << 24 ) ^
                 ( FSb[ (unsigned char) ( Y2 >> 16 ) ] << 16 ) ^
                 ( FSb[ (unsigned char) ( Y3 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (unsigned char) ( Y0       ) ]       );

    X2 = RK[2] ^ ( FSb[ (unsigned char) ( Y2 >> 24 ) ] << 24 ) ^
                 ( FSb[ (unsigned char) ( Y3 >> 16 ) ] << 16 ) ^
                 ( FSb[ (unsigned char) ( Y0 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (unsigned char) ( Y1       ) ]       );

    X3 = RK[3] ^ ( FSb[ (unsigned char) ( Y3 >> 24 ) ] << 24 ) ^
                 ( FSb[ (unsigned char) ( Y0 >> 16 ) ] << 16 ) ^
                 ( FSb[ (unsigned char) ( Y1 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (unsigned char) ( Y2       ) ]       );

    PUT_UINT32( X0, output,  0 );
    PUT_UINT32( X1, output,  4 );
    PUT_UINT32( X2, output,  8 );
    PUT_UINT32( X3, output, 12 );
}

/* AES 128-bit block decryption routine */

void _cry_aes_decrypt( CRYAES* ctx, unsigned char input[16], unsigned char output[16] )
{
    unsigned long int *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->drk;

    GET_UINT32( X0, input,  0 ); X0 ^= RK[0];
    GET_UINT32( X1, input,  4 ); X1 ^= RK[1];
    GET_UINT32( X2, input,  8 ); X2 ^= RK[2];
    GET_UINT32( X3, input, 12 ); X3 ^= RK[3];

#define AES_RROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
    RK += 4;                                    \
                                                \
    X0 = RK[0] ^ RT0[ (unsigned char) ( Y0 >> 24 ) ] ^  \
                 RT1[ (unsigned char) ( Y3 >> 16 ) ] ^  \
                 RT2[ (unsigned char) ( Y2 >>  8 ) ] ^  \
                 RT3[ (unsigned char) ( Y1       ) ];   \
                                                \
    X1 = RK[1] ^ RT0[ (unsigned char) ( Y1 >> 24 ) ] ^  \
                 RT1[ (unsigned char) ( Y0 >> 16 ) ] ^  \
                 RT2[ (unsigned char) ( Y3 >>  8 ) ] ^  \
                 RT3[ (unsigned char) ( Y2       ) ];   \
                                                \
    X2 = RK[2] ^ RT0[ (unsigned char) ( Y2 >> 24 ) ] ^  \
                 RT1[ (unsigned char) ( Y1 >> 16 ) ] ^  \
                 RT2[ (unsigned char) ( Y0 >>  8 ) ] ^  \
                 RT3[ (unsigned char) ( Y3       ) ];   \
                                                \
    X3 = RK[3] ^ RT0[ (unsigned char) ( Y3 >> 24 ) ] ^  \
                 RT1[ (unsigned char) ( Y2 >> 16 ) ] ^  \
                 RT2[ (unsigned char) ( Y1 >>  8 ) ] ^  \
                 RT3[ (unsigned char) ( Y0       ) ];   \
}

    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 1 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 2 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 3 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 4 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 5 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 6 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 7 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 8 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 9 */

    if( ctx->nr > 10 )
    {
        AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 10 */
        AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 11 */
    }

    if( ctx->nr > 12 )
    {
        AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 12 */
        AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 13 */
    }

    /* last round */

    RK += 4;

    X0 = RK[0] ^ ( RSb[ (unsigned char) ( Y0 >> 24 ) ] << 24 ) ^
                 ( RSb[ (unsigned char) ( Y3 >> 16 ) ] << 16 ) ^
                 ( RSb[ (unsigned char) ( Y2 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (unsigned char) ( Y1       ) ]       );

    X1 = RK[1] ^ ( RSb[ (unsigned char) ( Y1 >> 24 ) ] << 24 ) ^
                 ( RSb[ (unsigned char) ( Y0 >> 16 ) ] << 16 ) ^
                 ( RSb[ (unsigned char) ( Y3 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (unsigned char) ( Y2       ) ]       );

    X2 = RK[2] ^ ( RSb[ (unsigned char) ( Y2 >> 24 ) ] << 24 ) ^
                 ( RSb[ (unsigned char) ( Y1 >> 16 ) ] << 16 ) ^
                 ( RSb[ (unsigned char) ( Y0 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (unsigned char) ( Y3       ) ]       );

    X3 = RK[3] ^ ( RSb[ (unsigned char) ( Y3 >> 24 ) ] << 24 ) ^
                 ( RSb[ (unsigned char) ( Y2 >> 16 ) ] << 16 ) ^
                 ( RSb[ (unsigned char) ( Y1 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (unsigned char) ( Y0       ) ]       );

    PUT_UINT32( X0, output,  0 );
    PUT_UINT32( X1, output,  4 );
    PUT_UINT32( X2, output,  8 );
    PUT_UINT32( X3, output, 12 );
}

void cry_aes_encrypt( CRYAES* ctx, unsigned char *s, unsigned long int len )
{
    unsigned int r = len % 16;
    if ( r > 0)
        s = realloc(s,(len - r) + 16);

    int i;
    for (i = 0; i < len; i += 16,s+=16)
    {
        _cry_aes_encrypt(ctx,s,s);
    }
}

void cry_aes_decrypt( CRYAES* ctx, unsigned char *s, unsigned long int len )
{
    int i;
    for (i = 0; i < len; i += 16,s+=16)
    {
        _cry_aes_decrypt(ctx,s,s);
    }
}
