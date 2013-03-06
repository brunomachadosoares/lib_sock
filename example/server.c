#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "libsock.h"

int new_data(struct tcp_conn *conn, char *data, int len)
{
	int i = 0;

	printf("recv from %d { ", conn->inst);
	for(i=0; i<len; i++) {
		printf("%02x ", data[i]);
	}
	printf("}\n");

	msg_write(conn, data, len);

	return 0;
}

int close_cnx(struct tcp_conn *conn)
{
	close(conn->inst);
	return 0;
}

int connected(struct tcp_conn *conn)
{
   	return 0;
}

int main(void)
{
    GMainLoop *loop;

	start_server_connection(9090, new_data, close_cnx, connected);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
