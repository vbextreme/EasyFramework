#include <ef/mth.h>
#include <ef/memory.h>
#include <ef/err.h>

double mth_gtor(double gradi){
    return ((gradi*MTH_2PI)/MTH_ANGLE_FULL);
}

void mth_random_begin(void){
    srand((unsigned)time(NULL));
}

int mth_random(int n){
    return rand() % n;
}

int mth_random_range(int min,int max){
    return min + (rand() % (max-min+1));
}

float mth_random_gauss(float media, float stddeviation){
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
			x1 = 2.0 * mth_random_f01() - 1.0;
			x2 = 2.0 * mth_random_f01() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return (media + y1 * stddeviation);
}

double mth_random_f01(void){
    return ((double)(rand()) / ((double)RAND_MAX + 1.0));
}

void mth_random_string_azAZ09(char* out, size_t size){
	static char const * const azAZ09 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVQXYZ0123456789";
	size--;
	while( size-->0 ){
		size_t r = mth_random(62);
		*out++ = azAZ09[r];
	}
	*out = 0;
}

void mth_rotate(float *x,float *y,float centerx,float centery,float rad){
    float crad=cos(rad);
    float srad=sin(rad);
    double nx= *x - centerx;
    double ny= *y - centery;
    *x=((nx * crad) - (ny * srad)) + centerx;
    *y=((nx * srad) + (ny * crad)) + centery;
}

time_t mth_date_julian_time(double jd){
    struct tm date = {0};
	
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
    date.tm_wday = c - e - g1;
    date.tm_hour = (jd - jdi) * 24.0;
    if (g <= 13) date.tm_mon = g - 1;
    else date.tm_mon = g - 13;
    if (date.tm_mon > 2) date.tm_year = (d - 4716)-1900;
    else date.tm_year = (d - 4715)-1900;
	return mktime(&date);	
}

double mth_date_julian(int year,int month,double day){
    int a,b=0,c,e;
    if (month < 3){
        year--;
        month += 12;
    }
    if (year > 1582 || (year == 1582 && month>10) || (year == 1582 && month==10 && day > 15)){
        a=year/100;
        b=2-a+a/4;
    }
    c = 365.25*year;
    e = 30.6001*(month+1);
    return b+c+e+day+1720994.5;
}

int mth_date_julian_ut(int d,int m,int y){
    int mm, yy;
    int k1, k2, k3;
    int j;
    yy = y - (int)((12 - m) / 10);
    mm = m + 9;
    if (mm >= 12){
        mm = mm - 12;
    }
    k1 = (int)(365.25 * (yy + 4712));
    k2 = (int)(30.6001 * mm + 0.5);
    k3 = (int)((int)((yy / 100) + 49) * 0.75) - 38;
    // 'j' for dates in Julian calendar:
    j = k1 + k2 + d + 59;
    if (j > 2299160){
        // For Gregorian calendar:
        j = j - k3; // 'j' is the Julian date at 12h UT (Universal Time)
    }
    return j;
}


double mth_sun_position(double j){
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

double mth_moon_position(double j, double ls){
    double ms,l,mm,ev,sms,ae,ec;
    int i;
    ms = 0.985647332099*j - 3.762863;
    if (ms < 0) ms += 360.0;
    l = 13.176396*j + 64.975464;
    i = l/360;
    l = l - i*360.0;
    if (l < 0) l += 360.0;
    mm = l-0.1114041*j-349.383063;
    i = mm/360;
    mm -= i*360.0;
    //n = 151.950429 - 0.0529539*j;
    //i = n/360;
    //n -= i*360.0;
    ev = 1.2739*sin((2*(l-ls)-mm)*MTH_RAD);
    sms = sin(ms*MTH_RAD);
    ae = 0.1858*sms;
    mm += ev-ae- 0.37*sms;
    ec = 6.2886*sin(mm*MTH_RAD);
    l += ev+ec-ae+ 0.214*sin(2*mm*MTH_RAD);
    l= 0.6583*sin(2*(l-ls)*MTH_RAD)+l;
    return l;
}

double mth_moon_phase(int year,int month,int day, double hour, int* ip){
    double j= mth_date_julian(year,month,(double)day+hour/24.0)-2444238.5;
    double ls = mth_sun_position(j);
    double lm = mth_moon_position(j, ls);
    double t = lm - ls;
    if (t < 0) t += 360;
    if( ip ) *ip = (int)((t + 22.5)/45) & 0x7;
    return (1.0 - cos((lm - ls)*MTH_RAD))/2;
}

void mth_mat_addi(int** r, int** a, int** b, size_t szr, size_t szc){
    for(size_t row=0; row < szr; row++){
        for( size_t col=0; col < szc ; col++){
            r[row][col] = a[row][col] + b[row][col];
        }
    }
}

void mth_mat_subi(int** r, int** a, int** b, size_t szr, size_t szc){
    for(size_t row=0; row < szr; row++){
        for( size_t col=0; col < szc ; col++){
            r[row][col] = a[row][col] - b[row][col];
        }
    }
}

void mth_mat_imuli(int** r, int a, int** b, size_t szr, size_t szc){
    for(size_t row=0; row < szr; row++){
        for( size_t col=0; col < szc ; col++){
            r[row][col] = a * b[row][col];
        }
    }
}

void mth_mat_muli(int** r, int** a, int** b, size_t szr, size_t szc){
    for(size_t row=0; row < szr; row++){
        for( size_t col=0; col < szc ; col++){
			r[row][col] = 0;
			for( size_t rc = 0; rc < szc; ++rc){
	            r[row][col] += a[row][col] * b[row][col];
			}
        }
    }
}

int mth_mat_determinant2(int** a){
    return a[0][0] * a[1][1] - a[0][1] * a[1][0];
}

int mth_mat_determinant3(int** a){
    return a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) +
           a[0][1] * (a[1][2] * a[2][0] - a[1][0] * a[2][2]) +
           a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]) ;
}

void mth_fqr_generate(short int* buffer,int samplerate,double durata,double stfq,int loopfq,double fq,double amplitude,int fase){
    int szb = samplerate * durata;
    double byteforp = (double)samplerate / fq;
    double v;
    int i;
    int fqsample;

    if (loopfq == 0){
        fqsample = szb;
    }
    else{
        fqsample = ((double)szb / (durata / (1/fq))) * (double)loopfq;
    }

    if (fase != 0){
        fase = ((double)szb / (durata / (1/fq))) / fase;
    }

    for (i = samplerate * stfq; i < szb ; i++){
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

__private void _fft(double complex buf[], double complex out[], int n, int step){
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);

        int i;
		for (i = 0; i < n; i += 2 * step) {
			double complex t = cexp(-I * MTH_PI * i / n) * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

double complex* mth_fft(double complex buf[], int n){
    double complex* out = (double complex*) malloc(sizeof(double complex) * n);
	_fft(out, buf, n, 1);
	return out;
}

double mth_bbppigreco(long int i){
	double k;
	double sum = 0;
	if( i <= 0) { return -1.0; }
	for(k=0; k<i; ++k)
		sum = sum + ((4/(8*k+1))-(2/(8*k+4))-(1/(8*k+5))-(1/(8*k+6)))*(1/(pow(16,k)));
	return sum;
}

__const size_t mth_round_up_power_two(size_t n){
	if( n < 3 ) return 2;
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	++n;
	return n;
}

#define KILO (1000UL)
size_t mth_si_prefix_translate_base(const sip_e sibase){
	static const size_t base[] = {
		[MTH_SIP_ONE]   = 1UL,
		[MTH_SIP_DECA]  = 10UL,
		[MTH_SIP_HECTO] = 100UL,
		[MTH_SIP_KILO]  = KILO,
		[MTH_SIP_MEGA]  = KILO*KILO,
		[MTH_SIP_GIGA]  = KILO*KILO*KILO,
		[MTH_SIP_TERA]  = KILO*KILO*KILO*KILO,
		[MTH_SIP_PETA]  = KILO*KILO*KILO*KILO*KILO,
		[MTH_SIP_EXA]   = KILO*KILO*KILO*KILO*KILO*KILO,
		[MTH_SIP_ZETTA] = KILO*KILO*KILO*KILO*KILO*KILO*KILO,
		[MTH_SIP_YOTTA] = KILO*KILO*KILO*KILO*KILO*KILO*KILO*KILO
	};
	return base[sibase];
}

const char* mth_si_prefix_translate_short_string(const sip_e sibase){
	static const char* base[] = {
		[MTH_SIP_ONE]   = "  ",
		[MTH_SIP_DECA]  = "da",
		[MTH_SIP_HECTO] = "h ",
		[MTH_SIP_KILO]  = "k ",
		[MTH_SIP_MEGA]  = "M ",
		[MTH_SIP_GIGA]  = "G ",
		[MTH_SIP_TERA]  = "T ",
		[MTH_SIP_PETA]  = "P ",
		[MTH_SIP_EXA]   = "E ",
		[MTH_SIP_ZETTA] = "Z ",
		[MTH_SIP_YOTTA] = "Y "
	};
	return base[sibase];
}

double mth_si_prefix_base(sip_e* siout, const double num){
	if( num <= 0 ){
		if( siout ) *siout = 0;	
		return num;
	}
	size_t si = MTH_SIP_YOTTA;
	double base;
	while( num < (base = mth_si_prefix_translate_base(si)) ){
		--si;
	}
	if( siout ) *siout = si;
	return num / base;
}

#define KiBYTE (1024UL)

size_t mth_iec_prefix_translate_base(const iecp_e b){
	static size_t base[] = {
		[MTH_IEC_BYTE]      = 1UL,
		[MTH_IEC_KILOBYTE]  = KiBYTE,
		[MTH_IEC_MEGABYTE]  = KiBYTE*KiBYTE,
		[MTH_IEC_GIGABYTE]  = KiBYTE*KiBYTE*KiBYTE,
		[MTH_IEC_TERABYTE]  = KiBYTE*KiBYTE*KiBYTE*KiBYTE,
		[MTH_IEC_PETABYTE]  = KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE,
		[MTH_IEC_EXABYTE]   = KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE,
		[MTH_IEC_ZETTABYTE] = KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE,
		[MTH_IEC_YOTTABYTE] = KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE*KiBYTE
	};
	return base[b];
}

const char* mth_iec_prefix_translate_short_string(const iecp_e b){
	static const char* base[] = {
		[MTH_IEC_BYTE]      = "B  ",
		[MTH_IEC_KILOBYTE]  = "KiB",
		[MTH_IEC_MEGABYTE]  = "MiB",
		[MTH_IEC_GIGABYTE]  = "GiB",
		[MTH_IEC_TERABYTE]  = "TiB",
		[MTH_IEC_PETABYTE]  = "PiB",
		[MTH_IEC_EXABYTE]   = "EiB",
		[MTH_IEC_ZETTABYTE] = "ZiB",
		[MTH_IEC_YOTTABYTE] = "YiB"
	};
	return base[b];
}

double mth_ice_prefix_base(iecp_e* iecout, const double num){
	if( num <= 0 ){
		if( iecout ) *iecout = 0;	
		return num;
	}
	size_t iec = MTH_IEC_EXABYTE;
	double base;
	while( num < (base = mth_iec_prefix_translate_base(iec)) ){
		--iec;
	}
	if( iecout ) *iecout = iec;
	return num / base;
}

__private const unsigned char base64et[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
__private const unsigned char base64dt[80] = {
	['A' - '+'] = 0,  ['B' - '+'] = 1,  ['C' - '+'] = 2,  ['D' - '+'] = 3,	['E' - '+'] = 4,  ['F' - '+'] = 5,  ['G' - '+'] = 6,
	['H' - '+'] = 7,  ['I' - '+'] = 8,  ['J' - '+'] = 9,  ['K' - '+'] = 10, ['L' - '+'] = 11, ['M' - '+'] = 12, ['N' - '+'] = 13,
	['O' - '+'] = 14, ['P' - '+'] = 15, ['Q' - '+'] = 16, ['R' - '+'] = 17, ['S' - '+'] = 18, ['T' - '+'] = 19,	['U' - '+'] = 20, 
	['V' - '+'] = 21, ['W' - '+'] = 22, ['X' - '+'] = 23, ['Y' - '+'] = 24, ['Z' - '+'] = 25, 
	['a' - '+'] = 26, ['b' - '+'] = 27, ['c' - '+'] = 28, ['d' - '+'] = 29, ['e' - '+'] = 30, ['f' - '+'] = 31, ['g' - '+'] = 32, 
	['h' - '+'] = 33, ['i' - '+'] = 34, ['j' - '+'] = 35, ['k' - '+'] = 36, ['l' - '+'] = 37, ['m' - '+'] = 38, ['n' - '+'] = 39,
	['o' - '+'] = 40, ['p' - '+'] = 41, ['q' - '+'] = 42, ['r' - '+'] = 43,	['s' - '+'] = 44, ['t' - '+'] = 45,	['u' - '+'] = 46, 
	['v' - '+'] = 47, ['w' - '+'] = 48, ['x' - '+'] = 49, ['y' - '+'] = 50, ['z' - '+'] = 51, 
	['0' - '+'] = 52, ['1' - '+'] = 53, ['2' - '+'] = 54, ['3' - '+'] = 55, ['4' - '+'] = 56, ['5' - '+'] = 57, ['6' - '+'] = 58, 
	['7' - '+'] = 59, ['8' - '+'] = 60, ['9' - '+'] = 61,
	['+' - '+'] = 62, ['/' - '+'] = 63
};

char* base64_encode(const void* src, const size_t size){
	size_t una = size % 3;
	size_t ali = (size - una) / 3;
	una = 3 - una;
	const char* data = src;
	const size_t outsize = (size * 4) / 3 + 4;

	dbg_info("size:%lu outsize:%lu unaligned:%lu counter:%lu", size, outsize, una, ali);

	char* ret = mem_many(char, outsize);
	if( !ret ){
		err_pushno("eom");
		return NULL;
	}

	char* next = ret;

	while( ali --> 0 ){
		*next++ = base64et[data[0] >> 2];
		*next++ = base64et[((data[0] & 0x03) << 4) | (data[1] >> 4)];
		*next++ = base64et[((data[1] & 0x0F) << 2) | (data[2] >> 6)];
		*next++ = base64et[data[2] & 0x3F];
		data += 3;
	}
	
	switch( una ){
		case 1:
			*next++ = base64et[data[0] >> 2];
			*next++ = base64et[((data[0] & 0x03) << 4) | (data[1] >> 4)];
			*next++ = base64et[((data[1] & 0x0F) << 2)];
			*next++ = '=';
		break;

		case 2:
			*next++ = base64et[data[0] >> 2];
			*next++ = base64et[((data[0] & 0x03) << 4)];
			*next++ = '=';
			*next++ = '=';
		break;
	}

	*next = 0;

	return ret;
}

void* base64_decode(size_t* size, const char* b64){
	const unsigned char* str = (const unsigned char*)b64;
	const size_t len = strlen(b64);
	size_t una = 0;
	if( str[len - 1] == '=' ) ++una;
	if( str[len - 2] == '=' ) ++una;
	
	const size_t countali = (len / 4) * 3 - una;
	if( size ) *size = countali;

	void* data = mem_many(char, countali);
	if( !data ){
		err_pushno("eom");
		return NULL;
	}
	char* next = data;

	size_t count = (len / 4) - 1;

	dbg_info("len:%lu count:%lu cali:%lu una:%lu", len, count, countali, una);

	while( count --> 0 ){
		*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
		*next++ = (base64dt[str[1] - '+'] << 4) | (base64dt[str[2] - '+'] >> 2);
		*next++ = (base64dt[str[2] - '+'] << 6) |  base64dt[str[3] - '+'];
		str += 4;
	}

	switch( una ){
		case 0:
			*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
			*next++ = (base64dt[str[1] - '+'] << 4) | (base64dt[str[2] - '+'] >> 2);
			*next++ = (base64dt[str[2] - '+'] << 6) |  base64dt[str[3] - '+'];
		break;
		case 1:
			*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
			*next++ = (base64dt[str[1] - '+'] << 4) | (base64dt[str[2] - '+'] >> 2);
			//*next++ = (base64dt[str[2]] << 6);
		break;
		case 2:
			*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
			//*next++ = (base64dt[str[1]] << 4) | (base64dt[str[2]] >> 2);
			//*next++ = (base64dt[str[2]] << 6) |  base64dt[str[3]];
		break;
	}
	
	return data;
}












