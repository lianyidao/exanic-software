#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "exasock/bpn_ext.h"

#define BUF_LEN 2048

int main (int argc, char *argv[])
{
    struct sockaddr_in sa;
    char *p;
    char **arg = argv;
    int tcp = 0;
    int fd;
    int err = 0;
    int n = 0;
    int len;
    char buf[BUF_LEN];
    int64_t ns;

    const char *port_str;
    const char *addr_str;

    /* Parse command line arguments */

    if (argc < 3)
        goto usage_error;

    if (strcmp("-t", argv[1]) == 0) {
        tcp = 1;
        ++arg;
    }

    addr_str = arg[1];
    port_str = arg[2];

    fd = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    if (fd < 0)
    {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        err = EXIT_FAILURE;
        goto err_socket;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(10000);
    if (0 && !tcp) err = bind(fd, (struct sockaddr *) &sa, sizeof(sa));
    if (err) {
        fprintf(stderr, "bind: %s (%d)\n", strerror(errno), err);
        goto exit;
    }

    sa.sin_family = AF_INET;
    if (inet_aton(addr_str, &sa.sin_addr) == 0)
        goto usage_error;
    sa.sin_port = htons(strtol(port_str, &p, 10));
    if (port_str[0] == '\0' || *p != '\0')
        goto usage_error;

    if (tcp) err = connect(fd, (struct sockaddr *)&sa, sizeof(sa));
    if (err)
    {
        fprintf(stderr, "connect: %s (%d)\n", strerror(errno), err);
        err = EXIT_FAILURE;
        goto exit;
    }
    //fprintf(stderr, "connected to %s:%s\n", addr_str, port_str);

    strcpy(buf, "hello\n");
    len = strlen(buf) + 1;
    n = sendto(fd, buf, len, 0, (void *) &sa, sizeof(sa));
    //n = send(fd, buf, len, 0);
    if (-1 == n)
    {
        fprintf(stderr, "send: %s (%d)\n", strerror(errno), err);
        err = EXIT_FAILURE;
        goto exit;
    }

    ns = exasock_get_tx_ns(fd);
    printf("tx_ns=%ld\n", ns);


exit:
    close(fd);

    ns = exasock_get_tx_ns(fd);
    printf("closed,tx_ns=%ld\n", ns);

err_socket:
    return err;

usage_error:
    fprintf(stderr,
            "Usage: exasock %s <server-addr> <server-port>\n"
            "\n",
            argv[0]);

    return EXIT_FAILURE;
}
