#include <ef/termmode.h>

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
