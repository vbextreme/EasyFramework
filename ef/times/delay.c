#include <ef/delay.h>
#include <ef/err.h>

#include <utime.h>
#include <time.h>
#include <sys/time.h>

size_t time_ms(void){
	struct timespec ts; 
	clock_gettime(CLOCK_REALTIME, &ts); 
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

size_t time_us(void){
	struct timespec ts; 
	clock_gettime(CLOCK_REALTIME, &ts); 
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

size_t time_cpu_ms(void){
	struct timespec ts; 
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts); 
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

size_t time_cpu_us(void){
	struct timespec ts; 
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts); 
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

double time_dbls(void){
	struct timespec ts;  
	clock_gettime(CLOCK_REALTIME, &ts); 
    return ts.tv_sec + ts.tv_nsec*1e-9;
}

err_t delay_ms(uint64_t ms){
	struct timespec tv;
	tv.tv_sec = (time_t) ms / 1000;
	tv.tv_nsec = (long) ((ms - (tv.tv_sec * 1000)) * 1000000L);

	while (1){
		int rval = nanosleep(&tv, &tv);
		if (rval == 0){
			return 0;
		}
		else if (errno == EINTR){
			dbg_warning("is correct recall nanosleep with same delay?");
			continue;
		}
		else{
			err_pushno("nanosleep fail");
			return -1;
		}
	}
	return 0;
}

err_t delay_us(uint64_t us){
	struct timespec tv;
	tv.tv_sec = (time_t) us / 1000000;
	tv.tv_nsec = (long) ((us - (tv.tv_sec * 1000000)) * 1000L);

	while (1){
		int rval = nanosleep (&tv, &tv);
		if (rval == 0){
			return 0;
		}
		else if (errno == EINTR){
			dbg_warning("is correct recall nanosleep with same delay?");
			continue;
		}
		else{
			err_pushno("nanosleep fail");
			return -1;
		}
	}
	return 0;
}

err_t delay_dbls(double s){
	struct timespec tv;
	tv.tv_sec = (time_t) s;
	tv.tv_nsec = (long) ((s - tv.tv_sec) * 1e+9);

	while (1)
	{
		int rval = nanosleep(&tv, &tv);
		if (rval == 0){
			return 0;
		}
		else if (errno == EINTR){
			dbg_warning("is correct recall nanosleep with same delay?");
			continue;
		}
		else{
			err_pushno("nanosleep fail");
			return -1;
		}
	}
	return 0;
}

void delay_hard(uint64_t us){
	uint32_t t = time_us();
	while( time_us() - t < us );
}


