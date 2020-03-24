#include <ef/spawn.h>
#include <ef/file.h>
#include <ef/proc.h>
#include <ef/str.h>
#include <ef/err.h>

#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>

extern char** environ;

const char* os_shell_get(void){
	static char shell[PATH_MAX] = {0};
	if( shell[0] ) return shell;
	const char* env = getenv("SHELL");
	if( !env || !*env || strlen(env) > PATH_MAX ){
		strcpy(shell, SPAWN_SHELL_PATH);
	}
	else{
		strcpy(shell, env);
	}
	return shell;
}

void spawn_disable_zombie(void){
	struct sigaction arg = {
		.sa_handler = SIG_DFL,
		.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_NOCLDWAIT
	};
	if( sigaction(SIGCHLD, &arg, NULL) ){
		dbg_error("sigaction");
		dbg_errno();
	}
}

void spawn_end(void){
	wait(NULL);
}

__private void spawn_close_files(int skipbefore){
	pid_t pid = getpid();
	__vector_free int* vfd = proc_pid_fd(pid);
	if( !vfd ){
		dbg_warning("fail to get fd");
		return;
	}
	vector_foreach(vfd, i){
		if( vfd[i] < skipbefore ) continue;
		close(vfd[i]);
	}
}

pid_t spawn_shell(const char* cmdline, int disableoutput){
	pid_t child = fork();
	switch( child ){
		case -1: return -1;
		case 0:{
			spawn_close_files(disableoutput ? 1 : 3);
			const char* shell = os_shell_get();
			execle(shell, shell, SPAWN_ARGUMENT, cmdline, NULL, environ);
			err_pushno("fail exevc '%s %s %s'", shell, SPAWN_ARGUMENT, cmdline);
		}
		return -1;

		default: return child;
	}
	
	return -1;
}

err_t spawn_shell_slurp(char** out, char** err, int* exitcode, const char* cmdline){
	int opipe[2];
	int epipe[2];
	
	*out = NULL;
	*err = NULL;

	if( pipe(opipe) ){
		err_pushno("opening opipe");
		return -1;
	}
	if( pipe(epipe) ){
		close(opipe[0]);
		close(opipe[1]);
		err_pushno("opening epipe");
		return -1;
	}

	pid_t child = fork();
	switch( child ){
		case -1: return -1;
		case 0:{
			dup2(epipe[1], STDERR_FILENO);
			close(epipe[0]);
			close(epipe[1]);
			dup2(opipe[1], STDOUT_FILENO);
			close(opipe[0]);
			close(opipe[1]);
			spawn_close_files(3);
			const char* shell = os_shell_get();
			execle(shell, shell, SPAWN_ARGUMENT, cmdline, NULL, environ);
			err_pushno("fail exevc '%s %s %s'", shell, SPAWN_ARGUMENT, cmdline);
		}
		return -1;
	}

	close(opipe[1]);
	close(epipe[1]);
	
	size_t lenOut;
	size_t lenErr;
	*out = fd_slurp(&lenOut, opipe[0], FILE_CHUNK, 1);
	*err = fd_slurp(&lenErr, epipe[0], FILE_CHUNK, 1);

	spawn_wait(child, exitcode);
	
	close(opipe[0]);
	close(epipe[0]);
	
	return 0;
}

err_t spawn_wait(pid_t pid, int* exitcode){
	if( waitpid(pid, exitcode, 0) == -1 ){
		err_pushno("wait pid %d", pid);
		return -1;
	}
	if( exitcode ) *exitcode = WIFEXITED(*exitcode) ? WEXITSTATUS(*exitcode) : -1;
	return 0;
}

err_t spawn_check(pid_t pid, int* exitcode){
	if( waitpid(pid, exitcode, WNOHANG) < 0 ){
		err_pushno("wait pid %d", pid);
		return -1;
	}
	int exited = WIFEXITED(*exitcode);
	if( exitcode ) *exitcode = exited ? WEXITSTATUS(*exitcode) : -1;
	return exited;
}
