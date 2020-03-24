#ifndef __EF_MATH_H__
#define __EF_MATH_H__

#define _USE_MATH_DEFINES
#include <ef/type.h>
#include <math.h>
#include <complex.h>
#include <time.h>

/** PI*/
#define MTH_PI	M_PI
/** PI*2 */
#define MTH_2PI	(2.0*MTH_PI)
/** radiant */
#define	MTH_RAD	(MTH_PI/180.0)
/**small float*/
#define MTH_SMALL_FLOAT	(1e-12)
/** full degree angle*/
#define MTH_ANGLE_FULL 360

#define MTH_DOUBLE_CMP(A,B,R) ((A) > (B) - (R) && (A) < (B) + (R))

/** get max from two numbers
 * @param A
 * @param B
 */
#define MTH_MAX(A,B) ((A>B)?A:B)

/** get max from three numbers
 * @param A
 * @param B
 * @param C
 */
#define MTH_3MAX(A,B,C) MTH_MAX(MTH_MAX(A,B),C)

/** get min from two numbers
 * @param A
 * @param B
 */
#define MTH_MIN(A,B) ((A<B)?A:B)

/** get min from three numbers
 * @param A
 * @param B
 * @param C
 */
#define MTH_3MIN(A,B,C) MTH_MIN(MTH_MIN(A,B),C)

/** round number to up value
 * @param N is number
 * @param S is to round
 */
#define ROUND_UP(N,S) ((((N)+(S)-1)/(S))*(S))

/** round number to up value as power of two, example 16 is 16, 17 is 32
 * @param N is number
 */
#define ROUND_UP_POW_TWO32(N) ({\
	   	unsigned int r = (N);\
	   	--r;\
		r |= r >> 1;\
		r |= r >> 2;\
		r |= r >> 4;\
		r |= r >> 8;\
		r |= r >> 16;\
		++r;\
		r;\
	})

/** round number to down value as power of two, example 16 is 16, 17 is 16
 * @param N is number
 */
#define ROUND_DOWN_POW_TWO32(N) ({\
	   	unsigned int r = ROUND_UP_POW_TWO32((N)+1);\
		r >> 1;\
	})

/** fast version to get modulo of pwer of two value
 * @param N is number
 * @param M is modulo
 */
#define FAST_MOD_POW_TWO(N,M) ((N) & ((M) - 1))

/** fast way to count bit
 * @param B is value to counting bit
 */
#define FAST_BIT_COUNT(B) __builtin_popcount(B)


/** get alpha value for continuate median
 * @see MM_NEXT
 * @param CHANNEL is offset value, 2000
 * @param COUNT is wight of value
 * @return alpha value
 */
#define MM_ALPHA(CHANNEL,COUNT) ((CHANNEL)/((COUNT)+1))
/** calcolate ahpla
 * @see MM_NEXT
 * @param CHANNEL is offset value, 2000
 * @param ALPHA is wight of value
 * @return ahpla value
 */
#define MM_AHPLA(CHANNEL,ALPHA) ((CHANNEL/2)-ALPHA)
/** calcolate next median value
 * @param CHANNEL a channel
 * @param ALPHA get with MM_ALPHA
 * @param AHPLA get with MM_AHPLA
 * @param NEWVAL new value
 * @param OLDVAL previus median
 * @return new median
 * @code
 * const long alpha = MM_ALPHA(2000, 5);
 * const long ahpla = MM_AHPLA(2000, alpha);
 * long median = 0;
 * while(1){
 *  long newval = anyfunctiontogetit();
 *	long median = MM_NEXT(alpha, ahpla, newval,  median);
 *	printf("median:%ld",medianNew);
 * }
 * @endcode
 */
#define MM_NEXT(CHANNEL,ALPHA,AHPLA,NEWVAL,OLDVAL) (((ALPHA)*(NEWVAL)+(AHPLA)*(OLDVAL))/(CHANNEL/2))

/** unsigned long lut bit, get index
 * @param VAL
 */
#define ULLBIT_INDEX(VAL) ((VAL)/(sizeof(unsigned long)*8))

/** unsigned long lut bit, get bit
 * @param VAL
 */
#define ULLBIT_BIT(VAL) ((unsigned long)1<<FAST_MOD_POW_TWO(VAL, sizeof(unsigned long)*8))

/** unsigned long lut bit, set bit in lut
 * @param V is array
 * @param VAL 
 */
#define ULLBITS(V, VAL) (V[ULLBIT_INDEX(VAL)] |= ULLBIT_BIT(VAL))

/** unsigned long lut bit, clear bit in lut
 * @param V is array
 * @param VAL
 */
#define ULLBITC(V, VAL) (V[ULLBIT_INDEX(VAL)] &= ~ULLBIT_BIT(VAL))

/** unsigned long lut bit, test bit in lut
 * @param V is array
 * @param VAL
 */
#define ULLBITT(V, VAL) (V[ULLBIT_INDEX(VAL)] & ULLBIT_BIT(VAL))


/** degree to radiant
 * @param gradi degree
 * @return radiant
 */
double mth_gtor(double gradi);

/** initialize random number*/
void mth_random_begin(void);

/** get random from 0 to N-1
 * @param n number
 * @return random
 */
int mth_random(int n);

/** get random in range, from min to max
 * @param min min value
 * @param max max value
 * @return random
 */
int mth_random_range(int min,int max);

/** get random gauss
 * @param media mediana value
 * @param stddeviation deviation
 * @return random
 */
float mth_random_gauss(float media, float stddeviation);

/** get random from 0.0 to 1.0
 * @return random
 */
double mth_random_f01(void);

/** get random string [a-zA-Z0-9] with size = size -1
 * @param size size string with 0
 * @param out output
 */
void mth_random_string_azAZ09(char* out, size_t size);

/** rotate a point
 * @param x x position, out new position here
 * @param y y position, out new position here
 * @param centerx x rotation center
 * @param centery y rotation center
 * @param rad radiant rotation
 */
void mth_rotate(float *x,float *y,float centerx,float centery,float rad);

/** convert julian date to time_t
 * @param jd julian date
 * @return time_t date
 */
time_t mth_date_julian_time(double jd);

/** convert yy mm dd in julian date
 * @param year
 * @param month
 * @param day dd+hh/24.0
 * @return julian date
 */
double mth_date_julian(int year,int month,double day);

/** convert yy mm dd in julian date
 * @param d day
 * @param m month
 * @param y year
 * @return julian date at 12h UT(universal Time)
 */
int mth_date_julian_ut(int d,int m,int y);

/** get sun position by julian date
 * @param j julian
 * @return sun position
 */
double mth_sun_position(double j);

/** get moon position by julian date and sun position
 * @param j julian
 * @param ls sun
 * @return sun position
 */
double mth_moon_position(double j, double ls);

/** get moon phase by date 
 * @param year
 * @param month
 * @param day
 * @param hour
 * @param ip out value
 * @return moon phase
 */
double mth_moon_phase(int year,int month,int day, double hour, int* ip);

/** sum matrix a to b and return in r
 * @param r out
 * @param a in
 * @param b in
 * @param szr row count
 * @param szc col count
 */
void mth_mat_addi(int** r, int** a, int** b, size_t szr, size_t szc);

/** sub matrix a to b and return in r
 * @param r out
 * @param a in
 * @param b in
 * @param szr row count
 * @param szc col count
 */
void mth_mat_subi(int** r, int** a, int** b, size_t szr, size_t szc);

/** mul int to matrix return in r
 * @param r out
 * @param a integer
 * @param b in
 * @param szr row count
 * @param szc col count
 */
void mth_mat_imuli(int** r, int a, int** b, size_t szr, size_t szc);

/** mul matrix a to b and return in r
 * @param r out
 * @param a in
 * @param b in
 * @param szr row count
 * @param szc col count
 */
void mth_mat_muli(int** r, int** a, int** b, size_t szr, size_t szc);

/** determinat of matrix
 * @param a in
 * @return determinant
 */
int mth_mat_determinant2(int** a);

/** determinat3 of matrix
 * @param a in
 * @return determinant3
 */
int mth_mat_determinant3(int** a);

/** determinat of matrix
 * @param buffer data
 * @param samplerate samplerate
 * @param durata lenght
 * @param stfq startfq
 * @param loopfq loop
 * @param fq frequency
 * @param amplitude amplitude
 * @param fase fase
 */
void mth_fqr_generate(short int* buffer,int samplerate,double durata,double stfq,int loopfq,double fq,double amplitude,int fase);

/** simpple fft
 * @param buf in
 * @param n size buffer
 * @return fft result, free memory after use
 */
double complex* mth_fft(double complex buf[], int n);

/** calcolate pi
 * @param i
 * @return pi
 */
double mth_bbppigreco(long int i);

/** round number to up value as power of two
 * @param n is number
 */
__const size_t mth_round_up_power_two(size_t n);

/** International_System_of_Units*/
typedef enum{ MTH_SIP_ONE, MTH_SIP_DECA, MTH_SIP_HECTO, MTH_SIP_KILO, MTH_SIP_MEGA, MTH_SIP_GIGA, MTH_SIP_TERA, MTH_SIP_PETA, MTH_SIP_EXA, MTH_SIP_ZETTA, MTH_SIP_YOTTA } sip_e;

/** translate si to base*/
size_t mth_si_prefix_translate_base(const sip_e sibase);

/** translate to short string form*/
const char* mth_si_prefix_translate_short_string(const sip_e sibase);

/** move number to base
 * @param siout the si base
 * @param num number to translate
 * @return a base on si
 */
double mth_si_prefix_base(sip_e* siout, const double num);

/**  International Electrotechnical Commission */
typedef enum{ MTH_IEC_BYTE, MTH_IEC_KILOBYTE, MTH_IEC_MEGABYTE, MTH_IEC_GIGABYTE, MTH_IEC_TERABYTE, MTH_IEC_PETABYTE, MTH_IEC_EXABYTE, MTH_IEC_ZETTABYTE, MTH_IEC_YOTTABYTE } iecp_e;

/** translate iec to base*/
size_t mth_iec_prefix_translate_base(const iecp_e b);

/** translate to short string form*/
const char* mth_iec_prefix_translate_short_string(const iecp_e b);

/** move number to base
 * @param iecout the iec base
 * @param num number to translate
 * @return a base on iec
 */
double mth_ice_prefix_base(iecp_e* iecout, const double num);

/** encode data to base64 string
 * @param src data sources
 * @param size size of data
 * @return a new string or null if error, remember to free
 */
char* base64_encode(const void* src, const size_t size);

/** deencode string b64 to data
 * @param size out size of decoded data
 * @param b64 base64 string
 * @return a new data or null if error, remember to free
 */
void* base64_decode(size_t* size, const char* b64);

#endif
