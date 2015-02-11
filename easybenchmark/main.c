#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "easybenchmark.h"
int main()
{
    printf("Hello world!\n");


    double st = bch_get();

    usleep(10*1000);

    double en = bch_get();

    printf("%lf",bch_clc(st,en));

    return 0;
}

#endif

#include <sys/time.h>
#include <sys/resource.h>
#include "easybenchmark.h"

FLOAT64 bch_get()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

FLOAT64 bch_clc(FLOAT64 st, FLOAT64 en)
{
    return en-st;
}
