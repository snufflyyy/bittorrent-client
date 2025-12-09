#include "metadata.h"

#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "utils/file.h"
#include "bencode.h"

TorrentMetadata* torrent_metadata_create(const char* filename) {
    TorrentMetadata* metadata = (TorrentMetadata*) malloc(sizeof(TorrentMetadata));
	if (!metadata) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for torrent metadata!\n");
		return NULL;
	}

    usize bencode_length = 0;
	u8* bencode = file_to_byte_array(filename, &bencode_length);
	if (!bencode) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to open torrent file: %s\n", filename);
		free(metadata);
		return NULL;
	}

	usize bencode_string_index = 0;
	BencodeObject* bencoded_metadata = bencode_object_parse(bencode, bencode_length, &bencode_string_index);
	if (!bencoded_metadata) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to parse bencoded torrent metadata from file: %s\n", filename);
		free((void*) bencode);
		free(metadata);
		return NULL;
	}

	free((void*) bencode);

	BencodeObject* bencoded_announce = bencode_object_dictionary_get(bencoded_metadata, "announce");

	metadata->announce = (char*) malloc(sizeof(char) * (bencoded_announce->string_length + 1));
	if (!metadata->announce) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for announce string!\n");
		bencode_object_destroy(bencoded_metadata);
		free(metadata);
		return NULL;
	}

	memcpy(metadata->announce, bencoded_announce->string, bencoded_announce->string_length);
	metadata->announce[bencoded_announce->string_length] = '\0';

	BencodeObject* bencoded_info = bencode_object_dictionary_get(bencoded_metadata, "info");

	SHA1(bencoded_info->bencode_data, bencoded_info->bencode_data_length, metadata->info_sha1);

	BencodeObject* bencoded_name = bencode_object_dictionary_get(bencoded_info, "name");
	BencodeObject* bencoded_length = bencode_object_dictionary_get(bencoded_info, "length");
	BencodeObject* bencoded_piece_length = bencode_object_dictionary_get(bencoded_info, "piece length");
	BencodeObject* bencoded_pieces = bencode_object_dictionary_get(bencoded_info, "pieces");

	metadata->info.name = (char*) malloc(sizeof(char) * bencoded_name->string_length + 1);
	if (!metadata->announce) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for announce string!\n");
		free(metadata->announce);
		bencode_object_destroy(bencoded_metadata);
		free(metadata);
		return NULL;
	}

	memcpy(metadata->info.name, bencoded_name->string, bencoded_name->string_length);
	metadata->info.name[bencoded_name->string_length] = '\0';

	metadata->info.length = bencoded_length->number;
	metadata->info.piece_length = bencoded_piece_length->number;
	metadata->info.piece_count = (bencoded_pieces->string_length / 20);
	metadata->info.pieces = (u8**) malloc(sizeof(u8*) * metadata->info.piece_count);
	if (!metadata->info.pieces) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for piece strings!\n");
		free(metadata->info.name);
		free(metadata->announce);
		bencode_object_destroy(bencoded_metadata);
		free(metadata);
		return NULL;
	}

	for (usize i = 0; i < metadata->info.piece_count; i++) {
		metadata->info.pieces[i] = (u8*) malloc(sizeof(u8) * 20);
		if (!metadata->info.pieces[i]) {
			fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for piece string!\n");
			free(metadata->info.pieces);
			free(metadata->info.name);
			free(metadata->announce);
			bencode_object_destroy(bencoded_metadata);
			free(metadata);
		}

		memcpy(metadata->info.pieces[i], bencoded_pieces->string + (i * 20), 20);
	}

	bencode_object_destroy(bencoded_metadata);

	return metadata;
}

void torrent_metadata_print(TorrentMetadata* metadata) {
	printf("announce: %s\n", metadata->announce);
	printf("info:\n");
	printf("\tname: %s\n", metadata->info.name);
	printf("\tlength: %u\n", metadata->info.length);
	printf("\tpiece length: %u\n", metadata->info.piece_length);
	printf("\tpiece count: %u\n", metadata->info.piece_count);
	printf("\tpieces (%u out of %u shown):\n", metadata->info.piece_count / 2, metadata->info.piece_count);

	for (usize i = 0; i < metadata->info.piece_count / 2; i++) {
		printf("\t\t");
		for (usize j = 0; j < 20; j++) {
			printf("%02x ", metadata->info.pieces[i][j]);
		}
		printf("\n");
	}
}

void torrent_metadata_destroy(TorrentMetadata* metadata) {
	if (metadata->announce) { free(metadata->announce); }
	if (metadata->info.name) { free(metadata->info.name); }
	if (metadata->info.pieces) {
		for (usize i = 0; i < metadata->info.piece_count; i++) {
			if (metadata->info.pieces[i]) { free(metadata->info.pieces[i]); }
		}
		free(metadata->info.pieces);
	}
	free(metadata);
}
