#ifndef __EF_SOCKET_H__
#define __EF_SOCKET_H__

#include <ef/type.h>
#include <ef/file.h>
#include <ef/deadpoll.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <linux/wireless.h>

#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>
#include <gnutls/x509.h>
#include <gnutls/abstract.h>

#define SOCKET_IPV6_LENGHT 52
#define SOCKET_EPOLL_EVENTS (EPOLLIN | EPOLLRDHUP | EPOLLHUP)
#define SOCKET_SIMULTANEOUS_CONNECTION_MAX 128
#define SOCKET_EVENT_MAX 256
#define SOCKET_WIFI_ESSID_SIZE IW_ESSID_MAX_SIZE
#define SOCKET_DEADPOLL_EVENT (EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP | EPOLLRDHUP)
#define SOCKET_TLS_PRIORITY "NORMAL:-VERS-TLS-ALL:+VERS-TLS1.2"
#define SOCKET_TLS_SERVER   0x01
#define SOCKET_TLS_INSECURE 0x02

typedef struct esocket socket_s;

typedef int(*socketEvent_f)(deadpoll_s* dp, socket_s* s);

typedef enum { SOCKET_TYPE_UNIX, SOCKET_TYPE_NET4, SOCKET_TYPE_NET6 } socketType_e;

typedef union esport{
	const char* name;
	int port;
}esport_u;

typedef struct tls{
	const char* hostname;
	const char* cert;
	const char* key;
	const char* cacert;
	const char* password;
}tls_s;

typedef struct tlsSession{
	gnutls_certificate_credentials_t creds;
	gnutls_session_t session;
	char *hostname;
}tlsSession_s;

typedef struct esunix{
	struct ucred credentials;
	struct sockaddr_un addr;
}esunix_s;

typedef struct esnet4{
	struct sockaddr_in addr;
}esnet4_s;

typedef struct esnet6{
	struct sockaddr_in6 addr;
}esnet6_s;

typedef struct esocket {
	void* userdata;
	socketEvent_f onread;
	socketEvent_f onclose;
	socketType_e type;
	int fd;
	tlsSession_s* tls;
	stream_s* stream;
	union{
		esunix_s unixs;
		esnet4_s net4;
		esnet6_s net6;
	};
}socket_s;

/** get socket fd*/
#define socket_fd(S) ((S)->fd)

/** get socket type*/
#define socket_type(S) ((S)->type)

/** get socket userdata*/
#define socket_userdata(S) ((S)->userdata)

/** create a socket
 * @param type of socket
 * @param onread callback for read
 * @param onclose callbacl for close
 * @param userdata user data
 * @return obj socket successfull, NULL for error
 */
socket_s* socket_new(socketType_e type, socketEvent_f onread, socketEvent_f onclose, void* userdata);

/** close and free socket
 * @param s socket to close
 * @return 0 successfull, -1 error
 */
void socket_close(socket_s* s);

void socket_close_auto(socket_s** s);

/** cleanup function
 * @see __cleanup
 */
#define __socket_close __cleanup(socket_close_auto)

/** open a socket
 * @param s socket to open, is checked
 * @param nonblock 1 for non blocking socket
 * @param datagram 1 for datagram socket
 * @param enableStream if 0 stream disabled otherwise is size of chunk of stream
 * @return s successfull, NULL and close socket on error
 */
socket_s* socket_open(socket_s* s, int nonblock, int datagram, size_t enableStream);

/** listen
 * @param s socket to init, is checked
 * @param port aadr or port depend if is unix or inet
 * @return s successfull, NULL and closed socket on error
 */
socket_s* socket_listen(socket_s* s, esport_u port);

/** accept a socket
 * @param out socket to accept, init before, is checked
 * @param server socket get a request
 * @param enableStream if 0 stream disabled otherwise is size of chunk of stream
 * @param timeoutms set timeout of accept connection 0 no timeout
 * @param tls a tls session for enable a socket tls
 * @return socket out or null for error or timeout, auto close out socket if error is returned
 */
socket_s* socket_accept(socket_s* out, socket_s* server, size_t enableStream, long timeoutms, tlsSession_s* tls);

/** connect to server
 * @param s socket
 * @param addr address
 * @param port if unix socket port is unused
 * @param tls a tls session for enable a socket tls
 * @return s successfull, NULL on error and socket is closed
 */
socket_s* socket_connect(socket_s* s, const char* addr, int port, tlsSession_s* tls);

/** get address/ip of socket
 * @param s socket
 * @return string rappresentation of address
 */
const char* socket_addr_get(socket_s* s);

/** you can use this inside deadpoll, cast to pollCbk_f*/
err_t socket_parse_events(deadpoll_s* dp, int event, socket_s* s);

/** return 1 if fd is open otherwise 0*/
int socket_isopen(socket_s* s);

/** get socket error*/
int socket_error(socket_s* s);

/** get wifi info
 * @param device name of device
 * @param essid get out essid name, size max IW_ESSID_MAX_SIZE
 * @param dbm power in decibel
 * @param bitrate speed of connection
 * @return 0 successfull, -1 error
 */
err_t socket_wifi_info(const char* device, char* essid, int* dbm, int* bitrate);

/** init function for tls, call at main */
err_t tls_session_begin(void);

/** create a new session tls
 * @param mode SOCKET_TLS_SERVER if is server, SOCKET_TLS_INSECURE if use not secure socket
 * @param hostname hostname
 * @param keyfile key file
 * @param certfile cert file
 * @param password password of cert file
 * @param cacertfile cacert file
 * @return session or NULL for error
 */
tlsSession_s* tls_session_new(int mode,	const char* hostname, const char *keyfile, const char *certfile, const char* password, const char *cacertfile);

/** free a tls session
 * @param tls session
 */
void tls_session_free(tlsSession_s* tls);

/** attach tls session to socket
 * @param tls tls session
 * @param s socket to attach a tls session
 * @return -1 error 0 successfull
 */
err_t tls_socket_attach(tlsSession_s* tls, socket_s* s);

/** check pending data on tls session, check before enter in epoll
 * @param tls session
 * @return -1 error or size of pending data
 */
ssize_t tls_pending(tlsSession_s* tls);

/** same syscall write but on tls */
ssize_t tls_write(tlsSession_s* tls, void* buf, size_t size);

/** same syscall read but on tls */
ssize_t tls_read(tlsSession_s* tls, void* buf, size_t size);


#endif
