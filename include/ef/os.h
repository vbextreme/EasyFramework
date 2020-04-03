#ifndef __EF_OS_H__
#define __EF_OS_H__

#include <ef/type.h>

/** set locale and return previous setting*/
const char* os_setlocale(int category, const char* locale);

/** get current locale */
#define os_getlocale() locale_charset()

void os_begin(void);

#endif

