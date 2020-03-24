#include "test.h"
#include <ef/ftp.h>

/*@test -t --ftp 'test ftp'*/

void ftp_ls(char* site){	
	ftp_s* ftp = ftp_new(4096, WWW_FLAGS_SSL, NULL);
    if( !ftp ){
		err_print();
		return;
	}

	ftp_site(ftp, site);
	ftp_cd(ftp, "/");

	ftpStat_s* lst = ftp_list(ftp);
	if( !lst ){
		err_print();
		return;
	}

	vector_foreach(lst, i){
		printf("[%4x]%s\n", lst[i].mode, lst[i].name);
	}

	ftp_stat_vector_free(lst);
	ftp_free(ftp);
}


/*@fn*/
void test_ftp(__unused const char* argA, __unused const char* argB){
	err_enable();
	
	ftp_ls("ftp://ftp.gnu.org");	

	err_restore();
}
