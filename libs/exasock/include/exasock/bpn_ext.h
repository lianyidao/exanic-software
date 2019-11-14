/**
 * author: Lian Xuezhi
 * date: 20190904
 */
#ifndef EXASOCK_API_BPN_EXT_H
#define EXASOCK_API_BPN_EXT_H
#include "extensions.h"
#include <time.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


int exasock_accelerated(int fd);

int exasock_exanic_interface(const char *ifname);

int64_t exasock_get_tx_ns(int fd);


#ifdef __cplusplus
}
#endif

#endif // header
