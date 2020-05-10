#ifndef _AUTOGEN_H_
#define _AUTOGEN_H_
#define AUTOGEN_OPT \
{ 0, 'Z', "code", ARGDEF_NOARG, NULL, "private code generation"},\
{ 0, 'e', "err", ARGDEF_NOARG, NULL, "test error"},\
{ 0, 'f', "file", ARGDEF_NOARG, NULL, "test file use argva for get config file, default ../config.test"},\
{ 0, 't', "ftp", ARGDEF_NOARG, NULL, "test ftp"},\
{ 0, 'F', "fuzzy-search", ARGDEF_NOARG, NULL, "test fuzzy search, use arg-a for data, use arg-b for search string, default is vbextreme"},\
{ 0, 'g', "gui", ARGDEF_NOARG, NULL, "test gui"},\
{ 0, 'H', "hash", ARGDEF_NOARG, NULL, "test function hash, use arg-a for file data, without a use internal data"},\
{ 0, 'i', "imap", ARGDEF_NOARG, NULL, "test imap, -a imaps://server.com;myemail@email.com -b <ls><show:from:to:dir><delete:id:dir><info:dir>"},\
{ 0, 'j', "json", ARGDEF_NOARG, NULL, "test json, if argA is setted print json error, if argB is setted exit after first error"},\
{ 0, 'm', "mem", ARGDEF_NOARG, NULL, "test memory"},\
{ 0, 'M', "math", ARGDEF_NOARG, NULL, "test math"},\
{ 0, 'O', "oauth", ARGDEF_NOARG, NULL, "test oauth"},\
{ 0, 'o', "os", ARGDEF_NOARG, NULL, "test os"},\
{ 0, 'p', "promise", ARGDEF_NOARG, NULL, "test promise"},\
{ 0, 'R', "rb-hash", ARGDEF_NOARG, NULL, "test rbhash, use arg-a for file data, without a use internal data, use arg-b for hash function, default vbx, all for test all"},\
{ 0, 'S', "socket", ARGDEF_NOARG, NULL, "test socket arg-a set client/client.tls/server/server.tls, arg-b type:addr:port, where type is unix or net4. write exit to end"},\
{ 0, 's', "str", ARGDEF_NOARG, NULL, "test string"},\
{ 0, 'E', "term", ARGDEF_NOARG, NULL, "test terminal"},\
{ 0, 'r', "threads", ARGDEF_NOARG, NULL, "test threads"},\
{ 0, 'T', "trie", ARGDEF_NOARG, NULL, "test trie, use arg-a for file data, without use internal data set"},\
{ 0, 'v', "vector", ARGDEF_NOARG, NULL, "test vector"},
#define AUTOGEN_PROTO \
void test_code(__unused const char* argA, __unused const char* argB);\
void test_err(__unused const char* argA, __unused const char* argB);\
void test_file(const char* argA, __unused const char* argB);\
void test_ftp(__unused const char* argA, __unused const char* argB);\
void test_fuzzysearch(const char* a, const char* b);\
void test_gui(__unused const char* argA, __unused const char* argB);\
void test_hash(const char* a, __unused const char*b);\
void test_imap(const char* argA, __unused const char* argB);\
void test_json(const char* argA, const char* argB);\
void test_mem(__unused const char* argA, __unused const char* argB);\
void test_math(__unused const char* argA, __unused const char* argB);\
void test_oauth(__unused const char* argA, __unused const char* argB);\
void test_os(__unused const char* argA, __unused const char* argB);\
void test_promise(const char* argA, __unused const char* argB);\
void test_hashtable(const char* a, const char* b);\
void test_socket(const char* argA, const char* argB);\
void test_str(__unused const char* argA, __unused const char* argB);\
void test_term(__unused const char* argA, __unused const char* argB);\
void test_thr(__unused const char* argA, __unused const char* argB);\
void test_trie(const char* a, __unused const char* b);\
void test_vector(__unused const char* argA, __unused const char* argB);
#define AUTOGEN_FN \
test_code,\
test_err,\
test_file,\
test_ftp,\
test_fuzzysearch,\
test_gui,\
test_hash,\
test_imap,\
test_json,\
test_mem,\
test_math,\
test_oauth,\
test_os,\
test_promise,\
test_hashtable,\
test_socket,\
test_str,\
test_term,\
test_thr,\
test_trie,\
test_vector,
#endif
