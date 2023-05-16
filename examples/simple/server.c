#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <enet/enet.h>

#define MAX_CONNS    32
#define MAX_CHANNELS 2

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

    ENetHost *server = enet_host_create(&server_address, MAX_CONNS, MAX_CHANNELS, 0, 0);
    if (server) {
        printf("ENET server listening on port: %u\n", port);
    } else {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        goto cleanup;
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
                printf(
                    "A packet of length %zu containing %s was received from %u:%u on channel %u.\n",
                    event.packet->dataLength,
                    event.packet->data,
                    event.peer->address.host,
                    event.peer->address.port,
                    event.channelID
                );
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

cleanup:
    printf("Closing ENET server\n");
    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}
