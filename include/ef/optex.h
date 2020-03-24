#ifndef __EF_OPTEX_H__
#define __EF_OPTEX_H__

#include "ef/type.h"

typedef enum{ 
	ARGDEF_NOARG, /**< no argument */
	ARGDEF_SIGNED, /**< long argument*/ 
	ARGDEF_UNSIGNED, /**< unsigned long argument*/
	ARGDEF_DOUBLE, /**<double argument*/
	ARGDEF_STR
} argdef_e;

typedef struct argdef{
	int hasset; /**< argument is setting*/
	int vshort; /**< short format value*/
	char* vlong; /**< long format value*/
	argdef_e typeParam; /**< @see argdef_e*/
	void* autoset; /**< automatic assign value*/
	char* descript; /**< descript of argument*/
}argdef_s;

/** parse command line arguments
 * @param args the structure argdef
 * @param argv the main argv
 * @param argc the main argc
 * @return last argument parsed or -1 for error
 */
int opt_parse(argdef_s* args, char** argv, int argc);

void opt_errno(int* ac, int* sub, int* info);
/** print error on terminal
 * @param argc main argc
 * @param argv main argv
 */
void opt_error(int argc, char** argv);

/** display opt usage
 * @param args the structure argdef
 * @param argv0 the software name
 */
void opt_usage(argdef_s* args, char* argv0);

/** check if option is enabled
 * @param ARGS structure argdef
 * @param ID id of argument
 * @return 1 if set otherwise 0
 */
#define opt_enabled(ARGS,ID) (ARGS[ID].hasset)

/** get str value automatic setted
 * @param ARGS structure argdef
 * @param ID id of argument
 * @return const char*
 */
#define opt_arg_str(ARGS,ID) (const char*)(ARGS[ID].autoset)

#endif

