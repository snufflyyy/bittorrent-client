#pragma once

#include "types.h"

typedef enum TorrentMetadataInfoType {
    SINGLE_FILE,
    MULTIPLE_FILES,
} TorrentMetadataInfoType;

typedef struct TorrentMetadataInfoFile {
    u32 length;
    char* path;
} TorrentMetadataInfoFile;

typedef struct TorrentMetadataInfo {
    TorrentMetadataInfoType type;

	u8** pieces;
	usize piece_length;
	u32 piece_count;

    char* name;
	u32 length;

	TorrentMetadataInfoFile* files;
	usize files_length;
} TorrentMetadataInfo;

typedef struct TorrentMetadata {
	char* announce;

	char** announce_list;
	usize announce_list_length;

	u32 creation_date;
	char* created_by;
	char* encoding;

	TorrentMetadataInfo info;
	u8 info_sha1[20];
} TorrentMetadata;

TorrentMetadata* torrent_metadata_create(const char* filename);
void torrent_metadata_print(TorrentMetadata* metadata);
void torrent_metadata_destroy(TorrentMetadata* metadata);
