#ifndef ENET_SHARED_H
#define ENET_SHARED_H

#include <stdint.h>

#define HOST_MAX_CONNS    32
#define HOST_MAX_CHANNELS 1

#define PING "PING"
#define PONG "PONG"

int
safe_parse_packet_data(char *buff, size_t buff_sz, uint8_t *data, size_t data_sz)
{
    size_t min_sz = data_sz > buff_sz ? buff_sz : data_sz;
    memcpy(buff, data, min_sz);
    // ensure last char is terminating char
    buff[min_sz] = '\0';
    return 0;
}

#endif // ENET_SHARED_H
