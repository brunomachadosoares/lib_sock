#include "libsock.h"

#define MAX_LEN     500
#define MIN_LEN     5

static FILE *arq = NULL;
static int flag_trace = 0;
static char filename[100];
static struct tm *ret_local_date(void);

void activ_trace(void)
{
    int cmd_len = 0;
    struct tm *local = NULL;

    local = ret_local_date();

    snprintf(filename, cmd_len, "trace_%02d%02d%02d%02d.log",
            local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);

    flag_trace = 1;
}

static struct tm *ret_local_date(void)
{
    time_t t;
    struct tm *local;

    t     = time(NULL);
    local = localtime(&t);

    return local;
}

static int open_file(void)
{
    arq = fopen(filename, "a+");
    if(arq == NULL){
        fprintf(stderr, "Fail Open Log File!\n");
        return (-1);
    }

    return 0;
}

static void write_log(char *line)
{
    struct tm *local = NULL;

    if(open_file() != 0)
        return;

    local = ret_local_date();

    fprintf(arq, "[%02d/%02d,%02d:%02d:%02d] - %s\n", local->tm_mday, local->tm_mon+1,
                local->tm_hour, local->tm_min, local->tm_sec, line);

    fclose(arq);
}

void trace(char *format, ...)
{
    int n = 0;
    va_list args;
    char *line = NULL;

    if(flag_trace != 1)
        return;

    va_start(args, format);

    n = vsnprintf(NULL, 0, format, args);

    if(n > MIN_LEN && n < MAX_LEN){
        line = g_malloc0(n * sizeof(char));
        vsnprintf(line, n, format, args);
    }
    va_end(args);

    write_log(line);

    if(line)
        free(line);
}
