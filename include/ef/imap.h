#ifndef __EF_IMAP_H__
#define __EF_IMAP_H__

#include <ef/type.h>
#include <ef/www.h>
#include <ef/vector.h>

typedef enum { 
	IMAP_FETCH_FLAGS_ERROR = -1,
	IMAP_FETCH_FLAGS_SEEN, 
	IMAP_FETCH_FLAGS_ANSWERED, 
	IMAP_FETCH_FLAGS_FLAGGED, 
	IMAP_FETCH_FLAGS_DELETED, 
	IMAP_FETCH_FLAGS_DRAFT 
} imapFetchFlags_e;

typedef struct imapMimeContent{
	struct imapMimeContent* next; /**< next mime*/
	char* type; /**< content-type*/
	char* filename; /**< filename if exists */
	char* disposition; /**< content-disposition, example attachment*/
	void* data; /**< data of content*/
	size_t dataSize; /**< size of data*/
}imapMimeContent_s;

typedef struct imapMime{
	int xauth; /**< xaut pass/fail*/
	int xsid; /**< xsid pass/fail*/
	char* from; /**< from eamil*/
	char* to; /**< to email*/
	char* replyto; /**< reply from*/
	char* date; /**<date message*/
	char* subject; /**<subject*/
	char* boundary; /**< boundary, its used for farse content*/
	imapMimeContent_s* content; /**< all content*/
}imapMime_s;

typedef struct imapFetch{
	size_t id; /**<id*/
	size_t size; /**<size of fetch*/
	const char* mime; /**<unparsed mime*/
	imapFetchFlags_e flag; /**< flag*/
}imapFetch_s;

typedef struct imapExamine{
	size_t exists; /**<exists mexage, exists id is most recent message*/
	size_t recent; /**<recent*/
	size_t unseeid; /**<first unsee id*/
}imapExamine_s;

typedef struct imap{
	www_s www;
	char* server;
	size_t lenServer;
}imap_s;

/** create imap
 * @param bufferupsize minimal buffer size
 * @param flags see www flags
 * @param prog see www prog
 * @return imap or NULL on error
 */
imap_s* imap_new(size_t bufferupsize, int flags, wwwProgress_s* prog);

/** end of used imap
 * @param ima imap object
 */
void imap_free(imap_s* ima);

/** set email and pass before use imap, init before call this
 * @param ima imap object
 * @param email your email@server.com
 * @param pass password
 */
void imap_email_password(imap_s* ima, const char* email, const char* pass);

/** set imap server
 * @param ima imap object
 * @param server server address for examples: imaps://outlook.office365.com
 * @return -1 error 0 suzzessfull
 */
err_t imap_server(imap_s* ima, const char* server);

/** list directory
 * @param ima imap object
 * @return vector of char** contains dir name, need to free each element of vector
 */
char** imap_ls(imap_s* ima);

/** number of exist, unsee messages
 * @param out structure out info
 * @param ima imap object
 * @param dir directory name
 * @return -1 error 0 suzzessfull
 */
err_t imap_examine(imapExamine_s* out, imap_s* ima, const char* dir);

/** get a messages, not free return object
 * @param ima imap object
 * @param dir directory name
 * @param nobody if set get only header message
 * @param from get from id
 * @param to get to id
 * @return NULL for error 
 */
imapFetch_s* imap_fetch(imap_s* ima, const char* dir, int nobody, size_t from, size_t to);

/** delete mime
 * @param mime mime object
 */
void imap_mime_delete(imapMime_s* mime);

/** free all mime
 * @param mime mime object
 * @param count count of mime
 */
void imap_mime_free(imapMime_s* mime, size_t count);

/** convert fetch to mime, call imap_mime_free after used imapMime_s
 * @param fetch object
 * @param count count of fetched messages
 * @return NULL for error
 */
imapMime_s* imap_fetch_to_mime(imapFetch_s* fetch, size_t count);

/** change imap flag
 * @param ima imap object
 * @param dir directory name
 * @param id id to set/remove flags
 * @param mode + for add flag - for remove flag
 * @param flag flag to change
 * @return -1 error 0 suzzessfull
 */
err_t imap_flags(imap_s* ima, const char* dir, size_t id, char mode, imapFetchFlags_e flag);


#endif
