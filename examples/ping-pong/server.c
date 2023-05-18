#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <enet/enet.h>

#include "shared.h"

#define BUFFER_SZ 64
static char buffer[BUFFER_SZ] = { 0 };

int
main(void)
{
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    uint16_t port = 8888;

    ENetAddress server_address;
    server_address.host = ENET_HOST_ANY;
    server_address.port = port;

    ENetHost *server = enet_host_create(&server_address, HOST_MAX_CONNS, HOST_MAX_CHANNELS, 0, 0);
    if (server) {
        printf("ENET server listening on port: %u\n", port);
    } else {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        goto done;
    }

    bool has_finished = false;
    ENetEvent event;

    while (!has_finished) {
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf("New client connected\n");
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // parse incoming data
                if (safe_parse_packet_data(buffer, BUFFER_SZ, event.packet->data, event.packet->dataLength) != 0) {
                    fprintf(stderr, "ERROR: Failed parsing incoming packet data, dropping\n");
                    enet_packet_destroy(event.packet);
                    continue;
                }

                // act on received data: send PONG on PING
                if (strcmp(buffer, PING) == 0) {
                    printf("PING received\n");
                    ENetPacket *packet = enet_packet_create(PONG, strlen(PONG) + 1, ENET_PACKET_FLAG_RELIABLE);
                    if (!packet) {
                        fprintf(stderr, "ERROR: Failed creating PONG packet\n");
                        enet_packet_destroy(event.packet);
                        goto done;
                    }
                    enet_peer_send(event.peer, 0, packet);
                } else {
                    fprintf(stderr, "WARNING: unknown packet date received: '%s'\n", buffer);
                }

                // remember to destroy the packet
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Client has disconnected\n");
                has_finished = true;
                break;
            default:
                printf("other unexpected packet type received: %d\n", event.type);
                break;
            }
        }
    }

done:
    printf("Closing ENET server\n");
    if (server) {
        enet_host_destroy(server);
    }
    enet_deinitialize();
    return 0;
}
