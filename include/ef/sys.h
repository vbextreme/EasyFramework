#ifndef __EF_SYSCLASS_H__
#define __EF_SYSCLASS_H__

#include <ef/type.h>
#include <ef/vector.h>

typedef struct powerstate{
	size_t voltageMin; /**< uV */
	size_t voltageNow; /**< uV */
	size_t energyFull; /**< uW/h */
	size_t energyNow; /**< uW/h */
	size_t powerNow; /**< uW */
	size_t capacity; /**< % */
	double timeleft; /**< s*/
	char* status; /** Charging Discharnging*/
}powerstate_s;

/** cpufreq property*/
#define SYS_CPUFREQ_SCALING_CURFQ 0
/** cpufreq property*/
#define SYS_CPUFREQ_SCALING_MINFQ 1
/** cpufreq property*/
#define SYS_CPUFREQ_SCALING_MAXFQ 2
/** cpufreq property*/
#define SYS_CPUFREQ_CPUINFO_MINFQ 3
/** cpufreq property*/
#define SYS_CPUFREQ_CPUINFO_MAXFQ 4
/** cpufreq property*/
#define SYS_CPUFREQ_GOV 5
/** cpufreq property*/
#define SYS_CPUFREQ_GOV_AV 6

err_t cpufreq_begin(void);
err_t cpufreq_end(void);

/** read long property of cpufreq
 * @param out out value
 * @param cpu cpu to read
 * @param property property of cpufreq
 * @return 0 successfull -1 error
 */
err_t cpufreq_property_long_get(long* out, const size_t cpu, const size_t property);

/** read string property of cpufreq
 * @param out value, remember to free
 * @param cpu cpu to read
 * @param property property of cpufreq
 * @return 0 successfull -1 error
 */
err_t cpufreq_property_string_get(char** out, const size_t cpu, const size_t property);

/** read string list separated of space on cpufreq
 * @param cpu cpu to read
 * @param property property name of cpufreq
 * @return vector of char*, free each element of vector, NULL for error
 */
char** cpufreq_property_vector_string_get(const size_t cpu, const size_t property);

#define cpufreq_property_get(OUT, CPU, PROPERTY) _Generic(OUT,\
		long*: cpufreq_property_long_get,\
		char**: cpufreq_property_string_get,\
		vector_s*: cpufreq_property_vector_string_get\
)(OUT, CPU, PROPERTY)

/******************/
/*** powerstate ***/
/******************/

/** begin powerstate, call this before get*/
void powerstate_begin(void);

/** powerstate end, call when not need more data*/
void powerstate_end(void);

/** get powerstate structure
 * @param ps powerstate structure
 * @param device name, /sys/class/power_supply/
 * @return 0 successful -1 error
 */
err_t powerstate_get(powerstate_s* ps, const char* device);

/*** thermal ***/

/** get thermal from /sys/class/thermal/thermal_zone\<N\>/temp*/
ssize_t thermal_get(unsigned thermalzone);

/** get thermal critic from /sys/class/hwmon/hwmon\<HWMON\>/temp\<TEMP\>_crit */
ssize_t thermal_critic_get(unsigned hwmon, unsigned temp);

#endif
