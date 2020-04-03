#include <ef/os.h>
#include <locale.h>

const char* os_setlocale(int category, const char* locale){
	const char* previous = setlocale(category, NULL);
	setlocale(category, locale);
	return previous;
}

void os_begin(void){
	__cpu_init();
}
