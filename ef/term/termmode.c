#include <ef/termmode.h>
#include <ef/file.h>
#include <ef/err.h>
#include <fcntl.h>

__private termios_s previusSession;

void term_raw_mode(termios_s* old){
	term_settings_get(old);
    termios_s ns = *old;
	ns.c_lflag &= ~ICANON;
	ns.c_lflag &= ~ECHO;
	ns.c_lflag &= ~ISIG;
	ns.c_lflag &= ~IXON;
	ns.c_cc[VMIN] = 1;
	ns.c_cc[VTIME] = 0;
	term_settings_set(&ns);
}

void term_raw_enable(void){
	term_raw_mode(&previusSession);
}

void term_raw_disable(void){
	term_settings_set(&previusSession);
}

err_t term_winsize_get(winsize_s* ws){
	const char* pts;
	__fd_close int fd = -1;

    if( !(pts = ttyname(STDIN_FILENO)) || !(pts = ttyname(STDOUT_FILENO)) ){
		err_push("can't get tty name");
		return -1;
	}

	dbg_info("use tty %s", pts);
    while( (fd = open(pts, O_RDWR | O_NOCTTY, 0)) == -1 && errno == EINTR);
    if( fd == -1){
		err_pushno("open tty %s", pts);
		return -1;
	}

	if( ioctl(fd, TIOCGWINSZ, ws) == -1 ){
		err_pushno("ioctl tiocswinsz");
        return -1;
	}

	return 0;
}

