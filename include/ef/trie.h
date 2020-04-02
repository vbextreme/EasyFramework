#ifndef __EF_TRIE_H__
#define __EF_TRIE_H__

#include <ef/type.h>

typedef void(*trieFree_f)(void*);

typedef struct trieElement{
	char* charset;            /**< array of char*/
	struct trieElement* next; /**< array of next element*/ 
	void** endnode;           /**< array of end node*/
}trieElement_s;

typedef struct trie{
	trieElement_s root /**< root */;
	trieFree_f free    /**< callback free function*/;
}trie_s;

/** trie step state*/
typedef enum {
	TRIE_STEP_ERROR = -1, /**< error on step*/
	TRIE_STEP_NEXT,       /**< can step next*/
	TRIE_STEP_END_NODE    /**< can step next or end node*/
} trieStep_e;

/** create a trie
 * @param freefnc a free function called for each element
 * @return trie or NULL for error
 */
trie_s* trie_new(trieFree_f freefnc);

/** free trie, call free for each element
 * @param trie trie
 */
void trie_free(trie_s* trie);

void trie_free_auto(trie_s** trie);

/** cleanup*/
#define __trie_free __cleanup(trie_free_auto)

/** add node from current node 
 * @param node current node
 * @param ch new char
 * @param data if not null is an end node
 * @return next node
 */
trieElement_s* trie_add(trieElement_s* node, char ch, void* data);

/** insert string in trie, add ch 1 for last element
 * @param trie
 * @param str string to insert
 * @param data userdata for end node
 * @return 0 successfull; -1 error
 */
err_t trie_insert(trie_s* trie, const char* str, void* data);

/** step in trie, step with ch = 1 for last element
 * @param out out userdata if is an end node
 * @param el current element
 * @param ch char to check
 * @return trieStep
 * @see trieStep_e
 */
trieStep_e trie_step(void* out, trieElement_s** el, char ch);

/** search in trie
 * @param trie
 * @param str string to search
 * @return userdata if find endnode otherwise NULL
 */
void* trie_search(trie_s* trie, const char* str);


#endif
