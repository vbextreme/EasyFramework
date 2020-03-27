#include <ef/pty.h>
#include <ef/err.h>
#include <ef/str.h>

#include <fcntl.h>

/* fork from linux programming interface fork */
int term_ptm_open(char** slaveName){
    int masterFd;
    char *p;

    masterFd = posix_openpt(O_RDWR | O_NOCTTY);
    if( masterFd == -1 ){
		err_pushno("posix_openpt");
        return -1;
	}

    if( grantpt(masterFd) == -1 ){
		err_pushno("grantpt");
        close(masterFd);
        return -1;
    }

    if( unlockpt(masterFd) == -1 ){
		err_pushno("unlock");
        close(masterFd);
        return -1;
    }

    if( (p = ptsname(masterFd)) == NULL ){
		err_pushno("ptsname");
        close(masterFd);
        return -1;
    }

	*slaveName = str_dup(p, 0);
	if( !*slaveName ){
		close(masterFd);
		return -1;
	}

    return masterFd;
}

/* linux programming interface fork */
pid_t pty_fork(int *masterFd, char** slaveName, struct termios *slaveTermios, struct winsize *slaveWS){
	int mfd;
	int slaveFd;
	
	*masterFd = -1;

    pid_t childPid;

    mfd = term_ptm_open(slaveName);
    if( mfd == -1 )	return -1;

    childPid = fork();

    if( childPid == -1 ){
		err_pushno("fork");
        close(mfd);
		free(slaveName);
        return -1;
    }

    if( childPid != 0 ){
		/* parent */
		dbg_info("slave name:%s", *slaveName);
        *masterFd = mfd;
        return childPid;
    }

    /* child */

	if( setsid() == -1 ){
		err_pushno("setsid");
		err_fail("slave pty");
	}

    close(mfd);
    slaveFd = open(*slaveName, O_RDWR);
    if( slaveFd == -1 ){
		err_pushno("open");
		err_fail("slave pty");
    }

#ifdef TIOCSCTTY
    if( ioctl(slaveFd, TIOCSCTTY, 0) == -1 ){
		err_pushno("slave ioctl tiocsctty");
		err_fail("slave pty");
	}
#endif

    if( slaveTermios != NULL ){
        if( tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1 ){
			err_pushno("slave tcsetattr");
            err_fail("slave pty");
		}
	}

    if(slaveWS != NULL){
        if( ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1 ){
			err_pushno("slave ioctl tiocswinsz");
			err_fail("slave pty");
		}
	}

    /* Duplicate pty slave to be child's stdin, stdout, and stderr */

    if( dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO ){
		err_pushno("slave dup2 stdin");
		err_fail("slave pty");
	}
    if( dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO ){
		err_pushno("slave dup2 stdout");
		err_fail("slave pty");
	}
    if( dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO ){
		err_pushno("slave dup2 stderr");
		err_fail("slave pty");
	}

    if( slaveFd > STDERR_FILENO ){
		dbg_info("close slave fd");
        close(slaveFd);
	}

    return 0;
}

/*
pid_t term_screen_new(){
	int mfd;
	char slaveName[1024];

	pid_t p = term_pty_fork(&mfd, slaveName, 1024, NULL, NULL);

	if( p == 0 ){
		puts("slave write");
		fflush(stdout);
		delay_ms(5000);
		puts("slave exit");
		fflush(stdout);
		exit(0);
	}

	puts("master write in term");
	//write(mfd, "master write in mfd", 19);
	ssize_t nr;
	if( (nr=read(mfd,slaveName,1024))>0 ){
		slaveName[nr] = 0;
		printf("master rec::%s\n",slaveName);
	}
	//fgets(slaveName,1024,stdin);

	fsync(mfd);
	wait(NULL);
}
*/











