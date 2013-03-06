#include "libsock.h"

static int  port = 0;
static char *dst = NULL;

void client_callback(gpointer data)
{
    struct tcp_conn *lost_conn = (struct tcp_conn *) data;
    struct tcp_conn *conn  = NULL;

    close(lost_conn->inst);
    g_free(lost_conn);

    conn = client_create_sock(dst, port);
    g_timeout_add(300, client_connect, conn);
}

gboolean client_recv(gpointer data)
{
    int ret = 255;
    int num_bytes = 0;
    char buf[MAXDATASIZE];
    struct pollfd ufds[1];
    struct functions *func = NULL;
    struct tcp_conn  *conn = (struct tcp_conn *) data;

    func = get_functions();
    if(func == NULL)
        goto err;

    ufds[0].fd = conn->inst;
    ufds[0].events = POLLIN ;
    ret = poll(ufds, 1, 5000);

    if(ret == 0){
        return TRUE;
    } else if (ret == -1){
        goto err;
    } else {
        memset(buf, 0, MAXDATASIZE);
        num_bytes = recv(conn->inst, buf, MAXDATASIZE, 0);

        if(num_bytes <= 0){
            func->close_sock(conn);
            goto err;
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

        func->new_data(conn, buf, num_bytes);
        return TRUE;
    }

err:
    fprintf(stderr, "[%s] - Recv connection fail...\n", __FUNCTION__);
    return FALSE;
}

gboolean client_connect(gpointer data)
{
    int ret = 0;
    struct functions *func = NULL;
    struct tcp_conn *conn = (struct tcp_conn *) data;

    func = get_functions();
    if(func == NULL){
        fprintf(stderr, "[%s] - get_functions fail...\n", __FUNCTION__);
        return FALSE;
    }

    ret = connect(conn->inst,(struct sockaddr *)&conn->address, sizeof(struct sockaddr));
    if(ret == 0){
        func->connected(conn);
        g_timeout_add_full(200, 300, client_recv, conn, client_callback);
        return FALSE;
    } else { 
        func->close_sock(conn);
        return TRUE;
    }
}

struct tcp_conn *client_create_sock(char *dst, int port)
{
    int    sock = 0;
    long   arg  = 0;
    struct hostent *he    = NULL;
    struct tcp_conn *conn = NULL;
    struct sockaddr_in address;

    if ((he = gethostbyname(dst)) == NULL){
        goto err;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        goto err;
    }

    address.sin_family = AF_INET;
    address.sin_port   = htons(port);
    address.sin_addr   = *((struct in_addr *)he->h_addr);
    bzero(&(address.sin_zero), 8);

    if( (arg = fcntl(sock, F_GETFL, NULL)) < 0) {
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
    }
    arg |= O_NONBLOCK;
    if( fcntl(sock, F_SETFL, arg) < 0) {
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
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

void start_client_connection(char *remote_address,
    int remote_port,
    int(*new_data)(struct tcp_conn *conn, char *data, int len),
    int(*close_sock)(struct tcp_conn *conn),
    int(*connected)(struct tcp_conn *conn))
{
    struct tcp_conn *conn  = NULL;
    struct functions *func = NULL;

    dst  = remote_address;
    port = remote_port;

    func = g_new0(struct functions, 1);
    if(func != NULL){
        func->new_data   = new_data;
        func->close_sock = close_sock;
        func->connected  = connected;
        set_functions(func);
    } else {
		printf("create_sock memory fail\n");
		return;
    }

    conn = client_create_sock(dst, port);
    if(conn != NULL){
        g_timeout_add(300, client_connect, conn);
    } else {
		printf("create_sock memory fail\n");
		return;
    }
}
