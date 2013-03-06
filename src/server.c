#include "libsock.h"

gboolean loop_accept(gpointer data)
{
    struct tcp_conn *conn = (struct tcp_conn*)data;

    if(server_accept_conn(conn) == -1)
        return TRUE;

    return FALSE;
}

void server_callback(gpointer data)
{
    struct socket_set *sock_s  = (struct socket_set *) data;

    close(sock_s->remote->inst);
    g_free(sock_s->remote);

    server_accept_conn(sock_s->local);

    g_free(sock_s);
}

gboolean server_recv(gpointer data)
{
    int ret = 255;
    int num_bytes = 0;
    char buf[MAXDATASIZE];
    struct pollfd ufds[1];
    struct functions *func = NULL;
    struct socket_set *sock_s = NULL;

    sock_s = (struct socket_set *)data;

    ufds[0].fd = sock_s->remote->inst;
    ufds[0].events = POLLIN ;
    ret = poll(ufds, 1, 3000);

    if(ret == 0){
        return TRUE;
    } else if(ret == -1){
        goto err;
    } else {
        memset(buf, 0, MAXDATASIZE);
        num_bytes = recv(sock_s->remote->inst, buf, MAXDATASIZE, 0);

        func = get_functions();
        if(func == NULL)
            goto err;

        if(num_bytes <= 0){
            func->close_sock(sock_s->remote);
            return FALSE;
        }

        buf[strlen(buf)] = '\0';

        #ifdef DEBUG
            int i = 0;
            printf("RECV\n{ ");
            for(i=0;i<num_bytes;i++){
                printf("%02x ", buf[i]);
            }
            printf("}\n");
        #endif

        func->new_data(sock_s->remote, buf, num_bytes);
        return TRUE;
    }

err:
    fprintf(stderr, "[%s] - Recv connection fail...\n", __FUNCTION__);
    return FALSE;
}

int server_accept_conn(struct tcp_conn *conn)
{
    struct functions *func = NULL;
    struct socket_set *sock_s = NULL;
    struct tcp_conn *remote_conn = NULL;
    unsigned int sock_len = sizeof(struct sockaddr_in);

    remote_conn = g_new0(struct tcp_conn, 1);
    if(remote_conn == NULL)
        goto err;

    remote_conn->inst = accept(conn->inst, (struct sockaddr *)&remote_conn->address, &sock_len);
    if(remote_conn->inst < 0){
        g_timeout_add(300, loop_accept, conn);
        return 1;
    }

	if(fork() > 0) {
        g_timeout_add(300, loop_accept, conn);
		return 1;
	}

    sock_s = g_new0(struct socket_set, 1);
    if(sock_s == NULL)
        goto err;
    sock_s->local  = conn;
    sock_s->remote = remote_conn;

    g_timeout_add_full(200, 300, server_recv, sock_s, server_callback);

    func = get_functions();
    if(func == NULL)
        goto err;
    func->connected(remote_conn);

    return 0;

err:
    fprintf(stderr, "[%s] - NULL pointer...\n", __FUNCTION__);
    return -1;
}

struct tcp_conn *server_create_sock(int port)
{
    int    sock   = 0;
    long   arg    = 0;
    int    optval = 65535;
    struct sockaddr_in address;
    struct tcp_conn *conn = NULL;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        goto err;
    }

    address.sin_family      = AF_INET;
    address.sin_port        = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(address.sin_zero), 8);

    if( (arg = fcntl(sock, F_GETFL, NULL)) < 0) {
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
    }
    arg |= O_NONBLOCK;
    if( fcntl(sock, F_SETFL, arg) < 0) {
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1){
        fprintf(stderr, "setsockopt() fail..\n");
        goto err;
    }

    if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "BIND fail...\n");
        goto err;
    }

    if(listen(sock, BACKLOG) != 0){
        fprintf(stderr, "Listen fail...\n");
        goto err;
    }

    conn = g_new0(struct tcp_conn, 1);
    if(conn == NULL)
        goto err;

    conn->inst = sock;
    conn->address = address;

    return conn;

err:
    fprintf(stderr, "[%s] - Create socket fail...\n", __FUNCTION__);
    return NULL;
}

void start_server_connection(int port,
    int(*new_data)(struct tcp_conn *conn, char *data, int len),
    int(*close_sock)(struct tcp_conn *conn),
    int(*connected)(struct tcp_conn *conn))
{
    struct tcp_conn *conn = NULL;
    struct functions *func = NULL;

    func = g_new0(struct functions, 1);
    if(func != NULL){
        func->new_data   = new_data;
        func->close_sock = close_sock;
        func->connected  = connected;
        set_functions(func);
    } else {
		printf("Fail alloc memmory\n");
		return;
    }

    conn = server_create_sock(port);
    if(conn != NULL){
        server_accept_conn(conn);
    } else {
		printf("Fail alloc memmory\n");
        return;
    }
}
