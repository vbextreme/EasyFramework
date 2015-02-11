#ifndef EASYFILE_INCLUDED
#define EASYFILE_INCLUDED

#include <easytype.h>
#include <sys/types.h>

#define PRV_ALL         7000
#define PRV_SUID        4000
#define PRV_NOTUSE      4000
#define PRV_GUID        2000
#define PRV_USEGRP      2000
#define PRV_STICKY      1000
#define PRV_LIMITWRITE  1000
#define PRV_USR_ALL     0700
#define PRV_USR_READ    0400
#define PRV_USR_WRITE   0200
#define PRV_USR_EXECUTE 0100
#define PRV_GRP_ALL     0070
#define PRV_GRP_READ    0040
#define PRV_GRP_WRITE   0020
#define PRV_GRP_EXECUTE 0010
#define PRV_ALL_ALL     0007
#define PRV_ALL_READ    0004
#define PRV_ALL_WRITE   0002
#define PRV_ALL_EXECUTE 0001

#define INO_CPY_BLK 1048576
#define DIR_MAX_NAME 512

typedef enum {FT_UNKNOWN,FT_REG,FT_LINK,FT_DIR,FT_FIFO,FT_SOCKET,FT_DCHR,FT_DBLK,FT_COUNT}FILETYPE;
typedef UINT16 PRIVILEGE;
typedef uid_t UID;
typedef gid_t GID;


VOID prv_maskreset();
PRIVILEGE privilege(BOOL ur, BOOL uw, BOOL ux, BOOL gr, BOOL gw, BOOL gx, BOOL ar, BOOL aw, BOOL ax);

CHAR* pht_add(CHAR* d,char* n);
CHAR* pht_back(CHAR* d);
CHAR* pht_current(CHAR* d);
CHAR* pht_normalize(CHAR* d, CHAR* s);
BOOL pht_set(CHAR* d);
CHAR* pht_homedir();

BOOL ino_mklink(CHAR* d, CHAR* s);
BOOL ino_delete(CHAR* s);
BOOL ino_rename(CHAR* d,CHAR* s);
BOOL ino_exist(CHAR* s);
BOOL ino_info(CHAR* s, PRIVILEGE* p, UID* uid, GID* gid, FILETYPE* ft, UINT32* sz, DATE* acc, DATE* mod, DATE* cha);
BOOL ino_properties(CHAR* s, UID uid, GID gid);
BOOL ino_timeset(CHAR* s, DATE* acc, DATE* mod);
BOOL ino_cpy(CHAR* d, CHAR* s);

BOOL dir_new(CHAR* s,PRIVILEGE p);
FILETYPE dir_list(CHAR* d, BOOL filter, FILETYPE ftype, CHAR* path);

VOID cfg_reset(FILE* f);
BOOL cfg_read(CHAR* d,CHAR* v, FILE* f);
VOID cfg_write(CHAR* n,CHAR* v, FILE* f);

#endif // EASYFILE_INCLUDED
