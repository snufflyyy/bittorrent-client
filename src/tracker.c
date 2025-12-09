#include "tracker.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "bencode.h"
#include "utils/url.h"
#include "types.h"

/* returns an empty TorrentTrackerResult with .failed = true */
TorrentTrackerResult torrent_tracker_get(TorrentMetadata* metadata, const char* peer_id) {
    TorrentTrackerResult result = {0};

    URLSplitResult url = url_split(metadata->announce);

    struct addrinfo address_hints = {0};
    address_hints.ai_family = AF_UNSPEC; // allows for either IPv4 or IPv6, it doesnt matter to us
    address_hints.ai_socktype = SOCK_STREAM; // use TCP

    struct addrinfo* address_info;
    i32 status;
    if ((status = getaddrinfo(url.host, url.port, &address_hints, &address_info)) != 0) {
        fprintf(stderr, "[ERROR] [TRACKER] Failed to get address infomation: %s\n", gai_strerror(status));
        return (TorrentTrackerResult) { .failed = true };
    }

    i32 tracker_socket = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (tracker_socket == -1) {
        freeaddrinfo(address_info);
        fprintf(stderr, "[ERROR] [TRACKER] Failed to create socket!\n");
        return (TorrentTrackerResult) { .failed = true };
    }

    if (connect(tracker_socket, address_info->ai_addr, address_info->ai_addrlen) == -1) {
        fprintf(stderr, "[ERROR] [TRACKER] Failed to connect: %s:%s%s!\n", url.host, url.port, url.path);
        freeaddrinfo(address_info);
        close(tracker_socket);
        return (TorrentTrackerResult) { .failed = true };
    }

    freeaddrinfo(address_info);

    char* info_hash_string = url_encode(metadata->info_sha1, sizeof(metadata->info_sha1));

    char request[512];
    snprintf(request, sizeof(request), "GET %s?info_hash=%s&peer_id=%s HTTP/1.1\r\n\r\n", url.path, info_hash_string, peer_id);

    free(info_hash_string);

    if (send(tracker_socket, request, strlen(request), 0) == -1) {
        fprintf(stderr, "[ERROR] [TRACKER] Failed to send request!\n");
        close(tracker_socket);
        return (TorrentTrackerResult) { .failed = true };
    }

    u8 buffer[2048];
    usize total_bytes = 0;
    while (true) {
        i32 bytes_received = recv(tracker_socket, buffer + total_bytes, sizeof(buffer) - total_bytes, 0);
        if (bytes_received == -1) {
            fprintf(stderr, "[ERROR] [TRACKER] Failed to receive response!\n");
            close(tracker_socket);
            return (TorrentTrackerResult) { .failed = true };
        } else if (bytes_received == 0) {
            break;
        }

        total_bytes += bytes_received;
    }

    close(tracker_socket);

    if (total_bytes > sizeof(buffer)) {
        fprintf(stderr, "[ERROR] [TRACKER] Response is too long!\n");
        return (TorrentTrackerResult) { .failed = true };
    }

    u8* bencode = memmem(buffer, total_bytes, "\r\n\r\n", 4);
    if (!bencode) {
        fprintf(stderr, "[ERROR] [TRACKER] Response is invalid!\n");
        for (usize i = 0; i < sizeof(buffer); i++) {
            printf("%c", buffer[i]);
        }
        printf("\n");
        return (TorrentTrackerResult) { .failed = true };
    }

    bencode += 4;
    usize bencode_length = total_bytes - (bencode - buffer);

    usize bencoded_respone_index = 0;
    BencodeObject* bencoded_response = bencode_object_parse((u8*) bencode, bencode_length, &bencoded_respone_index);
    if (!bencoded_response) {
        fprintf(stderr, "[ERROR] [TRACKER] Response's bencode is invalid!\n");
        return (TorrentTrackerResult) { .failed = true };
    }

    BencodeObject* bencoded_interval = bencode_object_dictionary_get(bencoded_response, "interval");
    result.interval = bencoded_interval->number;

    BencodeObject* bencoded_peers = bencode_object_dictionary_get(bencoded_response, "peers");
    result.peers_length = bencoded_peers->list_length;

    result.peers = (TorrentTrackerPeer*) malloc(sizeof(TorrentTrackerPeer) * result.peers_length);
    if (!result.peers) {
        fprintf(stderr, "[ERROR] [TRACKER] Failed to allocate memory for peers!\n");
        bencode_object_destroy(bencoded_response);
        return (TorrentTrackerResult) { .failed = true };
    }

    for (usize i = 0; i < result.peers_length; i++) {
        BencodeObject* bencoded_peer_ip = bencode_object_dictionary_get(bencoded_peers->list[i], "ip");
        memcpy(result.peers[i].ip, bencoded_peer_ip->string, sizeof(result.peers[i].ip));

        BencodeObject* bencoded_peer_port = bencode_object_dictionary_get(bencoded_peers->list[i], "port");
        snprintf(result.peers[i].port, sizeof(result.peers[i].port), "%i", bencoded_peer_port->number);
    }

    bencode_object_destroy(bencoded_response);
    return result;
}

void torrent_tracker_result_print(TorrentTrackerResult* result) {
    printf("interval: %u\n", result->interval);
    printf("peers:\n");
    for (usize i = 0; i < result->peers_length; i++) {
        printf("\t%s:%s\n", result->peers[i].ip, result->peers[i].port);
    }
}

void torrent_tracker_result_destroy(TorrentTrackerResult* result) {
    if (result->peers) { free(result->peers); }
    result->peers = NULL;
    result->peers_length = 0;
}
