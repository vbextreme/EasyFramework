#include <ef/sig.h>
#include <ef/err.h>

err_t os_signal_get_status(sigset_t* mask){
	if( sigprocmask(0, NULL, mask) < 0 ){
		err_pushno("sigprocmask");
		return -1;
	}
	return 0;
}

err_t os_signal_set(sigaction_s* old, int num, signal_f fnc){
	sigaction_s sa = {
		.sa_sigaction = fnc,
		.sa_flags = SA_RESTART | SA_SIGINFO,
	};
	sigemptyset(&sa.sa_mask);
	
	if( sigaction(num, &sa, old) < 0 ){
		err_pushno("sigaction");
		return -1;
	}

	return 0;
}

err_t os_signal_restore(int num, sigaction_s* sa){
	if( sigaction(num, sa, NULL) < 0 ){
		err_pushno("sigaction");
		return -1;
	}
	return 0;
}

void signal_wait(void){
	pause();
}
