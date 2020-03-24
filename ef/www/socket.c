#include <ef/socket.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/str.h>
#include <sys/ioctl.h>

__private ssize_t tls_stream_read(stream_s* sm, void* buf, size_t size){
	tlsSession_s* tls = sm->rwuserdata;
	return tls_read(tls, buf, size);
}

__private ssize_t tls_stream_write(stream_s* sm, void* buf, size_t size){
	tlsSession_s* tls = sm->rwuserdata;
	return tls_write(tls, buf, size);
}

socket_s* socket_new(socketType_e type, socketEvent_f onread, socketEvent_f onclose, void* userdata){
	socket_s* s = mem_new(socket_s);
	if( !s ){
		err_pushno("malloc");
		return NULL;
	}

	s->fd = -1;
	s->onread = onread;
	s->onclose = onclose;
	s->userdata = userdata;
	s->type = type;
	s->stream = NULL;
	s->tls = NULL;

	switch( type ){
		case SOCKET_TYPE_UNIX:
			s->unixs.credentials.gid = 0;
			s->unixs.credentials.pid = 0;
			s->unixs.credentials.uid = 0;
			memset(&s->unixs.addr, 0, sizeof(s->unixs.addr));
			s->unixs.addr.sun_family = AF_UNIX;
		break;
		
		case SOCKET_TYPE_NET4:
			memset(&s->net4.addr, 0, sizeof(s->net4.addr));
			s->net4.addr.sin_family = AF_INET;
		break;

		case SOCKET_TYPE_NET6:
			memset(&s->net6.addr, 0, sizeof(s->net6.addr));
			s->net6.addr.sin6_family = AF_INET6;
		break;

		default:
			err_pushno("unknow type");
			free(s);
		return NULL;
	}

	return s;
}

void socket_close(socket_s* s){
	if( s->tls ){
		tls_session_free(s->tls);
	}

	if( s->fd != -1 ){
		if( shutdown(s->fd, SHUT_RDWR) ){
			dbg_error("shutdown");
			dbg_errno();
		}
		if( close(s->fd) ){
			dbg_error("close");
			dbg_errno();
		}
		s->fd = -1;
	}
	
	if( s->stream ){
		stream_detach(s->stream);
	}

	free(s);
}

void socket_close_auto(socket_s** s){
	if( *s ) socket_close(*s);
}

socket_s* socket_open(socket_s* s, int nonblock, int datagram, size_t enableStream){
	if( s == NULL ){
		dbg_warning("s==NULL");
		return NULL;
	}

	int mode = 0;
	int type = datagram ? SOCK_DGRAM : SOCK_STREAM;
	if( nonblock ) type |= SOCK_NONBLOCK;
	
	switch( s->type ){
		case SOCKET_TYPE_UNIX: mode = AF_UNIX; break;
		case SOCKET_TYPE_NET4: mode = AF_INET; break;
		case SOCKET_TYPE_NET6: mode = AF_INET6; break;
		default:
			err_pushno("unknow type");
			socket_close(s);
		return NULL;
	}


	if( (s->fd = socket(mode, type, 0)) == -1 ){
		err_pushno("socket");
		socket_close(s);
		return NULL;
	}

	if( enableStream && !(s->stream = stream_open_fd(s->fd, enableStream)) ){
		socket_close(s);
		return NULL;
	}

	return s;
}

__private err_t socket_unix_port(socket_s* s, const char* name){
	if( *name == '\0' ){
		*s->unixs.addr.sun_path = '\0';
		strncpy(s->unixs.addr.sun_path+1, name + 1, sizeof(s->unixs.addr.sun_path)-2);
	}
	else{
		strncpy(s->unixs.addr.sun_path, name, sizeof(s->unixs.addr.sun_path)-1);
	}
	return 0;
}

__private err_t socket_net4_port(socket_s* s, int port){
	s->net4.addr.sin_port = htons(port);
	return 0;
}

__private err_t socket_net6_port(socket_s* s, int port){
	s->net6.addr.sin6_port = htons(port);
	return 0;
}

__private err_t socket_unix_bind(socket_s* s){
	if( *s->unixs.addr.sun_path ) unlink(s->unixs.addr.sun_path);

	if( bind(s->fd, (struct sockaddr*)&s->unixs.addr, sizeof(s->unixs.addr)) == -1 ){
		err_pushno("can't bind file");
		return -1;
	}
	
	return 0;
}

__private err_t socket_net4_bind(socket_s* s){
	s->net4.addr.sin_addr.s_addr = INADDR_ANY;
	
	int flag = 1;
	if( setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) ){
		err_pushno("set reuse address");
		return -1;
	}

	if( bind(s->fd, (struct sockaddr*)&s->net4.addr, sizeof(s->net4.addr)) == -1 ){
		err_pushno("can't bind net4");
		return -1;
	}

	return 0;
}

__private err_t socket_net6_bind(socket_s* s){
	s->net6.addr.sin6_addr = in6addr_any;

	int flag = 1;
	if( setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) ){
		err_pushno("set reuse address");
		return -1;
	}

	if( bind(s->fd, (struct sockaddr*)&s->net6.addr, sizeof(s->net6.addr)) == -1 ){
		err_pushno("can't bind net6");
		return -1;
	}

	return 0;
}

socket_s* socket_listen(socket_s* s, esport_u port){
	if( !s ) return NULL;

	switch( s->type ){
		case SOCKET_TYPE_UNIX:
			socket_unix_port(s, port.name);
			socket_unix_bind(s);
		break;

		case SOCKET_TYPE_NET4:
			socket_net4_port(s, port.port);
			socket_net4_bind(s);
		break;

		case SOCKET_TYPE_NET6:
			socket_net6_port(s, port.port);
			socket_net6_bind(s);
		break;

		default:
			err_push("unknow socket type");
			socket_close(s);
		return NULL;
	}
	
	dbg_info("listen on fd:%d",s->fd);
	if( listen(s->fd, SOCKET_SIMULTANEOUS_CONNECTION_MAX) == -1 ){
		err_pushno("can't listen on server");
		socket_close(s);
		return NULL;
	}

	return s;
}

__private err_t socket_get_credentials(socket_s* s){
	socklen_t len = sizeof(struct ucred);
	if( getsockopt(s->fd, SOL_SOCKET, SO_PEERCRED, &s->unixs.credentials, &len) == -1) {
		err_pushno("can't get credentials");
		return -1;
	}
	return 0;
}

socket_s* socket_accept(socket_s* out, socket_s* server, size_t enableStream, long timeoutms, tlsSession_s* tls){
	if( !out ) goto ONERR;

	if( timeoutms ){
		errno = 0;
		switch( fd_timeout(server->fd, timeoutms) ){
			case -1: goto ONERR;
			case  1: goto ONERR;
		}
	}

	if( (out->fd = accept(server->fd, NULL, NULL)) == -1 ){
		err_pushno("can't accept connection");
		goto ONERR;
	}
	dbg_info("accept fd:%d",out->fd);

    if( out->type == SOCKET_TYPE_UNIX ){
		socket_get_credentials(out);
		if( *server->unixs.addr.sun_path ){
			strcpy(out->unixs.addr.sun_path, server->unixs.addr.sun_path);
		}
		else{
			out->unixs.addr.sun_path[0] = 0;
			strcpy(&out->unixs.addr.sun_path[1], &server->unixs.addr.sun_path[1]);
		}
	}

	if( enableStream ){
		if( !(out->stream = stream_open_fd(out->fd, enableStream)) ){
			goto ONERR;
		}
	}
	else{
		out->stream = NULL;
	}

	if( tls && tls_socket_attach(tls, out) ){
		out->tls = tls;
		goto ONERR;
	}

	return out;

ONERR:
	if( out ) socket_close(out);
	return NULL;
}

#define socket_unix_addr(S,A) socket_unix_port(S,A)

__private void socket_net4_addr(socket_s* s, const char* addr){
	inet_pton(AF_INET, addr, &s->net4.addr.sin_addr.s_addr);
}

__private void socket_net6_addr(socket_s* s, const char* addr){
	inet_pton(AF_INET6, addr, &s->net6.addr.sin6_addr);
}

socket_s* socket_connect(socket_s* s, const char* addr, int port, tlsSession_s* tls){
	iassert(s->fd != -1);
	if( !s ) return NULL;

	void* sa;
	size_t size;

	switch( s->type ){
		case SOCKET_TYPE_UNIX:
			socket_unix_addr(s, addr);
			sa = &s->unixs.addr;
			size = sizeof s->unixs.addr;
		break;

		case SOCKET_TYPE_NET4:
			socket_net4_port(s, port);
			socket_net4_addr(s, addr);
			sa = &s->net4.addr;
			size = sizeof s->net4.addr;
		break;

		case SOCKET_TYPE_NET6:
			socket_net6_port(s, port);
			socket_net6_addr(s, addr);
			sa = &s->net6.addr;
			size = sizeof s->net6.addr;
		break;

		default:
			err_push("unknow socket type");
			socket_close(s);
		return NULL;
	}

	if( connect(s->fd, (struct sockaddr*)sa, size) == -1 ){
		err_pushno("can't connect to server");
		socket_close(s);
		return NULL;
	}

	if( tls && tls_socket_attach(tls, s) ){
		socket_close(s);
		return NULL;
	}

	return s;
}

__private thread_local char inet6str[SOCKET_IPV6_LENGHT];
__private const char* inet6_ntoa(struct in6_addr* addr){
	inet_ntop(AF_INET6, addr, inet6str, SOCKET_IPV6_LENGHT - 1);
	inet6str[SOCKET_IPV6_LENGHT-1] = 0;
	return inet6str;
}

const char* socket_addr_get(socket_s* s){
	switch( s->type ){
		case SOCKET_TYPE_UNIX: return *s->unixs.addr.sun_path == 0 ? s->unixs.addr.sun_path + 1 : s->unixs.addr.sun_path;
		case SOCKET_TYPE_NET4: return inet_ntoa(s->net4.addr.sin_addr); break;
		case SOCKET_TYPE_NET6: return inet6_ntoa(&s->net6.addr.sin6_addr); break;
	}
	return NULL;
}

err_t socket_parse_events(deadpoll_s* dp, int event, socket_s* s){
	if( event & EPOLLHUP || event & EPOLLRDHUP ){
		if( s->onclose) return s->onclose(dp, s);
	}
	else if( event & EPOLLIN ) {
		if( s->onread ) return s->onread(dp, s);
	}
	else{
		dbg_warning("unknow event %d",event);
	}
	return 0;
}

int socket_isopen(socket_s* s){
	return s->fd == -1 ? 0 : 1;
}

int socket_error(socket_s* s){
	int ecode;
	unsigned int ecodesize = sizeof(int);
	getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &ecode, &ecodesize);
	return ecode;
}

err_t socket_wifi_info(const char* device, char* essid, int* dbm, int* bitrate){
	int fd;
	struct iwreq rqsk;
	struct iw_statistics iwstat;

	if( (fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		dbg_error("socket");
		dbg_errno();
		return -1;
	}
	
	strcpy(rqsk.ifr_name, device);
	
	rqsk.u.essid.length = IW_ESSID_MAX_SIZE;
	rqsk.u.essid.pointer = essid;
	essid[0] = 0;
	if( ioctl(fd, SIOCGIWESSID, &rqsk) == -1 ){
		dbg_warning("ioctl essid");
		dbg_errno();
		essid[0] = 0;
	}
	else{
		essid[rqsk.u.essid.length] = 0;
	}

	rqsk.u.essid.length = sizeof(struct iw_statistics);
	rqsk.u.essid.pointer = &iwstat;
	if( ioctl(fd, SIOCGIWSTATS, &rqsk) == -1 ){
		*dbm = 0;
	}
	else if( ((struct iw_statistics *)rqsk.u.data.pointer)->qual.updated & IW_QUAL_DBM){
        //signal is measured in dBm and is valid for us to use
        *dbm = ((struct iw_statistics *)rqsk.u.data.pointer)->qual.level - 256;
	}
	else{
		*dbm = 0;
	}

	if( ioctl(fd, SIOCGIWRATE, &rqsk) == -1 ){
		*bitrate = 0;
	}
	else{
		memcpy(&bitrate, &rqsk.u.bitrate.value, sizeof(int));
	}

	close(fd);
	return 0;
} 

err_t tls_session_begin(void){
	return gnutls_global_init();
}

/* From (public domain) example file in GNUTLS
 *
 * This function will try to verify the peer's certificate, and
 * also check if the hostname matches, and the activation, expiration dates.
 */
__private int tls_verify_x509_callback(gnutls_session_t session){
	unsigned int status;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;
	int ret;
	gnutls_x509_crt_t cert;
	tlsSession_s *s = (tlsSession_s*)gnutls_session_get_ptr(session);

	ret = gnutls_certificate_verify_peers2(session, &status);
	if( ret < 0 ){
		err_push("Could not verfify peer certificate due to an error");
		return GNUTLS_E_CERTIFICATE_ERROR;
    }

	if( status & GNUTLS_CERT_INVALID ) err_push("The certificate is not trusted.");
	if( status & GNUTLS_CERT_SIGNER_NOT_FOUND ) err_push("The certificate hasn't got a known issuer.");
	if( status & GNUTLS_CERT_REVOKED ) err_push("The certificate has been revoked.");
	if( status & GNUTLS_CERT_EXPIRED ) err_push("The certificate has expired");
	if( status & GNUTLS_CERT_NOT_ACTIVATED ) err_push("The certificate is not yet activated");
	if( status ) return GNUTLS_E_CERTIFICATE_ERROR;
	
	if( gnutls_certificate_type_get(session) != GNUTLS_CRT_X509) return GNUTLS_E_CERTIFICATE_ERROR;

	if( gnutls_x509_crt_init(&cert) < 0){
		err_push("in initialization");
		return GNUTLS_E_CERTIFICATE_ERROR;
    }

	if( !(cert_list = gnutls_certificate_get_peers(session, &cert_list_size)) ){
		err_push("No certificate was found!");
		return GNUTLS_E_CERTIFICATE_ERROR;
    }

	if( gnutls_x509_crt_import (cert, &cert_list[0], GNUTLS_X509_FMT_DER) < 0 ){
		err_push("parsing certificate");
		return GNUTLS_E_CERTIFICATE_ERROR;
    }

	if( s->hostname && *s->hostname ){
		if( !gnutls_x509_crt_check_hostname (cert, s->hostname) ){
			err_push("The certificate's owner does not match hostname '%s'", s->hostname);
			return GNUTLS_E_CERTIFICATE_ERROR;
		}
    }

	gnutls_x509_crt_deinit (cert);
	dbg_info("Peer passed certificate verification");
	return 0;
}

tlsSession_s* tls_session_new(int mode,	const char* hostname, const char *keyfile, const char *certfile, const char* password, const char *cacertfile){
	int ret;
	tlsSession_s* tls = mem_zero(tlsSession_s);
	if( !tls ){
		err_pushno("malloc");
		return NULL;
	}

	if( hostname ){
		tls->hostname = str_dup(hostname, 0);
		if( tls->hostname == NULL ){
			goto ONERR;
		}
	}

	if( gnutls_certificate_allocate_credentials(&tls->creds) < 0 ){
		err_push("Certificate allocation memory error");
		goto ONERR;
    }

	if( cacertfile != NULL ){
		if( (ret = gnutls_certificate_set_x509_trust_file(tls->creds, cacertfile, GNUTLS_X509_FMT_PEM)) < 0 ){
			err_push("Error setting the x509 trust file: %s", gnutls_strerror(ret));
			goto ONERR;
		}
		if( !(mode & SOCKET_TLS_INSECURE) ){
			gnutls_certificate_set_verify_function(tls->creds, tls_verify_x509_callback);
			gnutls_certificate_set_verify_flags(tls->creds, GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);
		}
    }

	if( keyfile && !certfile ) certfile = keyfile;
	if( certfile != NULL && keyfile != NULL ){
//		if( (ret = gnutls_certificate_set_x509_key_file(tls->creds, certfile, keyfile, GNUTLS_X509_FMT_PEM)) < 0 ){
		if( (ret = gnutls_certificate_set_x509_key_file2(tls->creds, certfile, keyfile, GNUTLS_X509_FMT_PEM, password, 0)) < 0 ){
			err_push("Error loading certificate or key file (cert::%s, key::%s): (%d)%s", certfile, keyfile, ret, gnutls_strerror(ret));
			goto ONERR;
		}
    }

	if( mode & SOCKET_TLS_SERVER ){
		ret = gnutls_init(&tls->session, GNUTLS_SERVER);
    }
	else{
		ret = gnutls_init(&tls->session, GNUTLS_CLIENT);
    }
	if( ret < 0 ){
		err_push("Cannot initialize GNUTLS session: %s", gnutls_strerror (ret));
		goto ONERR;
    }

	gnutls_session_set_ptr(tls->session, tls);
	
	if( (ret = gnutls_set_default_priority(tls->session)) < 0 ){
		err_push("Cannot set default GNUTLS session priority: %s", gnutls_strerror(ret));
		goto ONERR;
    }

	const char *errpos = NULL;
	if( (ret = gnutls_priority_set_direct(tls->session, SOCKET_TLS_PRIORITY, &errpos)) < 0 ){
		err_push("Cannot set GNUTLS session priority: %s", gnutls_strerror(ret));
		goto ONERR;
    }

	gnutls_session_set_ptr(tls->session, tls);

	if( (ret = gnutls_credentials_set(tls->session, GNUTLS_CRD_CERTIFICATE, tls->creds)) < 0 ){
		err_push("Cannot set session GNUTL credentials: %s", gnutls_strerror(ret));
		goto ONERR;
    }

	if( mode & SOCKET_TLS_SERVER ){
		/* requests but does not check a client certificate */
		gnutls_certificate_server_set_request(tls->session, GNUTLS_CERT_REQUEST);
    }

	return tls;

ONERR:
	if( tls->hostname ) free(tls->hostname);
	if( tls->session ) gnutls_deinit(tls->session);
	free(tls);
	return NULL;
}

void tls_session_free(tlsSession_s* tls){
	if( tls->session ) gnutls_deinit( tls->session );
	free(tls->hostname);
	free(tls);
}

err_t tls_socket_attach(tlsSession_s* tls, socket_s* s){
	iassert(tls);
	iassert(s->fd != -1);
	dbg_info("attach:%d",s->fd);
	s->tls = tls;
	int ret;
	gnutls_transport_set_ptr(tls->session, (gnutls_transport_ptr_t)(intptr_t)s->fd);
	if( (ret=gnutls_handshake(tls->session)) < 0 ){
		err_push("TLS handshake failed(%d): %s\n", ret, gnutls_strerror(ret));
		gnutls_bye(tls->session, GNUTLS_SHUT_RDWR);
		return -1;
    }
	if( s->stream ) stream_replace_io(s->stream, tls_stream_read, tls_stream_write, NULL, tls);

	return 0;
}

ssize_t tls_pending(tlsSession_s* tls){
	return gnutls_record_check_pending(tls->session);
}

ssize_t tls_read(tlsSession_s* tls, void* buf, size_t size){
	ssize_t ret;
	do{
		ret = gnutls_record_recv(tls->session, buf, (size_t)size);
	}while (ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN);
	if( ret < 0 ){
		err_push("TLS read failed:%s", gnutls_strerror(ret));
		return -1;
	}
	return ret;
}

ssize_t tls_write(tlsSession_s* tls, void* buf, size_t size){
	ssize_t ret;
	do{
		ret = gnutls_record_send(tls->session, buf, (size_t)size);
	}while (ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN);
	if( ret < 0 ){
		err_push("TLS write failed:%s", gnutls_strerror(ret));
		return -1;
	}
	return ret;
}

