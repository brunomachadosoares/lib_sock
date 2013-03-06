#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <glib.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <stdarg.h>

#define BACKLOG     1
#define MAXDATASIZE 4096
#define PRINT_      fprintf(stderr, "[%s] - Line [%d]\n", __FUNCTION__, __LINE__);

struct tcp_conn {
    int    inst;
    struct sockaddr_in address;
};

struct socket_set {
    struct tcp_conn *local;
    struct tcp_conn *remote;
};

struct functions {
    int(*new_data)(struct tcp_conn *conn, char *data, int len);
    int(*close_sock)(struct tcp_conn *conn);
    int(*connected)(struct tcp_conn *conn);
};

void start_server_connection(
    int port,
    int(*new_data)(struct tcp_conn *conn, char *data, int len),
    int(*close_sock)(struct tcp_conn *conn),
    int(*connected)(struct tcp_conn *conn));

void start_client_connection(
    char *remote_address,
    int remote_port,
    int(*new_data)(struct tcp_conn *conn, char *data, int len),
    int(*close_sock)(struct tcp_conn *conn),
    int(*connected)(struct tcp_conn *conn));

struct tcp_conn *server_create_sock(int port);
struct tcp_conn *client_create_sock(char *dst, int port);

int  msg_write(struct tcp_conn *conn, char *data, int len);
int  activate_log(void);
int  server_accept_conn(struct tcp_conn *conn);
void server_callback(gpointer data);
void client_callback(gpointer data);
void trace(char *format, ...);
void activ_trace(void);
void set_functions(struct functions *p);
struct functions *get_functions(void);

gboolean server_recv(gpointer data);
gboolean client_recv(gpointer data);
gboolean client_connect(gpointer data);
