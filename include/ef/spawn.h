#ifndef __EF_SPAWN_H__
#define __EF_SPAWN_H__

#include <ef/type.h>

#ifndef SPAWN_SHELL
	/** if no $SHELL is find use bash*/
	#define SPAWN_SHELL_PATH "/bin/bash"
	/** arg for exec*/
	#define SPAWN_ARGUMENT "-c"
#endif

/** return shell path*/
const char* os_shell_get(void);

/** disable zombie, warning, this disable get exit status*/
void spawn_disable_zombie(void);

/** call whend end of use spawn, wait(NULL)*/
void spawn_end(void);

/** replace current process with shell software*/
void shell(const char* cmdline);

/** execute command in shell
 * @param cmdline command to execute
 * @param disableoutput disable stdout stderr
 * @return pid of command or -1
 */
pid_t spawn_shell(const char* cmdline, int disableoutput);

/** execute command in shell and slurp out/err
 * @param out the stdout string, remember to free
 * @param err the stderr string, remember to free
 * @param exitcode exit code if not null
 * @param cmdline cmdline command
 * @return 0 successfull -1 error
 */
err_t spawn_shell_slurp(char** out, char** err, int* exitcode, const char* cmdline);

/** wait a pid*/
err_t spawn_wait(pid_t pid, int* exitcode);

/** wait a pid is not child*/
err_t spawn_wait_any(pid_t pid, int* exitcode);

/** same wait pid but without wait*/
err_t spawn_check(pid_t pid, int* exitcode);

/** return fd where write on pid exited*/
int spawn_waitfd(pid_t pid);

/** read and clean waitfd, close fd and waitpid on master process*/
err_t spawn_waitfd_read(int fd, pid_t* slave, int* res);


#endif
