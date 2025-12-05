#pragma once

#include "types.h"

// https://www.bittorrent.org/beps/bep_0003.html
typedef struct TorrentMetadataInfo {
	char* name;
	u32 length;

	u32 piece_length;
	u32 piece_count;
	u8** pieces;
} TorrentMetadataInfo;

typedef struct TorrentMetadata {
	char* announce;
	TorrentMetadataInfo info;
	char info_sha1[20];
} TorrentMetadata;

TorrentMetadata* torrent_metadata_create(const char* filename);
void torrent_metadata_print(TorrentMetadata* metadata);
void torrent_metadata_destroy(TorrentMetadata* metadata);
