#ifndef __EF_TERM_PTY_H__
#define __EF_TERM_PTY_H__

#include <ef/type.h>
#include <termios.h>
#include <sys/ioctl.h>

#ifndef TERM_MAX_SLAVE_NAME
	#define TERM_MAX_SLAVE_NAME 32
#endif

/** open pt master return
 * @param slaveName pointer to str where stored slave name, remember to free
 * @return master fd
 */
int term_ptm_open(char** slaveName);

/** create master and slave pty
 * @param masterFd pty master fd returned only in master process
 * @param slaveName return slave name in master and slave, need to free
 * @param slaveTermios if not null store termios setting only in slave process
 * @param slaveWS if not null store winsize setting only in slave process
 * @return slave pid for master, 0 for slave, -1 for error
 * @code
 * void screen_example(){
 *	int mfd;
 *	char* slaveName;
 *	pid_t p = term_pty_fork(&mfd, &slaveName, NULL, NULL);
 *
 *	if( p == 0 ){
 *		puts("slave write");
 *		fflush(stdout);
 *		delay_ms(5000);
 *		puts("slave exit");
 *		fflush(stdout);
 *		exit(0);
 *	}
 *
 *	puts("master write in term");
 *	//write(mfd, "master write in mfd", 19);
 *	ssize_t nr;
 *	if( (nr=read(mfd,slaveName,1024))>0 ){
 *		slaveName[nr] = 0;
 *		printf("master rec::%s\n",slaveName);
 *	}
 *	//fgets(slaveName,1024,stdin);
 *
 *	fsync(mfd);
 *	wait(NULL);
 *	}
 *	@endcode
 */
pid_t pty_fork(int *masterFd, char** slaveName, struct termios *slaveTermios, struct winsize *slaveWS);



#endif 
