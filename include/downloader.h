#pragma once

#include "metadata.h"
#include "peer.h"
#include "types.h"

typedef struct TorrentDownloader {
    char peer_id[20];
    TorrentMetadata* metadata;

    TorrentPeer** peers;
    usize peers_length;
} TorrentDownloader;

TorrentDownloader* torrent_downloader_create(const char* torrent_file);
void torrent_downloader_destroy(TorrentDownloader* downloader);
