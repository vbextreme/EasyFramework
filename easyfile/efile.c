#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <utime.h>
#include <pwd.h>

#include "easyfile.h"

VOID _tttd(struct timespec* s,DATE* d)
{
    struct tm* timeinfo;

    timeinfo = localtime (&s->tv_sec);
    d->y = timeinfo->tm_year + 1900;
    d->m = timeinfo->tm_mon + 1;
    d->d = timeinfo->tm_mday;
    d->hh = timeinfo->tm_hour;
    d->mm = timeinfo->tm_min;
    d->ss = timeinfo->tm_sec;
}

VOID _dttt(DATE* s,time_t* d)
{
    time_t raw;
    struct tm* ti;
    time (&raw);
    ti = localtime(&raw);
    ti->tm_year = s->y - 1900;
    ti->tm_mon = s->m - 1;
    ti->tm_mday = s->d;
    ti->tm_hour = s->hh;
    ti->tm_min = s->mm;
    ti->tm_sec = s->ss;
    *d = mktime(ti);
}

UINT32 _fttdt(FILETYPE ft)
{	
	switch ( ft ) 
	{
		case FT_DBLK: return DT_BLK;
		case FT_DCHR: return DT_CHR;
		case FT_DIR:  return DT_DIR;
		case FT_FIFO: return DT_FIFO;
		case FT_LINK: return DT_LNK;
		case FT_REG:  return DT_REG;
		case FT_SOCKET: return DT_SOCK;
		default: return DT_UNKNOWN;
	}
	return DT_UNKNOWN;
}

FILETYPE _dttft(UINT32 dt)
{	
	switch ( dt ) 
	{
		case DT_BLK: return FT_DBLK;
		case DT_CHR: return FT_DCHR;
		case DT_DIR:  return FT_DIR;
		case DT_FIFO: return FT_FIFO;
		case DT_LNK: return FT_LINK;
		case DT_REG:  return FT_REG;
		case DT_SOCK: return FT_SOCKET;
		default: return FT_UNKNOWN;
	}
	return FT_UNKNOWN;
}

VOID prv_maskreset()
{
	umask(0);
}


PRIVILEGE privilege(BOOL ur, BOOL uw, BOOL ux, BOOL gr, BOOL gw, BOOL gx, BOOL ar, BOOL aw, BOOL ax)
{
	PRIVILEGE p = 0;
	if ( ur ) p |= PRV_USR_READ;
	if ( uw ) p |= PRV_USR_WRITE;
	if ( ux ) p |= PRV_USR_EXECUTE;
	if ( gr ) p |= PRV_GRP_READ;
	if ( gw ) p |= PRV_GRP_WRITE;
	if ( gx ) p |= PRV_GRP_EXECUTE;
	if ( ar ) p |= PRV_ALL_READ;
	if ( aw ) p |= PRV_ALL_WRITE;
	if ( ax ) p |= PRV_ALL_EXECUTE;
	return p;
}

CHAR* pht_add(CHAR* d, CHAR* n)
{
    int ld = strlen(d);
    if ( d[ld-1] != '/')
    {
        d[ld++] = '/';
        d[ld] = '\0';
    }
    if (*n == '/') ++n;
    strcat(d,n);
    return d;
}

CHAR* pht_back(CHAR* d)
{
    CHAR* f = d + ( strlen(d) -1 );
    if ( *f == '/') --f;

    for (; f > d && *f != '/'; --f);
	
	if ( f > d ) *f = '\0';
   
    return d;
}

CHAR* pht_current(CHAR* d)
{
    return getcwd(d,DIR_MAX_NAME);
}

CHAR* pht_normalize(CHAR* d, CHAR* s)
{
	char tmp[DIR_MAX_NAME];
	
	if ( !strncmp(s,"..",2) )
	{
		pht_current(tmp);
		pht_back(tmp);
		
		if ( s[2] == '/' && s[3] )
		{
			snprintf(d,DIR_MAX_NAME,"%s/%s",tmp,&s[3]);
		}
		else
		{
			strcpy(d,tmp);
		}
		return d;
	}
	
	if ( *s == '.' )
	{
		if ( s[1] == '/' && s[2] ) 
		{
			pht_current(tmp);
			snprintf(d,DIR_MAX_NAME,"%s/%s",tmp,&s[2]);
		}
		else
		{
			pht_current(d);
		}
		return d;
	}
	
	strcpy(d,s);
    return d;
}

BOOL pht_set(CHAR* d)
{
    return (!chdir(d)) ? TRUE : FALSE;
}


CHAR* pht_homedir()
{
	CHAR *hd;
	if ((hd = getenv("HOME")) == NULL) 
		hd = getpwuid(getuid())->pw_dir;
	return hd;
}

BOOL ino_mklink(CHAR* d, CHAR* s)
{
    return (!symlink(s,d)) ? TRUE : FALSE;
}

BOOL ino_delete(CHAR* s)
{
    return (!unlink(s)) ? TRUE : FALSE;
}

BOOL ino_rename(CHAR* d,CHAR* s)
{
    return (!rename(s,d)) ? TRUE : FALSE;
}

BOOL ino_exist(CHAR* s)
{
	struct stat b;
	if (stat(s,&b) == -1) return FALSE;
	return TRUE;
}

BOOL ino_info(CHAR* s, PRIVILEGE* p, UID* uid, GID* gid, FILETYPE* ft, UINT32* sz, DATE* acc, DATE* mod, DATE* cha)
{
    struct stat b;
    if (stat(s,&b) == -1)
        return FALSE;

    if (p) *p = (PRIVILEGE)(b.st_mode);
    if (ft)
    {
		switch ( b.st_mode & S_IFMT ) 
		{
			case S_IFBLK: *ft = FT_DBLK; break;
			case S_IFCHR: *ft = FT_DCHR; break;
			case S_IFDIR: *ft = FT_DIR;  break;
			case S_IFIFO: *ft = FT_FIFO; break;
			case S_IFLNK: *ft = FT_LINK; break;
			case S_IFREG: *ft = FT_REG;  break;
			case S_IFSOCK: *ft = FT_SOCKET; break;
			default: *ft = FT_UNKNOWN; break;
		}
    }
    if (acc) _tttd(&b.st_atim,acc);
    if (mod) _tttd(&b.st_mtim,mod);
    if (cha) _tttd(&b.st_ctim,cha);
    if (sz) *sz = b.st_size;
	if (uid) *uid = b.st_uid;
	if (gid) *gid = b.st_gid;
	
    return TRUE;
}

BOOL ino_properties(CHAR* s, UID uid, GID gid)
{
    return (!chown(s,uid,gid)) ? TRUE : FALSE;
}

BOOL ino_privilege(CHAR* s, PRIVILEGE p)
{
    return (!chmod(s,p)) ? TRUE : FALSE;
}

BOOL ino_timeset(CHAR* s, DATE* acc, DATE* mod)
{
    struct utimbuf ut;

    if (!acc & !mod)
        return (!utime(s,NULL)) ? TRUE : FALSE;

    if (acc)
        _dttt(acc,&ut.actime);
    else
        ut.actime = 0;

    if (acc)
        _dttt(mod,&ut.modtime);
    else
        ut.modtime = 0;

    return (!utime(s,&ut)) ? TRUE : FALSE;
}

BOOL ino_cpy(CHAR* d, CHAR* s)
{
    UINT32 sz,rsz;
    if (!ino_info(s,NULL,NULL,NULL,NULL,&sz,NULL,NULL,NULL)) return FALSE;
    

    FILE* fs = fopen(s,"r");
        if (!fs) return FALSE;
    FILE* fd = fopen(d,"w");
        if (!fd){fclose(fs);return FALSE;}

    BYTE buffer[INO_CPY_BLK];

    while (sz)
    {
        if (sz <= INO_CPY_BLK)
        {
            sz = fread(buffer,1,sz,fs);
            fwrite(buffer,1,sz,fd);
            sz = 0;
        }
        else
        {
            rsz = fread(buffer,1,INO_CPY_BLK,fs);
            fwrite(buffer,1,rsz,fd);
            sz -= rsz;
        }
    }

    fclose(fs);
    fclose(fd);
    return TRUE;
}


BOOL dir_new(CHAR* s,PRIVILEGE p)
{
    return (!mkdir(s,p)) ? TRUE : FALSE;
}

FILETYPE dir_list(CHAR* d, BOOL filter, FILETYPE ftype, CHAR* path)
{
	static DIR* dir = NULL;
	UINT32 dt = _fttdt(ftype);
	
	if ( path )
	{
		if ( dir ) {closedir(dir); dir = NULL;}
		struct stat info;
			if (stat(path,&info) == -1) return -1;
		if ( !S_ISDIR(info.st_mode) ) return -1;
		
		dir = opendir(path);
		if ( !dir ) return -1;
	}
	
	struct dirent* dr;
	while ( (dr = readdir(dir)) )
	{
		if ( !strcmp(dr->d_name,".") ) continue;
		if ( !strcmp(dr->d_name,"..") ) continue;
		if ( filter && dr->d_type != dt ) continue;
		break;
	}
	
	if ( !dr ) { closedir(dir); dir = NULL; return -1;}
	strcpy(d,dr->d_name);
	return _dttft(dr->d_type);
}

#define skipspace(C) while ( *C && *C == ' ' ) ++C;
#define copyto(D,S,T) for (; *S && *S != T; *D++=*S++ ) 
#define copyto2(D,S,T,TT) for (; *S && *S != T && *S != TT; *D++=*S++ ) 
#define moveto(S,T) for (; *S && *S != T; ++S )
#define moveto2(S,T,TT) for (; *S && *S != T && *S != TT; ++S )

BOOL _cfg_find(UINT32* pos, CHAR* n, FILE* f)
{
	CHAR line[DIR_MAX_NAME];
	CHAR* c;
	UINT32 l = strlen(n);
	
	while ( 1 ) 
	{
		*pos = ftell(f);
		if ( !fgets(line,512,f) ) return FALSE;
		if ( line[0] == '\n' ) continue;
		c = line;
		skipspace(c);
		if ( *c == '\n' ) continue;
		if ( !strncmp(c,n,l) ) return TRUE;
	}
	return FALSE;
}

VOID _cfg_formatting(FILE* f)
{
	fseek(f,-1,SEEK_END);
	CHAR c = fgetc(f);;
	if ( c != '\n') fputc('\n',f);
	fseek(f,0,SEEK_END);
}

VOID cfg_reset(FILE* f)
{
	fseek(f,0,SEEK_SET);
}

BOOL cfg_read(CHAR* d,CHAR* v, FILE* f)
{
	CHAR line[DIR_MAX_NAME];
	CHAR* c;
	
	while ( 1 ) 
	{
		if ( !fgets(line,512,f) ) return FALSE;
		if ( line[0] == '\n' ) continue;
		c = line;
		skipspace(c);
		if ( *c == '\n' ) continue;
		break;
	}
	
	copyto2(d,c,' ','=');
	*d = '\0';
	
	skipspace(c);
	if ( *c != '=' ) 
	{
		*v = '\0';
		return FALSE;
	}
	
	++c;
	skipspace(c);
	copyto(v,c,'\n');
	*v = '\0';
	
	return TRUE;
}

VOID cfg_write(CHAR* n,CHAR* v, FILE* f)
{
	UINT32 pos;
	
	fseek(f,0,SEEK_END);
	UINT32 enpos = ftell(f);
	fseek(f,0,SEEK_SET);
	
	if ( enpos == 0 )
	{
		fprintf(f,"%s = %s\n",n,v);
		return;
	}
	
	if ( !_cfg_find(&pos,n,f) )
	{
		_cfg_formatting(f);
		fprintf(f,"%s = %s\n",n,v);
		return;
	}
	
	BYTE* b = malloc(enpos + 1);
	UINT32 r = fread(b,1,enpos,f);
	
	fseek(f,pos,SEEK_SET);
	fprintf(f,"%s = %s\n",n,v);
	fwrite(b,1,r,f);
	pos = ftell(f);
	ftruncate(fileno(f),pos);
	free(b);
}
