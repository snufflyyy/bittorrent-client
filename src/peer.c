#include "peer.h"
#include "metadata.h"
#include "types.h"

#include <asm-generic/socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void torrent_peer_handshake_send(TorrentPeer* peer, TorrentMetadata* metadata, const char* peer_id);
static bool torrent_peer_handshake_validate(u8 handshake_data[68], TorrentMetadata* metadata);

static void buffer_write_big_endian(u8* buffer, u32 value);

TorrentPeer* torrent_peer_connect(char ip[32], char port[16], char peer_id[20], TorrentMetadata* metadata) {
    TorrentPeer* peer = (TorrentPeer*) malloc(sizeof(TorrentPeer));
    if (!peer) {
        fprintf(stderr, "[ERROR] [PEER] Failed to allocate memory for peer!\n");
        return NULL;
    }

    struct addrinfo address_hints = {0};
    address_hints.ai_family = AF_UNSPEC; // allows for either IPv4 or IPv6, it doesnt matter to us
    address_hints.ai_socktype = SOCK_STREAM; // use TCP

    struct addrinfo* address_info;
    i32 status;
    if ((status = getaddrinfo(ip, port, &address_hints, &address_info)) != 0) {
        fprintf(stderr, "[ERROR] [PEER] Failed to get address infomation: %s\n", gai_strerror(status));
        free(peer);
        return NULL;
    }

    peer->socket = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (peer->socket == -1) {
        fprintf(stderr, "[ERROR] [PEER] Failed to create socket!\n");
        freeaddrinfo(address_info);
        free(peer);
        return NULL;
    }

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (setsockopt(peer->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "[ERROR] [PEER] Failed to set receive timeout!\n");
        freeaddrinfo(address_info);
        free(peer);
        return NULL;
    }

    if (setsockopt(peer->socket, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "[ERROR] [PEER] Failed to set send timeout!\n");
        freeaddrinfo(address_info);
        free(peer);
        return NULL;
    }

    if (connect(peer->socket, address_info->ai_addr, address_info->ai_addrlen) == -1) {
        fprintf(stderr, "[ERROR] [PEER] Failed to connect: %s:%s!\n", ip, port);
        close(peer->socket);
        freeaddrinfo(address_info);
        free(peer);
        return NULL;
    }

    freeaddrinfo(address_info);
    peer->connected = true;

    torrent_peer_handshake_send(peer, metadata, peer_id);

    u8 buffer[16384];
    i32 bytes_received = recv(peer->socket, buffer, sizeof(buffer), 0);
    if (bytes_received == 0 || bytes_received == -1) {
        fprintf(stderr, "[ERROR] [PEER] Failed to receive response!\n");
        close(peer->socket);
        free(peer);
        return NULL;
    }

    if (!torrent_peer_handshake_validate(buffer, metadata)) {
        fprintf(stderr, "[ERROR] [PEER] Failed to validate peer handshake!\n");
        close(peer->socket);
        free(peer);
        return NULL;
    }

    torrent_peer_send_request(peer, metadata, 0, 0, sizeof(buffer));

    bytes_received = recv(peer->socket, buffer, sizeof(buffer), 0);
    if (bytes_received == 0 || bytes_received == -1) {
        fprintf(stderr, "[ERROR] [PEER] Failed to receive response!\n");
        close(peer->socket);
        free(peer);
        return NULL;
    }

    printf("bytes received: %d\n", bytes_received);

    for (usize i = 0; i < bytes_received; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    return peer;
}

void torrent_peer_send_request(TorrentPeer* peer, TorrentMetadata* metadata, usize index, usize begin, usize length) {
    u8 request_data[17];
    u8* position = request_data;

    buffer_write_big_endian(position, 13);
    position += 4;

    *position = 6;
    position += 1;

    buffer_write_big_endian(position, index);
    position += 4;

    buffer_write_big_endian(position, begin);
    position += 4;

    buffer_write_big_endian(position, length);
    position += 4;

    if (send(peer->socket, request_data, sizeof(request_data), 0) == -1) {
        fprintf(stderr, "[ERROR] [PEER] [REQUEST] Failed to send request data!\n");
        close(peer->socket);
        return;
    }
}

void torrent_peer_destroy(TorrentPeer* peer) {
    close(peer->socket);
    free(peer);
}

static void torrent_peer_handshake_send(TorrentPeer* peer, TorrentMetadata* metadata, const char* peer_id) {
    u8 handshake_data[68];
    u8* position = handshake_data;

    *position = 19;
    position += 1;

    memcpy(position, "BitTorrent protocol", 19);
    position += 19;

    for (usize i = 0; i < 8; i++) {
        position[i] = 0;
    }
    position += 8;

    memcpy(position, metadata->info_sha1, 20);
    position += 20;

    memcpy(position, peer_id, 20);
    position += 20;

    if (send(peer->socket, handshake_data, sizeof(handshake_data), 0) == -1) {
        fprintf(stderr, "[ERROR] [PEER] [HANDSHAKE] Failed to send handshake data!\n");
        close(peer->socket);
        return;
    }
}

static bool torrent_peer_handshake_validate(u8 handshake_data[68], TorrentMetadata* metadata) {
    if (handshake_data[0] != 19) { return false; }
    if (memcmp(handshake_data + 1, "BitTorrent protocol", 19) != 0) { return false; }
    if (memcmp(handshake_data + 28, metadata->info_sha1, 20) != 0) { return false; }

    return true;
}

static void buffer_write_big_endian(u8* buffer, u32 value) {
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = (value) & 0xFF;
}
