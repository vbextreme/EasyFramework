#ifndef EASYMATH_H_INCLUDED
#define EASYMATH_H_INCLUDED

#define EASYMATH_V a

#include <easytype.h>
#include <complex.h>

#define	MTH_PI	3.1415926535897932384626433832795
#define	MTH_RAD	(MTH_PI/180.0)
#define MTH_SMALL_FLOAT	(1e-12)

#define MTH_2PI	6.28318530717958647692
#define MTH_ANGLE_FULL 360

#define MTH_MOON_FULL 0.9999
#define MTH_MOON_NEW  0.0001

#define PI	M_PI	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)

typedef double complex cplx;
typedef unsigned int** UIMATRIX;
typedef signed int** SIMATRIX;
typedef int* MTHMAP;

typedef struct _TIMEPLACE
{
    int year,month,day;
    double hour;
} TIMEPLACE;



float mth_gtor(float gradi);
void mth_rotate(float *x,float *y,float centerx,float centery,float rad);

void mth_initrandom();
int mth_random(int n);
int mth_randomrange(int min,int max);

MTHMAP mth_randominitext(int min,int max);
int mth_randomextractor(MTHMAP map,int min,int max);
void mth_randomfreeext(MTHMAP map);

void mth_randomstr(char *s,int nc);
void mth_randomstrnum(char *s,int nc);
float mth_randomgauss(float media, float stddeviation);
float mth_randomf01();

VOID mth_date_timespectodate(struct timespec* s,DATE* d);
VOID mth_date_totimet(DATE* s,time_t* d);
INT32 mth_date_cmp(DATE* a, DATE* b);
VOID mth_date_diff(DATE* d, DATE* a, DATE* b);
CHAR* mth_date_tostring(CHAR* s, DATE* d);
VOID mth_date_fromstring(DATE* d, CHAR* s);

int mth_date_isbise(int year);
int mth_date_nday(int month,int year);
void mth_date_julianjodate(TIMEPLACE* now, double jd);
double mth_date_julian(int year,int month,double day);
double mth_sun_position(double j);
double mth_moon_position(double j, double ls);
double mth_moon_phase(int year,int month,int day, double hour, int* ip);
int mth_jdate(int d,int m,int y);
double mth_moonphase(int d,int m,int y);

void mth_mat_addsi(SIMATRIX* r,SIMATRIX a,SIMATRIX b,unsigned int szr,unsigned int szc);
void mth_mat_subsi(SIMATRIX* r,SIMATRIX a,SIMATRIX b,unsigned int szr,unsigned int szc);
void mth_mat_imulsi(SIMATRIX* r,signed int a,SIMATRIX b,unsigned int szr,unsigned int szc);
void mth_mat_mulsi(SIMATRIX* r,SIMATRIX a,SIMATRIX b,unsigned int szracb,unsigned int szcarb);
signed int mth_mat_determinant2(SIMATRIX a);
signed int mth_mat_determinant3(SIMATRIX a);


void mth_fqr_generate(short int* buffer,int samplerate,double durata,double stfq,int loopfq,double fq,double amplitude,int fase);
//cplx deve essere il doppio del buffer letto e deve essere una potenza di due.
//la frequenza sara:index*samplerate / sizefft
void mth_fft(cplx buf[], int n);

#endif // EASYMATH_H_INCLUDED
