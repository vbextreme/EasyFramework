#ifndef __EF_OAUTH_H__
#define __EF_OAUTH_H__

#include <ef/type.h>
#include <ef/http.h>

#define OAUTH_RND_STR_SIZE 65

typedef struct oauth2Authorization{
	const char* authorizationLink;
	const char* tokenLink;
	const char* refreshTokenLink;
	const char* httpReplyOk;
	const char* httpReplyError;
	const char* appClientId;
	const char* appClientSecret;
	const char* scope;
	const char* accessType;
}oauth2Authorization_s;

typedef struct oauth2{
	http_s* http;
	char* codeReply;
	char* token;
	char* tokenRefresh;
	char* scope;
	char rndStr[OAUTH_RND_STR_SIZE];
	size_t expires;
	time_t expiresTime;
}oauth2_s;

/** create oauth2 call mth_random_init before begin */
oauth2_s* oauth2_new(wwwProgress_s* prog);

/** free oauth
 * @param oa obj
 */
void oauth2_free(oauth2_s* oa);

/** execute browser for authorizate application and give code, after show code need to set in oauth2.codeReply
 * @param oa obj initializated
 * @param auth structure contain information user
 * @param browser browser name for execute
 * @param timeoutms timeout
 * @return token or NULL for error
 */
err_t oauth2_athorization_browser(oauth2_s* oa, oauth2Authorization_s* auth, const char* browser, long timeoutms);

/** get athorization token
 * @param oa obj initializated
 * @param auth structure contain information user
 * @return 0 succesfull -1 error
 */
err_t oauth2_authorization_token_get(oauth2_s* oa, oauth2Authorization_s* auth);

/** return remaning time in seconds before expire token*/
long oauth2_authorization_remaining_time(oauth2_s* oa);

/** save oauth2 */
err_t oauth2_authorization_store(oauth2_s* oa, oauth2Authorization_s* auth, const char* path);

/** load oauth2 */
err_t oauth2_authorization_load(oauth2_s* oa, oauth2Authorization_s* auth, const char* path);

/** refresh athorization token
 * @param oa obj initializated
 * @param auth structure contain information user
 * @return 0 succesfull -1 error
 */
err_t oauth2_authorization_token_refresh(oauth2_s* oa, oauth2Authorization_s* auth);

/** return 1 if not need authorization browser, otherwise 0*/
int oauth2_authorization_ok(oauth2_s* oa);

#endif 
