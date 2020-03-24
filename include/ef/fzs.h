#ifndef __EF_FZS_H__
#define __EF_FZS_H__

#include <ef/type.h>
#include <ef/vector.h>

/**structure to use in vector for fzs qsort*/
typedef struct fzsElement{
	const char* str; /**< pointer to string*/
	size_t len;      /**< len of string*/
	size_t distance; /**< laste distance calcolate*/
}fzsElement_s;

/** calcolate levenshtein 
 * @param a 
 * @param lena if 0 calcolate strlen a
 * @param b
 * @param lenb if 0 calcolate strlen b
 * @return a size_t, depicting the difference between `a` and `b`, See <https://en.wikipedia.org/wiki/Levenshtein_distance> for more information.
 */
size_t fzs_levenshtein(const char *a, const size_t lena, const char *b, const size_t lenb);

/** find a element with minimal distance
 * @param v char** vector
 * @param str string to search
 * @param lens len of string, if 0 auto strlen
 */
ssize_t fzs_vector_find(char** v, const char* str, size_t lens);

/** reorder vector of fzse to distance of str
 * @param fzse vector of fzsElement_s
 * @param str string to reordering distance
 * @param lens len of string, if 0 auto strlen
 */
void fzs_qsort(fzsElement_s* fzse, const char* str, size_t lens);

#endif
