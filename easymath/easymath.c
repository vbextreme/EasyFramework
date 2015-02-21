#define _USE_MATH_DEFINES
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "easymath.h"

float mth_gtor(float gradi)
{
    return ((gradi*MTH_2PI)/MTH_ANGLE_FULL);
}

void mth_initrandom()
{
    srand((unsigned)time(NULL));
}

int mth_random(int n)
{
    return rand() % (n+1);
}

int mth_randomrange(int min,int max)
{
    return min + (rand() % (max-min+1));
}

MTHMAP mth_randominitext(int min,int max)
{
    int extn = (max-min)+1;
    MTHMAP map =(MTHMAP) malloc(sizeof(int) * extn);
    memset(map,0,sizeof(int) * extn);
    return map;
}

void mth_randomfreeext(MTHMAP map)
{
    free(map);
}

int mth_randomextractor(MTHMAP map,int min,int max)
{
    int trials = 0;
    unsigned int candidate;

    int i;
    for (i = 0; i < max-min ; i++)
        if (!map[i]) {trials = 1;break;}

    while (trials)
    {
        candidate = mth_randomrange(min,max);
        if (!map[candidate-min])
        {
            map[candidate-min] = 1;
            return candidate;
        }
    }

    return -1;
}

void mth_randomstr(char *s,int nc)
{
    int i;
    for (i=0;i<nc-1;i++)
    {
        *s++=(char)mth_randomrange('a','z');
    }
    *s='\0';
}

void mth_randomstrnum(char *s,int nc)
{
    int i;
    for (i=0;i<nc;i++)
    {
        *s++=(char)mth_randomrange('0','9');
    }
    *s='\0';
}

float mth_randomgauss(float media, float stddeviation)
{
	float x1, x2, w, y1;
	static float y2;
	static int use_last = 0;

	if (use_last)
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0 * mth_randomf01() - 1.0;
			x2 = 2.0 * mth_randomf01() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return (media + y1 * stddeviation);
}

float mth_randomf01()
{
    return ((double)(rand()) / ((double)RAND_MAX + 1.0));
}

void mth_rotate(float *x,float *y,float centerx,float centery,float rad)
{
    float crad=cos(rad);
    float srad=sin(rad);

    double nx= *x - centerx;
    double ny= *y - centery;
    *x=((nx * crad) - (ny * srad)) + centerx;
    *y=((nx * srad) + (ny * crad)) + centery;
}

VOID mth_date_timettodate(time_t s,DATE* d)
{
    struct tm* timeinfo;

    timeinfo = localtime (&s);
    d->y = timeinfo->tm_year;
    d->m = timeinfo->tm_mon + 1;
    d->d = timeinfo->tm_mday;
    d->hh = timeinfo->tm_hour;
    d->mm = timeinfo->tm_min;
    d->ss = timeinfo->tm_sec;
}

VOID mth_date_timespectodate(struct timespec* s,DATE* d)
{
    struct tm* timeinfo;
    timeinfo = localtime (&s->tv_sec);
    d->y = timeinfo->tm_year + 1900;
    d->m = timeinfo->tm_mon + 1;
    d->d = timeinfo->tm_mday;
    d->hh = timeinfo->tm_hour;
    d->mm = timeinfo->tm_min;
    d->ss = timeinfo->tm_sec;
}

VOID mth_date_totimet(DATE* s,time_t* d)
{
    time_t raw;
    struct tm* ti;
    time (&raw);
    ti = localtime(&raw);
    ti->tm_year = s->y - 1900;
    ti->tm_mon = s->m - 1;
    ti->tm_mday = s->d;
    ti->tm_hour = s->hh;
    ti->tm_min = s->mm;
    ti->tm_sec = s->ss;
    *d = mktime(ti);
}

INT32 mth_date_cmp(DATE* a, DATE* b)
{
	if( a->y < b->y ) return -1;
	if( a->y > b->y ) return 1;
	if( a->m < b->m ) return -1;
	if( a->m > b->m ) return 1;
	if( a->d < b->d ) return -1;
	if( a->d > b->d ) return 1;
	if( a->hh < b->hh ) return -1;
	if( a->hh > b->hh ) return 1;
	if( a->mm < b->mm ) return -1;
	if( a->mm > b->mm ) return 1;
	if( a->ss < b->ss ) return -1;
	if( a->ss > b->ss ) return 1;
	return 0;
}

VOID mth_date_diff(DATE* d, DATE* a, DATE* b)
{
	DATE* mj;
	DATE* mi;
	
	switch (mth_date_cmp(a,b))
	{
		case 0:	memset(d,0,sizeof(DATE)); return;
		case -1: mi = a; mj = b; break;
		case 1:	mi = b;	mj = a;	break;
		default: return;
	}
	
	d->y = mj->y - mi->y;
	d->m = mj->m - mi->m;
	d->d = mj->d - mi->d;
	d->hh = mj->hh - mi->hh;
	d->mm = mj->mm - mi->mm;
	d->ss = mj->ss - mi->ss;
}

CHAR* mth_date_tostring(CHAR* s, DATE* d)
{
	sprintf(s,"%d/%d/%d %d:%d:%d",d->d,d->m,d->y,d->hh,d->mm,d->ss);
	return s;
}

VOID mth_date_fromstring(DATE* d, CHAR* s)
{
	memset(d,0,sizeof(DATE));
	
	CHAR part[5];
	
	CHAR* cp =part;
	while ( *s && *s != '/' ) *cp++ = *s++;
	*cp = '\0';
	d->d = atoi(part);
	if ( !*s++ ) return;
	
	cp = part;
	while ( *s && *s != '/' ) *cp++ = *s++;
	*cp = '\0';
	d->m = atoi(part);
	if ( !*s++ ) return;
	
	cp = part;
	while ( *s && *s != ' ' ) *cp++ = *s++;
	*cp = '\0';
	d->y = atoi(part);
	if ( !*s++ ) return;
	
	cp = part;
	while ( *s && *s != ':' ) *cp++ = *s++;
	*cp = '\0';
	d->hh = atoi(part);
	if ( !*s++ ) return;
	
	cp = part;
	while ( *s && *s != ':' ) *cp++ = *s++;
	*cp = '\0';
	d->mm = atoi(part);
	if ( !*s++ ) return;
	
	cp = part;
	while ( *s && *s != ' ') *cp++ = *s++;
	*cp = '\0';
	d->ss = atoi(part);
}



int mth_date_isbise(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int mth_date_nday(int month,int year)
{
    switch (month)
    {
        case 2:
            return 28 + mth_date_isbise(year);
        break;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        break;
        default:
            return 31;

    }

    return -1;
}

void mth_date_julianjodate(TIMEPLACE* now, double jd)
{
    long jdi, b;
    long c,d,e,g,g1;

    jd += 0.5;
    jdi = jd;
    if (jdi > 2299160) {
        long a = (jdi - 1867216.25)/36524.25;
        b = jdi + 1 + a - a/4;
    }
    else b = jdi;

    c = b + 1524;
    d = (c - 122.1)/365.25;
    e = 365.25 * d;
    g = (c - e)/30.6001;
    g1 = 30.6001 * g;
    now->day = c - e - g1;
    now->hour = (jd - jdi) * 24.0;
    if (g <= 13) now->month = g - 1;
    else now->month = g - 13;
    if (now->month > 2) now->year = d - 4716;
    else now->year = d - 4715;
}

double mth_date_julian(int year,int month,double day)
{
    /*
      Returns the number of julian days for the specified day.
      */

    int a,b=0,c,e;
    if (month < 3)
    {
        year--;
        month += 12;
    }
    if (year > 1582 || (year == 1582 && month>10) || (year == 1582 && month==10 && day > 15))
    {
        a=year/100;
        b=2-a+a/4;
    }
    c = 365.25*year;
    e = 30.6001*(month+1);
    return b+c+e+day+1720994.5;
}

double mth_sun_position(double j)
{
    double n,x,e,l,dl,v;
    //double m2;
    int i;

    n=360/365.2422*j;
    i=n/360;
    n=n-i*360.0;
    x=n-3.762863;
    if (x<0) x += 360;
    x *= MTH_RAD;
    e=x;
    do {
	dl=e-.016718*sin(e)-x;
	e=e-dl/(1-.016718*cos(e));
    } while (fabs(dl)>=MTH_SMALL_FLOAT);
    v=360/MTH_PI*atan(1.01686011182*tan(e/2));
    l=v+282.596403;
    i=l/360;
    l=l-i*360.0;
    return l;
}

double mth_moon_position(double j, double ls)
{

    double ms,l,mm,n,ev,sms,ae,ec;
    //double d;
    //double ds, as, dm;
    int i;

    /* ls = sun_position(j) */
    ms = 0.985647332099*j - 3.762863;
    if (ms < 0) ms += 360.0;
    l = 13.176396*j + 64.975464;
    i = l/360;
    l = l - i*360.0;
    if (l < 0) l += 360.0;
    mm = l-0.1114041*j-349.383063;
    i = mm/360;
    mm -= i*360.0;
    n = 151.950429 - 0.0529539*j;
    i = n/360;
    n -= i*360.0;
    ev = 1.2739*sin((2*(l-ls)-mm)*MTH_RAD);
    sms = sin(ms*MTH_RAD);
    ae = 0.1858*sms;
    mm += ev-ae- 0.37*sms;
    ec = 6.2886*sin(mm*MTH_RAD);
    l += ev+ec-ae+ 0.214*sin(2*mm*MTH_RAD);
    l= 0.6583*sin(2*(l-ls)*MTH_RAD)+l;
    return l;
}

double mth_moon_phase(int year,int month,int day, double hour, int* ip)
{
    double j= mth_date_julian(year,month,(double)day+hour/24.0)-2444238.5;
    double ls = mth_sun_position(j);
    double lm = mth_moon_position(j, ls);

    double t = lm - ls;
    if (t < 0) t += 360;
    *ip = (int)((t + 22.5)/45) & 0x7;
    return (1.0 - cos((lm - ls)*MTH_RAD))/2;
}

int mth_jdate(int d,int m,int y)
{
    int mm, yy;
    int k1, k2, k3;
    int j;

    yy = y - (int)((12 - m) / 10);
    mm = m + 9;
    if (mm >= 12)
    {
        mm = mm - 12;
    }
    k1 = (int)(365.25 * (yy + 4712));
    k2 = (int)(30.6001 * mm + 0.5);
    k3 = (int)((int)((yy / 100) + 49) * 0.75) - 38;
    // 'j' for dates in Julian calendar:
    j = k1 + k2 + d + 59;
    if (j > 2299160)
    {
        // For Gregorian calendar:
        j = j - k3; // 'j' is the Julian date at 12h UT (Universal Time)
    }
    return j;
}

double mth_moonphase(int d,int m,int y)
{
    int nxdmf = -1;
    int nxdmn = -1;
    int nxhmf = -1;
    int nxhmn = -1;
    int id,ih;
    int daym = mth_date_nday(m,y);
    double vmoon;
    int ip;

    for (id = d; id <= daym ; id++)
    {
        for (ih = 0; ih < 23 ;ih++)
        {
            vmoon = mth_moon_phase(y,m,id,ih,&ip);
            if ( ip == 0)
            {
                if (vmoon  < MTH_MOON_NEW)
                {
                    nxdmn = id;
                    nxhmn = ih;
                    break;
                }
            }
            else if (ip == 4)
            {
                if (vmoon  > MTH_MOON_FULL)
                {
                    nxdmf = id;
                    nxhmf = ih;
                    break;
                }
            }
            else
                {break;}
        }
    }

    if (nxdmn == d)
    {
        return 0.0;//moon_phase(y,m,d,nxhmn,&ip);
    }
    else if (nxdmf == d)
    {
        return 1.0; //+ moon_phase(y,m,d,nxhmf,&ip);
    }

    if (nxdmn == -1)
    {
        if (nxdmf == -1)
        {
            double tmp;
            vmoon = mth_moon_phase(y,m,d,10,&ip);
            if (ip > 4)
            {
                 vmoon = 1.0 + (1.0 - vmoon);
            }
            else if (ip == 4)
            {
                if ( d > 1)
                {
                    tmp = mth_moon_phase(y,m,d-1,10,&ip);
                    if (vmoon < tmp ) vmoon = 1.0 + (1.0 - vmoon);
                }
                else
                {
                    tmp = mth_moon_phase(y,m,d+1,10,&ip);
                    if (vmoon > tmp ) vmoon = 1.0 + (1.0 - vmoon);
                }
            }

            return vmoon;
        }

        vmoon = mth_moon_phase(y,m,d,nxhmf,&ip);
        if (d > nxdmf ) vmoon = 1.0 + (1.0 - vmoon);
        return vmoon;
    }
    else
    {
        if (nxdmf == -1)
        {
            vmoon = mth_moon_phase(y,m,d,nxhmn,&ip);
            vmoon = 1.0 + (1.0 - vmoon);
            return vmoon;
        }

        ///giorno piu vicino
        if (nxdmn < nxdmf)
        {
            vmoon = mth_moon_phase(y,m,d,nxhmn,&ip);
            vmoon = 1.0 + (1.0 - vmoon);
        }
        else
        {
            vmoon = mth_moon_phase(y,m,d,nxhmf,&ip);
        }

        //if (ip > 4) vmoon = 1.0 + (1.0 - vmoon);
        return vmoon;
    }

    return -1;
}




void mth_mat_addsi(SIMATRIX* r,SIMATRIX a,SIMATRIX b,unsigned int szr,unsigned int szc)
{
    SIMATRIX c = &r[0][0];

    unsigned int row,col;

    for (row=0; row < szr ; row++)
    {
        for (col=0; col < szc ; col++)
        {
            c[row][col] = a[row][col] + b[row][col];
        }
    }
}

void mth_mat_subsi(SIMATRIX* r,SIMATRIX a,SIMATRIX b,unsigned int szr,unsigned int szc)
{
    SIMATRIX c = &r[0][0];

    unsigned int row,col;

    for (row=0; row < szr ; row++)
    {
        for (col=0; col < szc ; col++)
        {
            c[row][col] = a[row][col] - b[row][col];
        }
    }
}

void mth_mat_imulsi(SIMATRIX* r,signed int a,SIMATRIX b,unsigned int szr,unsigned int szc)
{
    SIMATRIX c = &r[0][0];

    unsigned int row,col;

    for (row=0; row < szr ; row++)
    {
        for (col=0; col < szc ; col++)
        {
            c[row][col] = a * b[row][col];
        }
    }
}

void mth_mat_mulsi(SIMATRIX* r,SIMATRIX a,SIMATRIX b,unsigned int szracb,unsigned int szcarb)
{
    SIMATRIX c = &r[0][0];

    unsigned int rowa,colb,rc;

    for (rowa=0; rowa < szracb ; rowa++)
    {
        for (colb=0; colb < szracb ;colb++)
        {
            for (c[rowa][colb]=0, rc=0; rc < szcarb ;rc++)
            {
                c[rowa][colb] += a[rowa][rc] * b[rc][colb];
            }
        }
    }
}


signed int mth_mat_determinant2(SIMATRIX a)
{
    return a[0][0] * a[1][1] - a[0][1] * a[1][0];
}

signed int mth_mat_determinant3(SIMATRIX a)
{
    return a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) +
           a[0][1] * (a[1][2] * a[2][0] - a[1][0] * a[2][2]) +
           a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]) ;
}


void mth_fqr_generate(short int* buffer,int samplerate,double durata,double stfq,int loopfq,double fq,double amplitude,int fase)
{

    int szb = samplerate * durata;
    double byteforp = (double)samplerate / fq;
    double v;
    int i;
    int fqsample;

    if (loopfq == 0)
    {
        fqsample = szb;
    }
    else
    {
        fqsample = ((double)szb / (durata / (1/fq))) * (double)loopfq;
    }

    if (fase != 0)
    {
        fase = ((double)szb / (durata / (1/fq))) / fase;
    }

    for (i = samplerate * stfq; i < szb ; i++)
    {
        fase++;

        v = 2.0 * MTH_PI;
        v = amplitude * sin((double)fase * v / byteforp);

        if ( v > 32766)
            buffer[i] = 32766;
        else if (v < -32766)
            buffer[i] = -32766;
        else
            buffer[i] = (short int) v;

        if (fase == fqsample) break;
    }

}

static void _fft(cplx buf[], cplx out[], int n, int step)
{
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);

        int i;
		for (i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * PI * i / n) * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

void mth_fft(cplx buf[], int n)
{
    cplx* out = (cplx*) malloc(sizeof(cplx) * n);
	//cplx out[n];
	int i;
	for (i = 0; i < n; i++) out[i] = buf[i];

	_fft(buf, out, n, 1);

    free(out);
}
