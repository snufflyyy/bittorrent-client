#include "downloader.h"

#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "metadata.h"
#include "peer.h"
#include "tracker.h"
#include "types.h"

TorrentDownloader* torrent_downloader_create(const char* torrent_file) {
    TorrentDownloader* downloader = (TorrentDownloader*) malloc(sizeof(TorrentDownloader));
    if (!downloader) {
        fprintf(stderr, "[ERROR] [DOWNLOADER] Failed to allocate memory for downloader!\n");
        return NULL;
    }

    srand(time(NULL));
    for (usize i = 0; i < sizeof(downloader->peer_id); i++) {
        downloader->peer_id[i] = (rand() % 26) + 97;
    }

    downloader->metadata = torrent_metadata_create(torrent_file);
    if (!downloader->metadata) {
        fprintf(stderr, "[ERROR] [DOWNLOADER] Failed to create torrent metadata from torrent file: %s\n", torrent_file);
        free(downloader);
        return NULL;
    }

    TorrentTrackerResult tracker_result = torrent_tracker_get(downloader->metadata, downloader->peer_id);
    if (tracker_result.failed) {
        fprintf(stderr, "[ERROR] [DOWNLOADER] Failed to get result from tracker!\n");
        torrent_metadata_destroy(downloader->metadata);
        free(downloader);
        return NULL;
    }

    downloader->peers_length = tracker_result.peers_length;
    downloader->peers = (TorrentPeer**) malloc(sizeof(TorrentPeer*) * downloader->peers_length);
    if (!downloader->peers) {
        fprintf(stderr, "[ERROR] [DOWNLOADER] Failed to allocate memory for peers!\n");
        torrent_tracker_result_destroy(&tracker_result);
        torrent_metadata_destroy(downloader->metadata);
        free(downloader);
        return NULL;
    }

    for (usize i = 0; i < downloader->peers_length; i++) {
        downloader->peers[i] = torrent_peer_connect(tracker_result.peers[i].ip, tracker_result.peers[i].port, downloader->peer_id, downloader->metadata);
        if (!downloader->peers[i]) {
            fprintf(stderr, "[ERROR] [DOWNLOADER] Failed to connect to peer (index: %lu)!\n", i);
        }
    }

    return downloader;
}

void torrent_downloader_destroy(TorrentDownloader* downloader) {
    if (downloader->metadata) { torrent_metadata_destroy(downloader->metadata); }
    for (usize i = 0; i < downloader->peers_length; i++) {
        if (downloader->peers[i]) { free(downloader->peers[i]); }
    }
    if (downloader->peers) { free(downloader->peers); }
    free(downloader);
}
