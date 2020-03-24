#ifndef __EF_CONFIG_H__
#define __EF_CONFIG_H__

#include <ef/type.h>
#include <ef/file.h>
#include <ef/trie.h>

#define CONFF_ERROR      0x01

typedef int (*config_f)(int type, char** value, void* userdata);

typedef struct configTrie{
	config_f fn;    /** callback for each config, return CONFF_ERROR for terminate reading config file, return CONFF_GRAB_VALUE for not free a value*/
	int type;       /** user type definition */
	void* userdata; /** userdata */
}configTrie_s;

/** read a config file, accept
 * KEY
 * KEY = VALUE
 * KEY = 'VALUE'
 * KEY = "VALUE"
 * @param tr a trie with endnode contains a configTrie_s
 * @param sm a stream
 * @return 1 if read one config, 0 no more config to read, -1 for error
 */
int config_parse_line(trie_s* tr, stream_s* sm);

/** read all configuration file
 * @param tr a trie with endnode contains a configTrie_s
 * @param sm a stream
 * @return 0 successfull, -1 for error
*/
err_t config_parse(trie_s* tr, stream_s* sm);

#endif
