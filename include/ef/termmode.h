#ifndef __EF_TERM_MODE_H__
#define __EF_TERM_MODE_H__

#include <ef/type.h>
#include <termios.h>
#include <sys/ioctl.h>

typedef struct termios termios_s;
typedef struct winsize winsize_s;

/** get termios setting */
#define term_settings_fdget(FD, PTR_SETTINGS) tcgetattr(FD, PTR_SETTINGS)

/** set termios setting */
#define term_settings_fdset(FD, PTR_SETTINGS) tcsetattr(FD, TCSANOW, PTR_SETTINGS)

/** get termios setting */
#define term_settings_get(PTR_SETTINGS) term_settings_fdget(0, PTR_SETTINGS)

/** set termios setting */
#define term_settings_set(PTR_SETTINGS) term_settings_fdset(0, PTR_SETTINGS)

/** set raw mode and get termios */
void term_raw_mode(termios_s* old);

/** enter in raw mode */
void term_raw_enable(void);

/** exit from raw mode */
void term_raw_disable(void);

/** read winsize struct*/
err_t term_winsize_get(winsize_s* ws);

#endif 
