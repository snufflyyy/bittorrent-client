#pragma once

#include <stdbool.h>

#include "types.h"
#include "metadata.h"

typedef struct TorrentPeer {
    bool connected;

    i32 socket;
    char ip[32];
    char port[16];
} TorrentPeer;

TorrentPeer* torrent_peer_connect(char ip[32], char port[16], char peer_id[20], TorrentMetadata* metadata);
void torrent_peer_send_request(TorrentPeer* peer, TorrentMetadata* metadata, usize index, usize begin, usize length);
void torrent_peer_destroy(TorrentPeer* peer);
