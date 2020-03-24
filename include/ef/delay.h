#ifndef __EF_DELAY_H__
#define __EF_DELAY_H__

#include <ef/type.h>
#include <sys/sysinfo.h>

/** get time in milliseconds from starting os */
size_t time_ms(void);

/** get time in microseconds from starting os */
size_t time_us(void);

/** get time in milliseconds only for cpu */
size_t time_cpu_ms(void);

/** get time in microseconds only for cpu*/
size_t time_cpu_us(void);

/** get time in seconds from starting os */
double time_dbls(void);

/** sleep milliseconds */
err_t delay_ms(uint64_t ms);

/** sleep microseconds */
err_t delay_us(uint64_t us);

/** sleep seconds as double */
err_t delay_dbls(double s);

/** wait microsecons without sleep
 * this function can be use 100% of cpu if the time is long
 */
void delay_hard(uint64_t us);

#endif
