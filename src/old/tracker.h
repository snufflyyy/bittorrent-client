#pragma once

#include <stdbool.h>

#include "metadata.h"
#include "types.h"

typedef struct TorrentTrackerPeer {
    char id[20];
    char ip[32];
    char port[16];
} TorrentTrackerPeer;

typedef struct TorrentTrackerResult {
    bool failed;
    char* failed_reason;

    u32 interval;
    TorrentTrackerPeer* peers;
    usize peers_length;
} TorrentTrackerResult;

TorrentTrackerResult torrent_tracker_get(TorrentMetadata* metadata, const char* peer_id);
void torrent_tracker_result_print(TorrentTrackerResult* result);
void torrent_tracker_result_destroy(TorrentTrackerResult* result);
