#include <stdio.h>

#include "downloader.h"

int main() {
    TorrentDownloader* downloader = torrent_downloader_create("../debian-13.2.0-amd64-DVD-1.iso.torrent");
    if (!downloader) {
        fprintf(stderr, "[ERROR] Failed to create torrent downloader!\n");
        return -1;
    }

    torrent_downloader_destroy(downloader);
}
