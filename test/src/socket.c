#include "test.h"
#include <ef/socket.h>

//TODO controllare la fine della sessione

#define SOCK_BUF 4096

__private const char* tlsKeyPath = "../tls/wildcard.key";
__private const char* tlsCertPath = "../tls/wildcard.crt";
//__private const char* tlsKeyPath = "../tls/key.pem";
//__private const char* tlsCertPath = "../tls/cert.pem";

/*@test -S --socket 'test socket arg-a set client/client.tls/server/server.tls, arg-b type:addr:port, where type is unix or net4. write exit to end'*/

//__private void tls_path_free(tls_s* files){
//	if( files->cacert ) free((void*)files->cacert);
//	if( files->cert ) free((void*)files->cert);
//	if( files->hostname ) free((void*)files->hostname);
//	if( files->key ) free((void*)files->key);
//	if( files->password ) free((void*)files->password);
//}

__private err_t server_client_close(deadpoll_s* dp, socket_s* socket){
	if( socket->type == SOCKET_TYPE_UNIX ){
		printf("%s(%d):close\n", socket_addr_get(socket), socket->unixs.credentials.pid);
	}
	else{
		printf("%s:close\n", socket_addr_get(socket));
	}
	deadpoll_unregister(dp, socket_fd(socket));
	//if( socket_userdata(socket) ) tls_path_free(socket_userdata(socket));
	socket_close(socket);
	return 0;
}

__private err_t server_client_inp(deadpoll_s* dp, socket_s* socket){
	char* line;
	size_t nr;
	
	while( stream_kbhit(socket->stream) && (nr=stream_inp(socket->stream, &line, '\n', 1)) > 0 ){
		printf("%s > %s", socket_addr_get(socket), line);
		if( !str_equal(line, nr, "close\n", strlen("close\n")) ){
			if( !dp ){
				puts("server error not dp");
			}
			else{
				server_client_close(dp, socket);
			}
			return 0;
		}
		stream_out(socket->stream, line, strlen(line));
		free(line);
	}
	stream_flush(socket->stream);

	return 0;
}

__private err_t server_accept(__unused deadpoll_s* dp, socket_s* server){
	printf("request connection:");
	fflush(stdout);
	tls_s* files = socket_userdata(server);

	dbg_info("accept");
	if( files ) dbg_info("cert:%s", files->cert);
	if( files ) dbg_info("key :%s", files->key);

	socket_s* servlet = socket_accept(
		socket_new(server->type, server_client_inp, server_client_close, NULL),
		server,
		SOCK_BUF,
		0,
		files ? 
			tls_session_new(SOCKET_TLS_SERVER,files->hostname, files->key, files->cert,files->password, files->cacert)
		:
			NULL
	);

	if( !servlet ){
		err_print();
		return -1;
	}

	if( files ){
		if( tls_pending(servlet->tls) ){
			servlet->onread(NULL, servlet);	
		}
	}

	if( deadpoll_register(dp, socket_fd(servlet), (pollCbk_f)socket_parse_events, servlet, SOCKET_DEADPOLL_EVENT, NULL) ){
		printf("err register");
		err_print();
		socket_close(servlet);
		return 0;
	}
	
	printf("%s\n", socket_addr_get(servlet));

	return 0;
}

__private err_t server_close(__unused deadpoll_s* dp, __unused socket_s* server){
	dbg_info("server close");
	return 0;
}

__private void test_server(socketType_e type, const char* addr, int port, int tls){
	dbg_info("server");
	dbg_info("addr:%s", addr);
	dbg_info("port:%d", port);
	dbg_info("tls :%d", tls);

	tls_s files = {
		.hostname = NULL,
		.cert = path_resolve(tlsCertPath),
		.key  = path_resolve(tlsKeyPath),
		.cacert = NULL,
		.password = NULL	
	};
	dbg_info("cert:%s", files.cert);
	dbg_info("key :%s", files.key);

	esport_u pu;
	if( type == SOCKET_TYPE_UNIX ){
		pu.name = addr;
	}
	else{
		pu.port = port;
	}

	__socket_close socket_s* server = socket_listen( 
		socket_open(
			socket_new(type, server_accept, server_close, tls ? &files : NULL),
			0,0,0
		),
		pu
	);

	if( !server ){
		dbg_error("no server");
		goto ONERR;
	}

	stream_s* inp = stream_open_fd(0, 4096);
   	if( !inp ){
		dbg_error("no inp");
		goto ONERR;
	}

	deadpoll_s* dps = deadpoll_new();
	if( !dps ) goto ONERR;
	if( deadpoll_register(dps, socket_fd(server), (pollCbk_f)socket_parse_events, server, SOCKET_DEADPOLL_EVENT, NULL) ){
		deadpoll_free(dps);
		goto ONERR;
	}

	printf("server run\n");
	deadpoll_loop(dps, -1);	
	
	deadpoll_free(dps);
	free((void*)files.cert);
	free((void*)files.key);

	return;
ONERR:
	dbg_error("an error");
	free((void*)files.cert);
	free((void*)files.key);
	err_print();
}

void test_client(socketType_e type, const char* addr, int port, int tls){
	dbg_info("client");
	dbg_info("addr:%s", addr);
	dbg_info("port:%d", port);
	dbg_info("tls :%d", tls);

	tls_s files = {
		.hostname = NULL,
		.cert = path_resolve(tlsCertPath),
		.key  = path_resolve(tlsKeyPath),
		.cacert = NULL,
		.password = NULL
	};

	__socket_close socket_s* client = socket_connect(
		socket_open(
			socket_new(type, NULL, NULL, NULL),
			0,
			0,
			SOCK_BUF
		),
		addr,
		port,
		tls ?
			tls_session_new(0, files.hostname, files.key, files.cert,files.password, files.cacert)
		:
			NULL
	);

	stream_s* inp = stream_open_fd(0, 4096);
	if( !inp ) goto ONERR;
	while( 1 ){
		fputs("client $ ", stdout);
		fflush(stdout);
		
		char* line;
		ssize_t nr;
		nr = stream_inp(inp, &line, '\n', 1);
		if( !str_equal(line, nr, "exit\n", strlen("exit\n")) ){
			free(line);
			break;
		}

		if( stream_out(client->stream, line, nr) ){
			free(line);
			goto ONERR;
		}
		stream_flush(client->stream);
		free(line);
	
		nr = stream_inp(client->stream, &line, '\n', 1);
		if( nr < 1 || line == NULL ) goto ONERR;
		if( !str_equal(line, nr, "exit\n", strlen("exit\n")) ){
			free(line);
			break;
		}
		
		printf("> %s", line);
	}
	
	free((void*)files.cert);
	free((void*)files.key);
	return;

ONERR:
	free((void*)files.cert);
	free((void*)files.key);
	err_print();
}



__private err_t argb_addr(const char* argb, socketType_e* type, char** addr, int* port){
	//dbg_info("argb::%s",argb);
	const char* parse = strchr(argb, ':');
	if( parse == NULL ){
		err_push("wrong argb format, use type:addr:port");
		return -1;
	}
	size_t len = parse - argb;
	if( !str_equal(argb, len, "unix", strlen("unix")) ){
		*type = SOCKET_TYPE_UNIX;
	}
	else if( !str_equal(argb, len, "net4", strlen("net4")) ){
		*type = SOCKET_TYPE_NET4;
	}
	else{
		err_push("wrong type(%.*s) use: unix/net4", (int)len, argb);
		return -1;
	}
	//dbg_info("next:%s",parse);
	
	++parse;
	const char* next = strchr(parse, ':');
	if( next == NULL ){
		if( *type != SOCKET_TYPE_UNIX ){
			err_push("no port for net4");
			return -1;
		}
		len = strlen(parse);
		//dbg_info("unix.addr.len::%lu", len);
		*addr = str_dup(parse, len);
		return 0;
	}
	len = next - parse;
	*addr = str_dup(parse, len);
	//dbg_info("net4.addr.len::%lu", len);
	
	parse = next + 1;
	//dbg_info("next:%s", parse);
	if( *parse == 0 ) return 0;

	errno = 0;
	char* en;
	*port = strtoul(parse, &en, 10);
	if( !en || en == parse || errno ){
		if( errno ){
			err_pushno("invalid port %s", parse);
		}
		else{
			err_push("invalid port %s", parse);
		}
		free(*addr);
		return -1;
	}

	return 0;
}

/*@fn*/
void test_socket(const char* argA, const char* argB){
	err_enable();
	if( tls_session_begin() ){
		err_print();
		dbg_error("tls");
		return;
	}

	socketType_e type;
	char* addr;
	int port;
	if( argb_addr(argB, &type, &addr, &port) ){
		err_print();
		dbg_error("argb");
		return;
	}
	
	if( !str_equal(argA, strlen(argA), "client", strlen("client")) ){
		test_client(type, addr, port, 0);
	}
	else if( !str_equal(argA, strlen(argA), "server", strlen("server")) ){
		test_server(type, addr, port, 0);
	}
	else if( !str_equal(argA, strlen(argA), "client.tls", strlen("client.tls")) ){
		test_client(type, addr, port, 1);
	}
	else if( !str_equal(argA, strlen(argA), "server.tls", strlen("server.tls")) ){
		test_server(type, addr, port, 1);
	}
	else{
		printf("error, use client or server on arga");
	}
	//dbg_info("addr:%s", addr);
	free(addr);
	err_restore();
}
