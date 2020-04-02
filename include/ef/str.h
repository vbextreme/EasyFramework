#ifndef __EF_STRING_H__
#define __EF_STRING_H__

#include <ef/type.h>

/** strcmp auto strlen
 * @param A string a
 * @param B string b where used strlen
 * @return same value of strncmp
 */
#define str_ancmp(A,B) strncmp(A,B,strlen(B))

/****************/
/*** string.c ***/
/****************/

/** duplicate string in new memory
 * @param src string to copy
 * @param optlen if 0 automatic strlen otherwise copy only optlen size and add 0
 * @return new memory, need to free
 */
char* str_dup(const char* src, size_t optlen);

/** duplicate string in new memory to ch
 * @param src string to copy
 * @param ch copy to ch, ch is not copy
 * @return new memory, need to free
 */
char* str_dup_ch(const char* src, const char ch);

/** compare string is equal, before compare len and after use memcmp
 * @param a string to compare
 * @param lena len of a, if 0 automatic strlen
 * @param b string to compare
 * @param lenb len of b, if 0 automatic strlen
 * @return same value of memcmp
 */
int str_equal(char const* a, size_t lena, char const* b, size_t lenb);

/** skip space and tab from string
 * @param str string
 * @return first char is not h
 */
const char* str_skip_h(const char* str);

/** skip space, tab and newline from string
 * @param str string
 * @return first char is not hn
 */
const char* str_skip_hn(const char* str);

/** go to next line
 * @param str string
 * @return first char in next line
 */
const char* str_next_line(const char* str);

/** copy n char to dest, safe version with 0
 * @return end of dst
 */
char* str_ncpy(char* restrict dst, size_t lend, const char* restrict src, size_t lens);

/** copy n char to dest, safe version with 0
 * @return end of dst
 */
char* str_cpy(char* dst, size_t lend, const char* src);

/** find ch in string
 * @param str string
 * @param ch char to search
 * @return pointer to ch in str or pointer to \0 in str
 */
const char* str_chr(const char* str, const char ch);

/** append string in dst, realloc if need
 * @param dst destination
 * @param len len of dest
 * @param size size of dest
 * @param src source
 * @param lenSrc len of src, 0 automatic strlen
 * @return dst or new address if need
 */
char* string_append(char* dst, size_t* len, size_t* size, const char* src, size_t lenSrc);

/** append string in head of dst, realloc if need
 * @param dst destination
 * @param len len of dest
 * @param size size of dest
 * @param src source
 * @param lenSrc len of src, 0 automatic strlen
 * @return dst or new address if need
 */
char* string_head(char* dst, size_t* len, size_t* size, const char* src, size_t lenSrc);

/** create string from formst
 * @param format same printf
 * @param va1 vaarg
 * @param va2 copy of vaarg
 * @return allocated string, need to free
 */
char* str_vprintf(const char* format, va_list va1, va_list va2);

/** create string from formst, remember to free*/
__printf(1,2) char* str_printf(const char* format, ...);

/** swap two string content, the array of char need to have same size
 * @param a
 * @param b
 */
void str_swap(char* restrict a, char* restrict b);

/** remove enter to end line
 * @return new string len or -1 if no enter on string
 */
ssize_t str_chomp(char* str);

/** decode a quote printable string
 * @param len if len return len of string
 * @param str string to decode
 * @return allocated string, need to free
 */
char* quote_printable_decode(size_t *len, const char* str);

/** convert to upper*/
void str_toupper(char* dst, const char* src);

/** convert to lower*/
void str_tolower(char* dst, const char* src);

#endif
