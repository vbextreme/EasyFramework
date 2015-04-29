#ifndef EASYBENCHMARK_H_INCLUDED
#define EASYBENCHMARK_H_INCLUDED

#include <easytype.h>
#include <stdio.h>

#define BCH_FLAG_START 0x01
#define BCH_FLAG_PAUSE 0x02
#define BCH_FLAG_STOP  0x04

#define BCH_PERF_GLOBAL BCHPERF __perf__
#define BCH_PERF_INIT __perf__ = bch_perf_init()
#define BCH_PERF_START bch_perf_add(__perf__,__FUNCTION__,BCH_FLAG_START)
#define BCH_PERF_PAUSE bch_perf_add(__perf__,__FUNCTION__,BCH_FLAG_PAUSE)
#define BCH_PERF_STOP  bch_perf_add(__perf__,__FUNCTION__,BCH_FLAG_STOP)
#define BCH_PERF_SAVE(FILENAME)  bch_perf(fopen(FILENAME,"w"),__perf__)

typedef struct __BCHPERF* BCHPERF;

FLOAT64 bch_get();
FLOAT64 bch_clc(FLOAT64 st, FLOAT64 en);

BCHPERF bch_perf_init();
VOID bch_perf_add(BCHPERF p,const CHAR* fname, BYTE flags);
BOOL bch_perf(FILE* out, BCHPERF p);

#endif 
