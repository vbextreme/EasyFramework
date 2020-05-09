#ifndef __EF_FILE_H__
#define __EF_FILE_H__

#include <ef/type.h>
#include <ef/rbuffer.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libtar.h>
#include <sys/mman.h>

/**************/
/*** file.c ***/
/**************/

#ifndef FILE_BUFFER
	#define FILE_BUFFER 4096
#endif
#ifndef FILE_CHUNK
	#define FILE_CHUNK 4096
#endif

typedef struct filetmp{
	char name[PATH_MAX]; /**< tmp file name*/
	int fd; /**tmp file fd*/
}filetmp_s;

typedef struct stat stat_s;
typedef struct utimbuf utimbuf_s;
typedef FILE file_t;
typedef struct dirent dirent_s;
typedef DIR dir_s;
typedef TAR tar_t;

#define __file_close __cleanup(file_close_auto)

/**************/
/*** path.c ***/
/**************/

/** set 0 last */
void path_kill_back(char* path);

/** copy last /name to begin of string*/
void path_set_last(char* path);

/** copy current path
 * @param path a buffer with PATH_MAX size
 * @return 0 successfull, -1 error, err is pushed and errno is setted
 */
err_t path_current(char* path);

/** current path
 * @return string allocated with new path, remember to free; NULL for error, no err is pushed errno is setted*/
char* path_current_new(void);

/** set current path*/
#define path_current_set(PATH) chdir(PATH)

/** add next to path, add /
 * @param path destination path
 * @param next path to add a path
 * @return 0 for successfull; -1 for error, err is pushed
 */
err_t path_add(char* path, const char* next);

/** store home directory to path
 * @param path a buffer with PATH_MAX size
 * @return 0 successfull, -1 error, err is pushed and errno is setted
 */
err_t path_home(char* path);

/** resolve path begin with spcial char
 * @param path path need to be resolve
 * @return path allocate, remember to free; NULL for error, err is pushed
 */
char* path_resolve(const char* path);

/** same path_resolve but with custom path*/
char* path_resolve_custom(const char* path, const char* home, const char* current);

/*************/
/*** dir.c ***/
/*************/

/** file is block devices*/
#define FILE_BLOCK_DEVICES DT_BLK
/** file is char device*/
#define FILE_CHARACTER_DEVICES DT_CHR
/** file is directory*/
#define FILE_DIRECTORY DT_DIR
/** file is pipe or fifo*/
#define FILE_NAMED_PIPE DT_FIFO
/** file is symbolic link*/
#define FILE_SYMBOLIC_LINK DT_LNK
/** file is regular file*/
#define FILE_FILE DT_REG
/** file is socket*/
#define FILE_SOCKET DT_SOCK
/** file is unknown*/
#define FILE_UNKNOWN DT_UNKNOWN

/** only for conventions*/
#define dir_open(PATH) opendir(PATH)
/** only for conventions*/
#define dir_close(D) closedir(D)
/** only for conventions*/
#define dir_read(D) readdir(D)

/** automatic close directory
 * @see __cleanup
 */
void dir_close_auto(dir_s** dir);

/** preperty for auto close dire when exit from scope
 * @see __cleanup
 */
#define __dir_close __cleanup(dir_close_auto)

/** entity name*/
#define dirent_name(ENT) (ENT)->d_name
/** entity type*/
#define dirent_type(ENT) (ENT)->d_type

/** return 1 if entity is dot or double dots*/
int dirent_currentback(dirent_s* ent);

/** foreach 
 * @param DR dir_s
 * @param IT dirent_s name
 */
#define dir_foreach(DR, IT) for( dirent_s* IT = dir_read(DR); IT; IT = dir_read(DR) )

/**************/
/*** file.c ***/
/**************/

/** return pointer to file extension name*/
char const* file_extension(char const* name);

/** check file exists
 * @param path file to check
 * @return 1 if file exists otherwise 0, no err is pushed, errno is setted */
int file_exists(const char* path);

/** auto close file
 * @see __cleanup
 */
void file_close_auto(file_t** file);

/** duplicate FILE*/
FILE* file_dup(FILE* file, char* mode);

/** remove file or directory with all file
 * @param path to delete
 * @return 0 for successfull; -1 for error, err is pushed errno is setted
 */
err_t file_rm(const char* path);

/** only for conventions*/
#define file_stat(PATH, PTRSTAT_S) stat(PATH, PTRSTAT_S)

/************/
/*** fd.c ***/
/************/

/** auto close fd
 * @see __cleanup
 */
void fd_close_auto(int* fd);

/** open new file descriptor
 * @param path file to open
 * @param mode select mode to open file, is the same of fopen
 * r open for read, error if not exists
 * r+ open for read write, error if not exists
 * w open for write, create if not exists or truncate if exists
 * w+ same w with read
 * ! before w or r, add exclusive flags, with w fail if file exists
 * & before w or r, add non block flags
 * @param privileges privileges of file, only if you create new file, otherwise you can pass 0
 * @return new fd, -1 for error
 */
int fd_open(const char* path, const char* mode, int privileges);

/** create a temporany file
 * @param out struct filetmp contains opened fd with flags !w+, and filename
 * @param path where to create path, the path need % char where create random value and size need less PATH_MAX-32
 * @param privileges privileges for file
 * @return 0 successfull -1 error
 */
err_t fd_open_tmp(filetmp_s* out, const char* path, int privileges);

/** count buffer
 * @param fd file descriptor
 * @return how many char waiting for reads
 */
ssize_t fd_kbhit(int fd);

/** read file
 * @param fd file descriptor
 * @param buf where store data
 * @param size size in byte of buffer
 * @return -1 error or count of bytes readed
 */
ssize_t fd_read(int fd, void* buf, size_t size);

/** read file untle have data or chunk is not full
 * @param fd file descriptor
 * @param buf where store data
 * @param chunk size in byte of chunk
 * @return -1 error or count of bytes readed
 */
ssize_t fd_read_chunk(int fd, void* buf, size_t chunk);

/** read all file
 * @param fd file descriptor
 * @param outlen len of data readed, can pass NULL
 * @param chunk size of block to read
 * @param nullchar if enabled add 0 to the end of buffer
 * @return pointer to mem where stored data, remember to free this, NULL for error
 */
void* fd_slurp(size_t* outlen, int fd, size_t chunk, int nullchar);

/** write file
 * @param fd file descriptor
 * @param buf data to write
 * @param size size in byte of buffer
 * @return -1 error or count of bytes writed
 */
ssize_t fd_write(int fd, void* buf, size_t size);

/** sizeof file
 * @param fd file descriptor
 * @return file size in bytes
 */
size_t fd_size(int fd);

/** map file in address of memory
 * @param fd to be mmap
 * @param len len of file
 * @param mode to be mapped, * for shared mem, w for enable write, r for enable read, e for enable execute, + for add w and r on the selected falgs
 * @return address or NULL for error, release memory with mem_shared_close
 * @see mem_shared_close
 */
void* fd_mmap(int fd, size_t len, char* mode);

/** copy files
 * @param fdo file output
 * @param fdi file input
 * @return -1 error or 0 for successfull
 */
err_t fd_copy(int fdo, int fdi);

/** read data from ring buffer, fit from fd is cbuffer is empty
 * @param fd file descriptor to read data
 * @param cb ring buffer
 * @param out buffer to read
 * @param size read all element setting by size, not exit before
 * @return -1 error or count of bytes readed
 */
ssize_t fd_read_cb(int fd, rbuffer_s* cb, void* out, size_t size);

/** read line
 * @param fd file descriptor
 * @param cb ring bugger
 * @param withnl add new line at end of string
 * @return memory where stored string, you need to free this, otherwise return NULL
 */
char* fd_read_cbline(int fd, rbuffer_s* cb, int withnl);

/** only for name conventions*/
#define fd_close(FD) close(FD)
/** only for name conventions*/
#define fd_truncate(FD,SIZE) ftruncate(FD, SIZE)
/** only for name conventions*/
#define fd_seek(FD, POS, AT) lseek(FD, POS, AT)
/** only for name conventions*/
#define fd_tell(FD) lseek(FD, 0, SEEK_CUR)
/** only for name conventions*/
#define fd_rewind(FD) lseek(FD,0,SEEK_SET)
/** only for name conventions*/
#define fd_toend(FD) lseek(FD, 0, SEEK_END)
/** create fd from memory with pseudoname*/
#define fd_mem_create(NAME) memfd_create(NAME,0)
/** only for name conventions*/
#define fd_stat(FD, PTRSTAT_S) fstat(FD,PTRSTAT_S)

/** for cleanup
 * @see __cleanup
 */
#define __fd_close   __cleanup(fd_close_auto)

/** wait data to fd
 * @param fd fd to wait
 * @param timeoutms time to get timeout, -1 infinite, 0 not wait, > 0 ms
 * @return -1 error, 0 data with no timeout, 1 timeout 
 */
int fd_timeout(int fd, long timeoutms);


/************/
/*** gz.c ***/
/************/

/** extract file gz
 * @param fdin file input
 * @param fdout file output
 * @return -1 error 0 otherwise
 */
err_t fgz_extract(int fdout, int fdin);

/*************/
/*** tar.c ***/
/*************/

/** open a tar file
 * @param path if orfd == -1 try to open filename
 * @param orfd fd where readind tar file, if -1 open and read from path
 * @param mode only support r, w is a to do
 * @return NULL for error or tar object
 */
tar_t* ftar_open(const char* path, int orfd, const char* mode);

/** reading a tar, 1 have read 0 no more data to read*/
#define ftar_read(T) th_read(T)

/** close a tar*/
#define ftar_close(T) tar_close(T)

/** cleanup function
 * @see __cleanup
 */
void ftar_close_auto(tar_t** tar);

/** cleanup function
 * @see __cleanup
 */
#define __ftar_close __cleanup(ftar_close_auto)

/** get name of current tar*/
#define ftar_get_name(T) ((T)->th_buf.name)

/** get path of current tar*/
#define ftar_get_path(T) th_get_pathname(tar)

/** if you not extract regular file you need to skip him*/
#define ftar_skip_reg(T) tar_skip_regfile(T)

/** check if tar is dir*/
#define ftar_isdir(T) TH_ISDIR(T)

/** check if tar is regular file*/
#define ftar_isreg(T) TH_ISREG(tar)

/** extract file
 * @param tar tar obj
 * @param fdout where write file
 * @return 0 successfull -1 error
 */
err_t ftar_extract_reg(tar_t* tar, int fdout);

/****************/
/*** stream.c ***/
/****************/

typedef struct stream stream_s;

typedef ssize_t (*customio_f)(stream_s*, void*, size_t);

typedef struct streamBuffer{
	char* buf;     /**< pointer to rwbuffer for stream*/
	size_t size;   /**< size of buffer*/
	size_t len;    /**< data len in buffer*/
	size_t cursor; /**< position*/
}streamBuffer_s;

typedef struct stream{
	streamBuffer_s r; /**< read buffer*/
	streamBuffer_s w; /**< write buffer*/
	int fd;           /**< file descriptor*/
	customio_f read;  /**< function used for read*/
	customio_f write; /**< function used for write*/
	customio_f kbhit; /**< function used for replace kbhit*/ 
	void* rwuserdata; /**< user data */
	char  rwbuffer[]; /**< private buffer memory */
}stream_s;

#ifndef STREAM_CHUNK 
	#define STREAM_CHUNK 4096
#endif 

#ifndef STREAM_STRING_CHUNK
	#define STREAM_STRING_CHUNK 4096
#endif

#ifndef STREAM_STRING_FIT_REALLOC
	#define STREAM_STRING_FIT_REALLOC _Y_
#endif

#ifndef STREAM_NUM_MAX_DIGIT
	#define STREAM_NUM_MAX_DIGIT 64
#endif

/** open stream from fd
 * @param fd fd to associate to stream
 * @param chunk size of buffer
 * @return stream successfull; NULL error, err is pushed errno is setted
 */
stream_s* stream_open_fd(int fd, size_t chunk);

/** open stream from path
 * @param path file to open
 * @param mode how open file
 * @see fd_open
 * @param privileges privileges of file
 * @param chunk size of buffe
 * @return stream successfull; NULL error, err is pushed errno is setted
 */
stream_s* stream_open(const char* path, const char* mode, int privileges, size_t chunk);

/** open stream from tmp file
 * @param outpathmax the output file name
 * @param path file to open
 * @see fd_open_tmp
 * @param privileges privileges of file
 * @see fd_open
 * @param chunk size of buffer
 * @return stream successfull; NULL error, err is pushed errno is setted
 */
stream_s* stream_open_tmp(char* outpathmax, const char* path, int privileges, size_t chunk);

/** open stream from file in memory
 * @param name filename to open
 * @param chunk size of buffer
 * @return stream successfull; NULL error, err is pushed errno is setted
 */
stream_s* stream_open_mem(const char* name, size_t chunk);

/** replace read/write with custom read/write function*/
void stream_replace_io(stream_s* sm, customio_f r, customio_f w, customio_f kbhit, void* iouserdata);

/** detach stream stream from fd, stream is free fd is not closed
 * @param sm stream object to be close
 */
void stream_detach(stream_s* sm);

/** close stream
 * @param sm stream object to be close
 */
void stream_close(stream_s* sm);

/** close stream for cleanup
 * @see __cleanup
 */
void stream_close_auto(stream_s** sm);

/**cleanup
 * @see __cleanup
 */
#define __stream_close __cleanup(stream_close_auto)

/** get size of all stream
 * @param sm stream object
 * @return size of stream; -1 error, err is pushed errno is setted
 */
ssize_t stream_size(stream_s* sm);

/** get size can be read from stream
 * @param sm stream object
 * @return size can be reading; -1 error, err is pushed errno is setted
 */
ssize_t stream_kbhit(stream_s* sm);

/** read from stream
 * @param sm stream object
 * @param buf out buffer
 * @param size size of data to be read
 * @return size of read; -1 error, err is pushed errno is setted
 */
ssize_t stream_read(stream_s* sm, void* buf, size_t size);

/** push back data to stream buffer
 * @param sm stream object
 * @param data data
 * @param size size of data to be rollback
 */
void stream_rollback(stream_s* sm, void* data, size_t size);

/** read all stream
 * @param outlen optional pointer to len of data readed
 * @param sm stream object
 * @param addNullChar add 0 to the end of read
 * @return memory allocated with all stream readed, remember to free; NULL for error, err is pushed errno is setted
 */
void* stream_slurp(size_t* outlen, stream_s* sm, int addNullChar);

/** input char
 * @param sm stream object
 * @param ch pointer to store char, if no data left ch == -1
 * @return nemelemnt readed; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_char(stream_s* sm, int* ch);

/** input string
 * @param sm stream object
 * @param out string allocated, need to be free
 * @param endch char when end input
 * @param addch add endch to the end string
 * @return nelement readed, set NULL to out if no other data available; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_string(stream_s* sm, char** out, char endch, int addch);

/** input string contains anyof chars
 * @param sm stream object
 * @param out string allocated, need to be free
 * @param endch any char when end input
 * @param addch add endch to the end string
 * @return nelement readed, set NULL to out if no other data available; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_toanyof(stream_s* sm, char** out, char* endch, int addch);

/** input string with contains endstr
 * @param sm stream object
 * @param out string allocated, need to be free
 * @param endstr string for end input
 * @param addch add string to the end string
 * @return nelement readed, set NULL to out if no other data available; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_strstr(stream_s* sm, char** out, char* endstr, int addch);

/** input long
 * @param sm stream object
 * @param num pointer to output num
 * @param base base of number
 * @return nelement readed; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_long(stream_s* sm, long* num, int base);

/** input unsigned long
 * @param sm stream object
 * @param num pointer to output num
 * @param base base of number
 * @return nelement readed; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_ulong(stream_s* sm, unsigned long* num, int base);

/** input double
 * @param sm stream object
 * @param num pointer to output num
 * @return element readed; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_double(stream_s* sm, double* num);

/** input float
 * @param sm stream object
 * @param num pointer to output num
 * @return nelement readed; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_inp_float(stream_s* sm, float* num);

/** overloading stream_inp* **/
#define stream_inp(STREAM, VAR, arg...) _Generic(VAR,\
	int* : stream_inp_char,\
	char**: stream_inp_string,\
	long* : stream_inp_long,\
	unsigned long* : stream_inp_ulong,\
	size_t: stream_inp_long,\
	double: stream_inp_double,\
	float : stream_inp_float\
)(STREAM, VAR, ## arg)

/** flush output
 * @param sm stream object
 */
void stream_flush(stream_s* sm);

/** write to stream
 * @param sm stream object
 * @param buf buffer
 * @param size size of data to be write
 * @return size of write; -1 error, err is pushed errno is setted
 */
ssize_t stream_write(stream_s* sm, void* buf, size_t size);

/** output char
 * @param sm stream object
 * @param ch char to write
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
err_t stream_out_char(stream_s* sm, char ch);

/** output string
 * @param sm stream object
 * @param str string
 * @param len len of string, if 0 len is setted to strlen(str)
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
err_t stream_out_string(stream_s* sm, char* str, size_t len);

/** fork Lukas Chmela GPLv3, write long value to stream
 * @param sm stream object
 * @param num num to write
 * @param base base of number
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
err_t stream_out_long(stream_s* sm, long num, int base);

/** fork Lukas Chmela GPLv3, write unsigned long value to stream
 * @param sm stream object
 * @param num num to write
 * @param base base of number
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
err_t stream_out_ulong(stream_s* sm, unsigned long num, int base);

/** write double value to stream
 * @param sm stream object
 * @param num num to write
 * @param dec numbers to view after point
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
err_t stream_out_double(stream_s* sm, double num, int dec);

/** overloading stream_out*/
#define stream_out(STREAM, VAR, arg...) _Generic(VAR,\
	char:  stream_out_char,\
	char*: stream_out_string,\
	int:   stream_out_long,\
	long:  stream_out_long,\
	unsigned int:  stream_out_ulong,\
	unsigned long: stream_out_ulong,\
	double: stream_out_double,\
	float : stream_out_double\
)(STREAM, VAR, ## arg)

/** printf to stream, sm stream object, format same printf, args
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
__printf(2,3) err_t stream_printf(stream_s* sm, const char* format, ...);

/** get stat structure from stream
 * @param sm stream object
 * @param stat stat_s
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
err_t stream_stat(stream_s* sm, stat_s* stat);

/** return position of stream
 * @param sm stream object
 * @return 0 position; -1 error, err is pushed, ernno is setted
 */
ssize_t stream_tell(stream_s* sm);

/** move stream to position
 * @param sm stream object
 * @param offset to move
 * @param mode SEEK_CUR SEEK_SET SEEK_END
 * @return 0 successfull; -1 error, err is pushed, ernno is setted
 */
err_t stream_seek(stream_s* sm, ssize_t offset, int mode);

/** skip anyof char in lst*/
err_t stream_skip_anyof(stream_s* sm, const char* lst);

/** skip space and tab*/
#define stream_skip_h(STREAM) stream_skip_anyof(STREAM, " \t")

/** skip space tab newline*/
#define stream_skip_hnl(STREAM) stream_skip_anyof(STREAM, " \t\n")

/** skip word [a-zA-Z_]*/
#define stream_skip_w(STREAM) stream_skip_anyof(STREAM, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_")

/** skip numbers [0-9a-fA-FxX_.]*/
#define stream_skip_n(STREAM) stream_skip_anyof(STREAM, "0123456789abcdefxABCDEFX.")

/** skip word and number [0-9a-zA-Z_]*/
#define stream_skip_wn(STREAM) stream_skip_anyof(STREAM, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_.")

/**skip to any of char in lst*/
err_t stream_skip_to(stream_s* sm, const char* lst);

/**skip to next char on lst*/
err_t stream_skip_next(stream_s* sm, const char* lst);

/**skip a line*/
#define stream_skip_line(STREAM) stream_skip_next(STREAM, "\n")

#endif
