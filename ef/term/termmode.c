#include <ef/termmode.h>

__private termios_s previusSession;

void term_raw_mode(termios_s* old){
	if( old == NULL ) old = &previusSession;
	tcgetattr(0, old);
    termios_s ns = *old;
	ns.c_lflag &= ~ICANON;
	ns.c_lflag &= ~ECHO;
	ns.c_lflag &= ~ISIG;
	ns.c_lflag &= ~IXON;
	ns.c_cc[VMIN] = 1;
	ns.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &ns);
}

void term_settings_changes(termios_s* old, termios_s* nw){
	term_settings_get(old);
    term_settings_set(nw);
}

void term_raw_enable(void){
	term_raw_mode(&previusSession);
}

void term_raw_disable(void){
	term_settings_set(&previusSession);
}
