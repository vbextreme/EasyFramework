#include <ef/sys.h>
#include <ef/config.h>
#include <ef/file.h>
#include <ef/str.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/proc.h>
#include <ef/trie.h>

#define PATH_SYS_CPU "/sys/devices/system/cpu"
#define PATHF_SYS_CPUFREQ "/sys/devices/system/cpu/cpu%lu/cpufreq/%s"
#define SYS_CPUFREQ_FCOUNT 7

__private int** cpufreqfd;
__private int cpufreqcpu;

#define PATHF_SYS_POWERSTAT "/sys/class/power_supply/%s/uevent"
__private trie_s* powerstattr;
__private powerstate_s powerstats;

#define PATHF_SYS_THERMAL "/sys/class/thermal/thermal_zone%u/temp"
#define PATHF_SYS_HWMON_CRITIC "/sys/class/hwmon/hwmon%u/temp%u_crit"

__private inline int cpufreq_idcpu(const char* name){
	if( strcmp(name, "cpu") ) return -1;
	name += 3;
	if( *name < '0' || *name > '9' ) return -1;
	++name;
	char* en;
	int ret = strtol(name, &en, 10);
	if( !en ) return -1;
	if( *en != 0 ) return -1;
	return ret;
}

err_t cpufreq_begin(void){
	static char* fname[] = {
		"scaling_cur_freq",
		"scaling_min_freq",
		"scaling_max_freq",
		"cpuinfo_min_freq",
		"cpuinfo_max_freq",
		"scaling_governor",
		"scaling_available_governors",
		NULL
	};

	cpufreqcpu = cpu_core_count();
	//dbg_info("cpu count:%d", cpufreqcpu);
	if( cpufreqcpu < 1 ) return -1;

	cpufreqfd = mem_zero_many(int*, cpufreqcpu);
	if( !cpufreqfd ){
		err_pushno("malloc");
		return -1;
	}

	for( size_t j = 0; (int)j < cpufreqcpu; ++j){
		cpufreqfd[j] = mem_many(int, SYS_CPUFREQ_FCOUNT); 
		if( !cpufreqfd[j] ){
			err_pushno("malloc");
			cpufreq_end();
			return -1;
		}
		for( size_t i = 0; i < SYS_CPUFREQ_FCOUNT; ++i){
			cpufreqfd[j][i] = -1;
		}
	}

	for( size_t i = 0; (int)i < cpufreqcpu; ++i){
		for( size_t j = 0; fname[j]; ++j){
			char path[PATH_MAX];
			sprintf(path, PATHF_SYS_CPUFREQ, i, fname[j]);
			//dbg_info("cpu[%lu].open:%s", i, path);
			cpufreqfd[i][j] = fd_open(path, "r", 0);
			if( cpufreqfd[i][j] < 0 ){
				err_pushno("open %s", path);
				cpufreq_end();
				return -1;
			}
		}
	}

	return 0;
}

err_t cpufreq_end(void){
	for( size_t i = 0; (int)i < cpufreqcpu; ++i){
		if( !cpufreqfd[i] ) continue;
		for(int j = 0; j < SYS_CPUFREQ_FCOUNT; ++j){
			if( cpufreqfd[i][j] > 0 ) fd_close(cpufreqfd[i][j]);
		}
		free(cpufreqfd[i]);
	}
	free(cpufreqfd);
	return -1;
}

err_t cpufreq_property_long_get(long* out, const size_t cpu, const size_t property){
	if( cpu > (size_t)cpufreqcpu - 1 ) return -1;
	if( property > SYS_CPUFREQ_FCOUNT - 1 ) return -1;
	
	char* en;
	char buf[80];
	ssize_t nr;
	if( (nr=fd_read(cpufreqfd[cpu][property], buf, 80)) < 1 || nr > 32 ){
		err_push("error on read property(%lu)", property);
		return -1;
	}
	buf[nr] = 0;
	*out = strtoul(buf, &en, 10);
	if( !en || *en != '\n' || *en != 0 ){
		err_push("incorrect value");
		return -1;
	}
	fd_seek(cpufreqfd[cpu][property], 0, SEEK_SET);
	return 0;
}

err_t cpufreq_property_string_get(char** out, const size_t cpu, const size_t property){
	if( cpu > (size_t)cpufreqcpu - 1 ) return -1;
	if( property > SYS_CPUFREQ_FCOUNT - 1 ) return -1;
	
	char buf[4096];
	ssize_t nr;
	if( (nr=fd_read(cpufreqfd[cpu][property], buf, 4095)) < 1 || nr > 4095 ){
		err_push("error on read property(%lu)", property);
		return -1;
	}
	buf[nr] = 0;
	fd_seek(cpufreqfd[cpu][property], 0, SEEK_SET);

	*out = str_dup(buf, nr);
	return *out ? 0 : -1;
}

char** cpufreq_property_vector_string_get(const size_t cpu, const size_t property){
	if( cpu > (size_t)cpufreqcpu - 1 ) return NULL;
	if( property > SYS_CPUFREQ_FCOUNT - 1 ) return NULL;
	
	char buf[4096];
	ssize_t nr;
	if( (nr=fd_read(cpufreqfd[cpu][property], buf, 4095)) < 1 || nr > 4095 ){
		err_pushno("read preperty(%lu)", property);
		return NULL;
	}
	fd_seek(cpufreqfd[cpu][property], 0, SEEK_SET);

	char** out = vector_new(char*, 8, free);
	if( !out ) return NULL;

	char* parse = buf;
	char* en;
	while( *parse && (en=strpbrk(parse, " \n")) ){
		if( en == parse ){
			++parse;
			continue;
		}
		parse[en-parse] = 0;
		char* nw = str_dup(parse, en-parse);
		parse = en + 1;
		if( nw == NULL ){
			continue;
		}
		vector_push_back(out, nw);
	}

	return out;
}

__private int powerstat_set_sizet(__unused int type, char** value, void* userdata){
	//dbg_info("set size_t:%s", value);
	size_t* lo = userdata;
	if( value ) *lo = strtoul(*value, NULL, 10);
	return 0;
}

__private int powerstat_set_string(__unused int type, char** value, void* userdata){
	//dbg_info("set string:%s", value);
	char** str = (char**)userdata;
	if( value ){
		*str = *value;
		*value = NULL;
	}
	else{
		*str = NULL;
	}
	return 0;
}

__private configTrie_s* powerstat_conf_new(int type, void* addr){
	configTrie_s* ct = mem_new(configTrie_s);
	if( ct == NULL ) err_fail("malloc");
	ct->fn = type == 0 ? powerstat_set_sizet : powerstat_set_string;
	ct->type = type;
	ct->userdata = addr; 
	return ct;
}   

__private void powerstat_trie_free(void* userdata){
	free(userdata);
}

void powerstate_begin(void){
	struct autoset{
		const char* name;
		const int type;
		void* addr;
	} as[] = {
		{"POWER_SUPPLY_VOLTAGE_MIN_DESIGN", 0, &powerstats.voltageMin },
		{"POWER_SUPPLY_VOLTAGE_NOW", 0, &powerstats.voltageNow },
		{"POWER_SUPPLY_ENERGY_FULL", 0, &powerstats.energyFull },
		{"POWER_SUPPLY_ENERGY_NOW", 0, &powerstats.energyNow },
		{"POWER_SUPPLY_POWER_NOW", 0, &powerstats.powerNow },
		{"POWER_SUPPLY_CAPACITY", 0, &powerstats.capacity },
		{"POWER_SUPPLY_STATUS", 1, &powerstats.status},
		{ NULL, -1, NULL },
	};
 
	powerstattr = trie_new(powerstat_trie_free);
	if( NULL == powerstattr ) return;

	for( size_t i = 0; as[i].name; ++i){
		trie_insert(powerstattr, as[i].name, powerstat_conf_new(as[i].type, as[i].addr));
	}
}

void powerstate_end(void){
	trie_free(powerstattr);
	powerstattr = NULL;
}

err_t powerstate_get(powerstate_s* ps, const char* device){
	if( powerstattr == NULL ){
		err_push("powerstat is not initialized");
		return -1;
	}

	free(powerstats.status);
	memset(&powerstats, 0, sizeof(powerstate_s));

	if( strlen(PATHF_SYS_CPUFREQ) + strlen(device) > PATH_MAX ){
		err_push("path to long");
		return -1;
	}
	char path[PATH_MAX];
	sprintf(path, PATHF_SYS_POWERSTAT, device);
	//dbg_info("open path %s", path);

	__stream_close stream_s* sm = stream_open(path, "r", 0, 4096);
	if( !sm ){
		dbg_error("can't open path");
		return -1;
	}

	if( config_parse(powerstattr, sm) < 0 ){
		dbg_error("on config parse");
		return -1;
	}

	//dbg_info("copy data with status:%s", powerstats.status);
	*ps = powerstats;
	ps->timeleft = ps->powerNow == 0 ? 0 : (double)ps->energyNow / (double)ps->powerNow;
	
	return 0;
}

ssize_t thermal_get(unsigned thermalzone){
	char path[PATH_MAX];
	char buf[4096];
	char* en;
	ssize_t ret;

	sprintf(path, PATHF_SYS_THERMAL, thermalzone);
	__fd_close int fd = fd_open(path, "r", 0);
	if( fd < 0 ) return -1;
	if( (ret=fd_read(fd, buf, 4096)) < 1 ) return -1;
	buf[ret] = 0;

	ret = strtol(buf, &en, 10);
	if( !en || *en != '\n' ) return -1;
	return ret;
}

ssize_t thermal_critic_get(unsigned hwmon, unsigned temp){
	char path[PATH_MAX];
	char buf[4096];
	char* en;
	ssize_t ret;

	sprintf(path, PATHF_SYS_HWMON_CRITIC, hwmon, temp);
	__fd_close int fd = fd_open(path, "r", 0);
	if( fd < 0 ) return -1;
	if( (ret=fd_read(fd, buf, 4096)) < 1 ) return -1;
	buf[ret] = 0;

	ret = strtol(buf, &en, 10);
	if( !en || *en != '\n' ) return -1;
	return ret;
}











