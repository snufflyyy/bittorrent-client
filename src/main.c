#include <stdio.h>
#include <stdlib.h>

#include <metadata/metadata.h>

int main() {
	TorrentMetadata* torrent_metadata = torrent_metadata_create("resources/big-buck-bunny.torrent");
	if (!torrent_metadata) {
		fprintf(stderr, "[ERROR] Failed to create torrent metadata!\n");
		return -1;
	}

	printf("\n");
	torrent_metadata_print(torrent_metadata);

	torrnet_metadata_destroy(torrent_metadata);
}
