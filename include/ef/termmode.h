#ifndef __EF_TERM_MODE_H__
#define __EF_TERM_MODE_H__

#include <ef/type.h>
#include <termios.h>
#include <sys/ioctl.h>

typedef struct termios termios_s;
typedef struct winsize winsize_s;

/** get termios setting */
#define term_settings_get(PTR_SETTINGS) tcgetattr(0, PTR_SETTINGS)

/** set termios setting */
#define term_settings_set(PTR_SETTINGS) tcsetattr(0, TCSANOW, PTR_SETTINGS)

/** set raw mode and get termios */
void term_raw_mode(termios_s* old);

/** enter in raw mode */
void term_raw_enable(void);

/** exit from raw mode */
void term_raw_disable(void);



#endif 
