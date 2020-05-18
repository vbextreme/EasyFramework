#include "test.h"
#include <ef/sys.h>
#include <ef/proc.h>
#include <ef/spawn.h>
#include <ef/os.h>

/*@test -o --os 'test os'*/

__private void printps(const char* name, size_t val){
	sip_e si;
	double v = mth_si_prefix_base(&si, val);
	printf("%s:%7.3f%s\n", name, v, mth_si_prefix_translate_short_string(si));
}

/*@fn*/
void test_os(__unused const char* argA, __unused const char* argB){
	sdbus_s* sd = sdbus_new();
	if( !sd ) err_fail("sdbus");

	sdbusBusNames_s* bn = sdbus_bus_names(sd);
	vector_foreach(bn->acquired,i){
		printf("acquired:'%s'\n", bn->acquired[i]);
	}
	vector_foreach(bn->activable, i){
		printf("activable:'%s'\n", bn->activable[i]);
	}
	sdbus_bus_names_free(bn);

	char* xml = sdbus_introspect_raw(sd, "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager");
	printf("<XML>%s</XML>\n", xml);
	//free(xml);

	sdbus_free(sd);
	return;
	
	cpu_auto_load_average_begin();	
	printf("core count:%d\n", cpu_core_count());
	
	double avg = cpu_auto_load_average(0);
	printf("load average:%f%%\n",avg);
	

	powerstate_begin();
	powerstate_s ps;
	if( powerstate_get(&ps, "BAT0") ){
		err_fail("read BAT0");
	}

	printps("capacity  ", ps.capacity);
	printps("energyFull", ps.energyFull);
	printps("energyNow ", ps.energyNow);
	printps("powerNow  ", ps.powerNow);
	printps("voltageMin", ps.voltageMin);
	printps("voltageNow", ps.voltageNow);
	printf("timeleft  :%f\n", ps.timeleft);
	printf("status    :%s\n", ps.status);

	powerstate_end();

	cpufreq_begin();
	size_t ncore = cpu_core_count();
	long v;
	for(size_t i = 0; i < ncore; ++i){
		cpufreq_property_get(&v, 0, SYS_CPUFREQ_SCALING_CURFQ);
		printps("cpu", v);
	}

	avg = cpu_auto_load_average(0);
	printf("load average:%f%%\n",avg);
	
	cpufreq_end();
	cpu_auto_load_average_end();

	printf("temp:%ld\n", thermal_get(0));
	printf("crit:%ld\n", thermal_critic_get(0,1));

	//spawn_disable_zombie();
	printf("<LS>\n");
	pid_t pid = spawn_shell("ls", 0);
	if( pid == -1 ){
		err_fail("spawn shell");
	}
	int ret = -1;
	if( spawn_wait(pid, &ret) ){
		dbg_fail("spawn wait");
	}
	printf("</LS return:%d>\n", ret);

	printf("<LS>\n");
	char* out;
	char* err;
	if( spawn_shell_slurp(&out, &err, &ret, "ls") ){
		err_fail("spawn shell");
	}
	printf("<OUT>\n");
	if( out ) puts(out);
	printf("</OUT>\n");
	printf("<ERR>\n");
	if( err ) puts(err);
	printf("</ERR>\n");
	printf("</LS return:%d>\n", ret);



	spawn_end();
}
