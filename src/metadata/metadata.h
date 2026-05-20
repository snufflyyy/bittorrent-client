#pragma once

#include <openssl/sha.h>

#include "types.h"

typedef enum TorrentMetadataInfoType {
	TORRENT_METADATA_TYPE_SINGLE,
	TORRENT_METADATA_TYPE_MULTI
} TorrentMetadataInfoType;

typedef struct TorrentMetadataInfoSingle {
	char* file_name;
	u32 file_length;
} TorrentMetadataInfoSingle;

typedef struct TorrentMetadataInfoMultiFile {
	char** path;
	u32 length;
} TorrentMetadataInfoMultiFile;

typedef struct TorrentMetadataInfoMulti {
	char* root_folder_name;

	TorrentMetadataInfoMultiFile* files;
	usize files_length;
} TorrentMetadataInfoMulti;

typedef struct TorrentMetadataInfo {
	u32 piece_length;

	u8* pieces;
	usize pieces_length;

	TorrentMetadataInfoType type;
	TorrentMetadataInfoSingle single;
	TorrentMetadataInfoMulti multi;
} TorrentMetadataInfo;

typedef struct TorrentMetadata {
	char* announce;

	i64 creation_date;

	char* comment;
	char* created_by;

	TorrentMetadataInfo info;
	u8 info_sha1[SHA_DIGEST_LENGTH];
} TorrentMetadata;

TorrentMetadata* torrent_metadata_create(const char* metadata_file);
void torrent_metadata_print(TorrentMetadata* metadata);
void torrnet_metadata_destroy(TorrentMetadata* metadata);
