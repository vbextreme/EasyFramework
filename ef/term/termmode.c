#include <ef/termmode.h>
#include <ef/file.h>
#include <ef/memory.h>
#include <ef/err.h>

#include <fcntl.h>

__private int pSInit;
__private termios_s previusSession;

__private char* hyperbuf;

err_t term_buff_same_screen(void){
	winsize_s ws;
	if( term_winsize_get(&ws) ){
		return -1;
	}
	size_t size = (ws.ws_col * ws.ws_row + 1)*4;
	hyperbuf = mem_many(char, size);
	if( !hyperbuf ) return -1;
	if( setvbuf(stdout, hyperbuf, _IOFBF, size) ){
		err_pushno("setvbuf"); 
	}
	dbg_info("new buffer size %lu", size);
	
	return 0;
}

void term_buff_end(void){
	if( hyperbuf ) free(hyperbuf);
}

void term_raw_mode(termios_s* old){
	term_settings_get(old);
    termios_s ns = *old;
	ns.c_lflag &= ~ICANON;
	ns.c_lflag &= ~ECHO;
	ns.c_lflag &= ~IXON;
	ns.c_cc[VMIN] = 1;
	ns.c_cc[VTIME] = 0;
	term_settings_set(&ns);
}

void term_raw_enable(void){
	pSInit = 1;
	term_raw_mode(&previusSession);
}

void term_raw_disable(void){
	if( pSInit ) term_settings_set(&previusSession);
	pSInit = 0;
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

