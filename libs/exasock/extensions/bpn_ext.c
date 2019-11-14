#include "../common.h"

#include <netinet/ether.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sched.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include <../../exanic/config.h>
#include <../../exanic/time.h>
#include <../../exanic/fifo_tx.h>

#include "../lock.h"
#include "../rwlock.h"
#include "../structs.h"
#include "../override.h"
#include "../exanic.h"


__attribute__((visibility("default")))
int
exasock_accelerated(int fd)
{
    struct exa_socket * restrict sock = exa_socket_get(fd);
    return sock && sock->bypass_state == EXA_BYPASS_ACTIVE;
}

__attribute__((visibility("default")))
int
exasock_exanic_interface(const char *ifname)
{
    char dev[16];
    int port;
    int n;

    exasock_override_off();
    n = exanic_find_port_by_interface_name(ifname, dev, sizeof(dev), &port);
    exasock_override_on();

    return 0 == n;
}


static struct exa_socket *
read_lock_exasock(int fd)
{
    struct exa_socket * restrict sock = exa_socket_get(fd);
    if (NULL == sock) NULL;

    exa_read_lock(&sock->lock);
    if (sock->bypass_state != EXA_BYPASS_ACTIVE) {
        exa_read_unlock(&sock->lock);
        return NULL;
    }

    // keep read lock
    return sock;
}

__attribute__((visibility("default")))
int64_t
exasock_get_tx_ns(int fd)
{
    struct exa_socket * restrict sock = read_lock_exasock(fd);
    if (NULL == sock) return 0;

    exanic_tx_t *tx = exanic_get_tx(sock);
    if (NULL == tx) {
        exa_read_unlock(&sock->lock);
        return 0;
    }

    exanic_cycles32_t n32 = exanic_get_tx_timestamp(tx);
    exanic_cycles_t n = exanic_expand_timestamp(tx->exanic, n32);
    int64_t ns = exanic_cycles_to_ns(tx->exanic, n);
    exa_read_unlock(&sock->lock);
    return ns;
}
