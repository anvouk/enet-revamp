#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <enet/enet.h>

#include "shared.h"

#define BUFFER_SZ 64
static char buffer[BUFFER_SZ] = { 0 };

ENetPeer *
client_new_connection(ENetHost *client, const char *srv_address, uint16_t srv_port, unsigned timeout)
{
    assert(client);

    ENetAddress address;
    enet_address_set_host(&address, srv_address);
    address.port = srv_port;

    // Initiate the connection, allocating the two channels 0 and 1.
    ENetPeer *peer = enet_host_connect(client, &address, HOST_MAX_CHANNELS, 0);
    if (!peer) {
        // No available peers for initiating an ENet connection
        return NULL;
    }

    // Send connection request and wait 'timeout' for response
    ENetEvent event;
    while (enet_host_service(client, &event, timeout) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            return peer;
        case ENET_EVENT_TYPE_DISCONNECT:
            // Server refused connection
            enet_peer_reset(peer);
            return NULL;
        case ENET_EVENT_TYPE_RECEIVE:
            // Should not receive any packet at the connection stage
            enet_packet_destroy(event.packet);
            assert(0);
            return NULL;
        default:
            // Unexpected event type at this stage
            assert(0);
            return NULL;
        }
    }

    // 'timeout' has expired without receiving nothing
    enet_peer_reset(peer);
    return NULL;
}

bool
client_close_connection(ENetHost *client, ENetPeer *peer, unsigned timeout)
{
    // Enqueue disconnection event
    enet_peer_disconnect(peer, 0);

    ENetEvent event;
    while (enet_host_service(client, &event, timeout) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            // packets received during disconnection attempt are discarded
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            return true;
        default:
            // Unexpected event type at this stage
            assert(0);
            break;
        }
    }
    return false;
}

int
main(void)
{
    if (enet_initialize() != 0) {
        fprintf(stderr, "ERROR: Failed initializing ENet.\n");
        return EXIT_FAILURE;
    }

    ENetHost *client = enet_host_create(
        NULL,              // no server address, we are a client
        1,                 // max allowed conns
        HOST_MAX_CHANNELS, // max allowed channels
        0,                 // assumed incoming bandwidth
        0                  // assumed outgoing bandwidth
    );
    if (!client) {
        fprintf(stderr, "ERROR: Failed creating ENet client\n");
        goto cleanup;
    }

    ENetPeer *peer = NULL;
    while (!peer) {
        peer = client_new_connection(client, "localhost", 8888, 2000);
        if (peer) {
            printf("Connection to host succeeded.\n");
        } else {
            fprintf(stderr, "ERROR: Connection to host failed.\n");
        }
    }

    // bootstrap the PING-PONG sequence here
    printf("Sending first ping\n");
    ENetPacket *first_packet = enet_packet_create(PING, strlen(PING) + 1, ENET_PACKET_FLAG_RELIABLE);
    if (!first_packet) {
        fprintf(stderr, "ERROR: Failed creating bootstrapper PING packet\n");
        goto done;
    }
    enet_peer_send(peer, 0, first_packet);

    ENetEvent event;

    // loop just 10 times for demo purposes
    while (1) {
        while (enet_host_service(client, &event, 100) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                // parse incoming data
                if (safe_parse_packet_data(buffer, BUFFER_SZ, event.packet->data, event.packet->dataLength) != 0) {
                    fprintf(stderr, "ERROR: Failed parsing incoming packet data\n");
                    enet_packet_destroy(event.packet);
                    enet_peer_reset(peer);
                    goto done;
                }

                // act on received data: send PING on PONG
                if (strcmp(buffer, PONG) == 0) {
                    printf("PONG received\n");
                    ENetPacket *packet = enet_packet_create(PING, strlen(PING) + 1, ENET_PACKET_FLAG_RELIABLE);
                    if (!packet) {
                        fprintf(stderr, "ERROR: Failed creating PING packet\n");
                        enet_packet_destroy(event.packet);
                        enet_peer_reset(peer);
                        goto done;
                    }
                    enet_peer_send(peer, 0, packet);
                } else {
                    fprintf(stderr, "WARNING: unknown packet date received: '%s'\n", buffer);
                }

                // remember to destroy the packet
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Server disconnected us\n");
                goto cleanup;
            default:
                printf("other unexpected packet type received: %d\n", event.type);
                break;
            }
        }
    }

done:
    if (client_close_connection(client, peer, 2000)) {
        printf("Disconnetion succeeded\n");
    } else {
        fprintf(stderr, "ERROR: Disconnection to host failed.\n");
    }

cleanup:
    printf("Closing ENET client\n");
    if (client) {
        enet_host_destroy(client);
    }
    enet_deinitialize();
    return 0;
}
