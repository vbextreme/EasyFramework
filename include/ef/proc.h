#ifndef __EF_PROC_H__
#define __EF_PROC_H__

#include <ef/type.h>
#include <ef/vector.h>

#define PATH_PROC "/proc"
#define PATH_PROC_STAT PATH_PROC "/stat"
#define PATH_PROC_MEM  PATH_PROC "/meminfo"
#define PATH_PROC_NET_DEV PATH_PROC "/net/dev"
#define PATHF_PROC_PID_FD PATH_PROC "/%d/fd"

//#define MEMINFO_ELEMS 8
//#define NET_DEV_NAME_MAX 256

typedef enum { CPU_USER, CPU_NICE, CPU_SYSTEM, CPU_IDLE, CPU_IOWAIT, CPU_IRQ, CPU_SOFTIRQ, CPU_STEAL, CPU_GUEST, CPU_GUEST_NICE, CPU_TIME_COUNT } cputime_e;

typedef struct memInfo{
	size_t total;
	size_t free;
	size_t available;
	size_t buffers;
	size_t cached;
	size_t totalswap;
	size_t freeswap;
	size_t shared;
	size_t SReclaimable;
	size_t SUnreclaim;
	size_t used;
	size_t toblink;
}memInfo_s;

typedef enum { ND_BYTES, ND_PACKETS, ND_ERRS, ND_DROP, ND_FIFO, ND_FRAME, ND_COMPRESSED, ND_MULTICAST, ND_COUNT } netDev_e;

typedef struct netDev {
	size_t receive[ND_COUNT];
	size_t transmit[ND_COUNT];
}netDev_s;

typedef struct pidStat{
	pid_t pid;
	char comm[PATH_MAX];
	char state;
	int ppid;
	int pgrp;
	int session;
	int ttynr;
	int tpgid;
	unsigned flags;
	unsigned long minflt;
	unsigned long cminflt;
	unsigned long majflt;
	unsigned long cmajflt;
	unsigned long utime;
	unsigned long stime;
	long cutime;
	long cstime;
	long priority;
	long nice;
	long numthreads;
	long itrealvalue;
	unsigned long statrtime;
	unsigned long vsize;
	long rss;
	unsigned long rsslim;
	unsigned long startcode;
	unsigned long endcode;
	unsigned long startstack;
	unsigned long kstkesp;
	unsigned long kstkeip;
	unsigned long signal;
	unsigned long blocked;
	unsigned long sigignore;
	unsigned long sigcatch;
	unsigned long wchan;
	unsigned long nswap;
    unsigned long cnswap;
	int exitsignal;
	int processor;
	unsigned rtpriority;
	unsigned policy;
	unsigned long delayacctblkioticks;
	unsigned long guesttime;
	long cguesttime;
	unsigned long startdata;
	unsigned long enddata;
	unsigned long startbrk;
	unsigned long argstart;
	unsigned long argend;
	unsigned long envstart;
	unsigned long envend;
	int exit_code;
}pidStat_s;

/** return numbers of cores*/
int cpu_core_count(void);

/**allocate memory for use with other tick function*/
size_t* cpu_tick_new(void);

/** get tick of cpu
 * @param tick is array of size CPU_TIME_COUNT * (ncores+1)
 * @param ncores number of core, if 0 call cpu_core_count
 * @return 0 successfull; -1 error
 */
err_t cpu_tick_get(size_t* tick, int ncores);

/** return sum of tick*/
size_t cpu_time_tick(size_t* tick);

/** return load average
 * @param tickS start tick
 * @param tickE end tick
 * @param core the core to view, 0 is all core
 * @param ncores count core, if 0 call cpu_core_count
 * @return load average
 */
double cpu_load_average(size_t* tickS, size_t* tickE, unsigned core, int ncores);

/** enable auto get average*/
void cpu_auto_load_average_begin(void);

/** disable auto get average*/
void cpu_auto_load_average_end(void);

/** get load average for cpu, if time ellapsed < 500ms delay 500ms */
double cpu_auto_load_average(unsigned core);

/** read memInfo
 * @param mem struct contains memory info
 * @return 0 successfull -1 error
 */
err_t meminfo_read(memInfo_s* mem);

/** fill netdev structure */
err_t net_device(netDev_s* net, const char* device);

/** get list of fd open in pid
 * @param pid the pid
 * @return vector or NULL for error
 */
int* proc_pid_fd(pid_t pid);

/** get /proc/pid/stat
 * @param ps structure where stored stat
 * @param pid pid to stat
 * @return -1 error 0 successfull
 */
err_t proc_pid_stat(pidStat_s* ps, pid_t pid);

#endif
