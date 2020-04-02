#ifndef __EF_SIGNAL_H__
#define __EF_SIGNAL_H__

#include <ef/type.h>
#include <signal.h>

typedef struct sigaction sigaction_s;
typedef siginfo_t siginfo_s;

typedef void(*signal_f)(int, siginfo_s*, void*);

err_t os_signal_get_status(sigset_t* mask);
err_t os_signal_set(sigaction_s* old, int num, signal_f fnc);
err_t os_signal_restore(int num, sigaction_s* sa);
void os_signal_wait(void);


#endif
