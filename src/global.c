#include "libsock.h"

static struct functions *server = NULL;

void set_functions(struct functions *p)
{
    if(p == NULL){
        fprintf(stderr, "[%s] - Fail...\n", __FUNCTION__);
        return;
    }

    server = p;
    server->new_data   = p->new_data;
    server->close_sock = p->close_sock;
    server->connected  = p->connected;
}

struct functions *get_functions(void)
{
    return server;
}

int msg_write(struct tcp_conn *conn, char *msg, int len)
{
    if(conn == NULL ){
        fprintf(stderr, "conn NULL\n");
        return -1;
    }

    if(msg == NULL){
        fprintf(stderr, "msg NULL... \n");
        return -1;
    }

    if(len <= 0){
        fprintf(stderr, "len <= 0\n");
        return -1;
    }

    #ifdef DEBUG
        int i = 0;
        printf("SEND\n{ ");
        for(i=0;i<len;i++){
        printf("%02x ", msg[i]);
        }
        printf("}\n");
    #endif

    if(send(conn->inst, (unsigned char *)msg, len, 0) != 0){
        return -1;
    }

    return 0;
}
