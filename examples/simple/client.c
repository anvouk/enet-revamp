#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <enet/enet.h>

#define MAX_CONNS    1
#define MAX_CHANNELS 2

ENetPeer *
client_new_connection(ENetHost *client, const char *srv_address, uint16_t srv_port, unsigned timeout)
{
    assert(client);

    ENetAddress address;
    enet_address_set_host(&address, srv_address);
    address.port = srv_port;

    // Initiate the connection, allocating the two channels 0 and 1.
    ENetPeer *peer = enet_host_connect(client, &address, MAX_CHANNELS, 0);
    if (!peer) {
        // No available peers for initiating an ENet connection
        return NULL;
    }

    // Send connection request and wait 'timeout' for response
    ENetEvent event;
    if (enet_host_service(client, &event, timeout) > 0) {
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
    if (enet_host_service(client, &event, timeout) > 0) {
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
        NULL,         // no server address, we are a client
        MAX_CONNS,    // max allowed conns
        MAX_CHANNELS, // max allowed channels
        0,            // assumed incoming bandwidth
        0             // assumed outgoing bandwidth
    );
    if (!client) {
        fprintf(stderr, "ERROR: Failed creating ENet client\n");
        goto cleanup;
    }

    ENetPeer *peer = client_new_connection(client, "localhost", 8888, 2000);
    if (peer) {
        printf("Connection to host succeeded.\n");
    } else {
        fprintf(stderr, "ERROR: Connection to host failed.\n");
        goto cleanup;
    }

    const char *packet_data = "Hello there";
    ENetPacket *packet = enet_packet_create(packet_data, strlen(packet_data) + 1, ENET_PACKET_FLAG_RELIABLE);
    if (!packet) {
        fprintf(stderr, "ERROR: Failed creating packer\n");
        enet_peer_reset(peer);
        goto cleanup;
    }
    enet_peer_send(peer, 0, packet);
    printf("Packet containing '%s' has been enqueued to host\n", packet_data);

    ENetEvent event;

    // loop just 10 times for demo purposes
    for (int waits = 10; waits > 0; --waits) {
        while (enet_host_service(client, &event, 100) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                printf(
                    "A packet of length %lu containing %s was received from %u:%u on channel %u.\n",
                    event.packet->dataLength,
                    event.packet->data,
                    event.peer->address.host,
                    event.peer->address.port,
                    event.channelID
                );
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Server disconnected us\n");
                goto done;
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
