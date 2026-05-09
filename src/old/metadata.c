#include "metadata.h"

#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "file.h"
#include "bencode.h"

TorrentMetadata* torrent_metadata_create(const char* filename) {
    TorrentMetadata* metadata = (TorrentMetadata*) malloc(sizeof(TorrentMetadata));
	if (!metadata) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for torrent metadata!\n");
		goto error;
	}

    usize bencode_length = 0;
	u8* bencode = file_to_byte_array(filename, &bencode_length);
	if (!bencode) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to open torrent file: %s\n", filename);
		goto error;
	}

	usize bencode_string_index = 0;
	BencodeObject* bencoded_metadata = bencode_object_parse(bencode, bencode_length, &bencode_string_index);
	if (!bencoded_metadata) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to parse bencoded torrent metadata from file: %s\n", filename);
		goto error;
	}

	free((void*) bencode);
	bencode = NULL;

	BencodeObject* bencoded_announce = bencode_object_dictionary_get(bencoded_metadata, "announce");
	if (!bencoded_announce) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to get announce from bencode!\n");
		goto error;
	}

	metadata->announce = (char*) malloc(sizeof(char) * (bencoded_announce->string_length + 1)); // +1 for '\0'
	if (!metadata->announce) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for announce string!\n");
		goto error;
	}

	memcpy(metadata->announce, bencoded_announce->string, bencoded_announce->string_length);
	metadata->announce[bencoded_announce->string_length] = '\0';

	BencodeObject* bencoded_announce_list = bencode_object_dictionary_get(bencoded_metadata, "announce-list");
	if (bencoded_announce_list) {
		metadata->announce_list_length = bencoded_announce_list->list_length;
		metadata->announce_list = (TorrentMetadataAnnounceListTier*) malloc(sizeof(TorrentMetadataAnnounceListTier) * metadata->announce_list_length);
		if (!metadata->announce_list) {
			fprintf(stderr, "[ERROR] [ANNOUNCE LIST] Failed to allocate memory for announce list tiers!\n");
			goto error;
		}

		for (usize i = 0; i < metadata->announce_list_length; i++) {
			BencodeObject* bencoded_announce_list_element = bencoded_announce_list->list[i];

			metadata->announce_list[i].urls_length = bencoded_announce_list_element->list_length;
			metadata->announce_list[i].urls = (char**) malloc(sizeof(char*) * metadata->announce_list[i].urls_length);
			if (!metadata->announce_list[i].urls) {
				fprintf(stderr, "[ERROR] [ANNOUNCE LIST] Failed to allocate memory for announce list urls!\n");
				goto error;
			}

			for (usize j = 0; j < metadata->announce_list[i].urls_length; j++) {
				BencodeObject* bencoded_announce_list_element_string = bencoded_announce_list->list[i]->list[j];

				metadata->announce_list[i].urls[j] = (char*) malloc(sizeof(char) * bencoded_announce_list_element_string->string_length + 1);
				if (!metadata->announce_list[i].urls[j]) {
					fprintf(stderr, "[ERROR] [ANNOUNCE LIST] Failed to allocate memory for annount list url string!\n");
					goto error;
				}

				memcpy(metadata->announce_list[i].urls[j], bencoded_announce_list_element_string->string, bencoded_announce_list_element_string->string_length);
				metadata->announce_list[i].urls[j][bencoded_announce_list_element_string->string_length] = '\0';
			}
		}
	}

	BencodeObject* bencoded_comment = bencode_object_dictionary_get(bencoded_metadata, "comment");
	if (bencoded_comment) {
		metadata->comment = (char*) malloc(sizeof(char) * bencoded_comment->string_length);
		if (!metadata->comment) {
			fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for comment string!\n");
			goto error;
		}

		memcpy(metadata->comment, bencoded_comment->string, bencoded_comment->string_length);
	}

	BencodeObject* bencoded_creation_date = bencode_object_dictionary_get(bencoded_metadata, "creation date");
	if (bencoded_creation_date) {
		metadata->creation_date = bencoded_creation_date->number;
	}

	BencodeObject* bencoded_created_by = bencode_object_dictionary_get(bencoded_metadata, "created by");
	if (bencoded_created_by) {
		metadata->created_by = (char*) malloc(sizeof(char) * bencoded_created_by->string_length);
		if (!metadata->created_by) {
			fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for created by string!\n");
			goto error;
		}

		memcpy(metadata->created_by, bencoded_created_by->string, bencoded_created_by->string_length);
	}

	BencodeObject* bencoded_encoding = bencode_object_dictionary_get(bencoded_metadata, "encoding");
	if (bencoded_encoding) {
		metadata->encoding = (char*) malloc(sizeof(char) * bencoded_encoding->string_length);
		if (!metadata->encoding) {
			fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for created by string!\n");
			goto error;
		}

		memcpy(metadata->encoding, bencoded_encoding->string, bencoded_encoding->string_length);
	}

	BencodeObject* bencoded_info = bencode_object_dictionary_get(bencoded_metadata, "info");
	if (!bencoded_info) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to get info from bencode!\n");
		goto error;
	}

	SHA1(bencoded_info->bencode_data, bencoded_info->bencode_data_length, metadata->info_sha1);

	BencodeObject* bencoded_files = bencode_object_dictionary_get(bencoded_info, "files");
	if (!bencoded_files) {
		metadata->info.type = SINGLE_FILE;

		BencodeObject* bencoded_length = bencode_object_dictionary_get(bencoded_info, "length");
		metadata->info.length = bencoded_length->number;
	} else {
		metadata->info.type = MULTIPLE_FILES;

		metadata->info.files_length = bencoded_files->list_length;
		metadata->info.files = (TorrentMetadataInfoFile*) malloc(sizeof(TorrentMetadataInfoFile) * metadata->info.files_length);
		if (!metadata->info.files) {
			fprintf(stderr, "[ERROR] [FILES] Failed to allocate memory for files!\n");
			goto error;
		}

		for (usize i = 0; i < metadata->info.files_length; i++) {
			BencodeObject* bencoded_file = bencoded_files->list[i];
			TorrentMetadataInfoFile* file = &metadata->info.files[i];

			file->length = bencode_object_dictionary_get(bencoded_file, "length")->number;

			BencodeObject* bencoded_path = bencode_object_dictionary_get(bencoded_file, "path");
			file->path_length = bencoded_path->list_length;
			file->path = (char**) malloc(sizeof(char*) * file->path_length);
			if (!file->path) {
				fprintf(stderr, "[ERROR] [FILES] Failed to allocate memory for file path!\n");
				goto error;
			}

			for (usize j = 0; j < file->path_length; j++) {
				BencodeObject* bencoded_path_element = bencoded_path->list[j];

				file->path[j] = (char*) malloc(sizeof(char) * bencoded_path_element->string_length + 1);
				if (!file->path[j]) {
					fprintf(stderr, "[ERROR] [FILES] Failed to allocate memory for file path string!\n");
					goto error;
				}
				memcpy(file->path[j], bencoded_path_element->string, bencoded_path_element->string_length);
				file->path[j][bencoded_path_element->string_length] = '\0';
			}
		}
	}

	BencodeObject* bencoded_name = bencode_object_dictionary_get(bencoded_info, "name");
	BencodeObject* bencoded_piece_length = bencode_object_dictionary_get(bencoded_info, "piece length");
	BencodeObject* bencoded_pieces = bencode_object_dictionary_get(bencoded_info, "pieces");

	metadata->info.name = (char*) malloc(sizeof(char) * bencoded_name->string_length + 1);
	if (!metadata->announce) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for announce string!\n");
		goto error;
	}

	memcpy(metadata->info.name, bencoded_name->string, bencoded_name->string_length);
	metadata->info.name[bencoded_name->string_length] = '\0';

	metadata->info.piece_length = bencoded_piece_length->number;
	metadata->info.piece_count = (bencoded_pieces->string_length / 20);
	metadata->info.pieces = (u8**) malloc(sizeof(u8*) * metadata->info.piece_count);
	if (!metadata->info.pieces) {
		fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for piece strings!\n");
		goto error;
	}

	for (usize i = 0; i < metadata->info.piece_count; i++) {
		metadata->info.pieces[i] = (u8*) malloc(sizeof(u8) * 20);
		if (!metadata->info.pieces[i]) {
			fprintf(stderr, "[ERROR] [METADATA] Failed to allocate memory for piece string!\n");
			goto error;
		}

		memcpy(metadata->info.pieces[i], bencoded_pieces->string + (i * 20), 20);
	}

	bencode_object_destroy(bencoded_metadata);

	return metadata;

error:
	if (bencode) { free(bencode); }
	bencode_object_destroy(bencoded_metadata);
	torrent_metadata_destroy(metadata);
	return NULL;
}

void torrent_metadata_print(TorrentMetadata* metadata) {
	printf("announce: %s\n", metadata->announce);
	if (metadata->announce_list) {
		printf("announce list:\n");
		for (usize i = 0; i < metadata->announce_list_length; i++) {
			for (usize j = 0; j < metadata->announce_list[i].urls_length; j++) {
				printf("\t%s\n", metadata->announce_list[i].urls[j]);
			}
		}
	}

	if (metadata->comment) { printf("comment: %s\n", metadata->comment); }
	if (metadata->created_by) { printf("created by: %s\n", metadata->created_by); }
	if (metadata->creation_date) { printf("creation date: %u\n", metadata->creation_date); }
	if (metadata->encoding) { printf("encoding: %s\n", metadata->encoding); }

	printf("info:\n");
	printf("\ttype: ");
	switch (metadata->info.type) {
		case SINGLE_FILE: {
			printf("single file\n");
			printf("\tlength: %u\n", metadata->info.length);
		} break;
		case MULTIPLE_FILES: {
			printf("multiple files\n");
			printf("\tfiles:\n");
			for (usize i = 0; i < metadata->info.files_length; i++) {
				printf("\t\tlength: %u\n", metadata->info.files[i].length);
				printf("\t\tpath: ");
				for (usize j = 0; j < metadata->info.files[i].path_length; j++) {
					printf("%s", metadata->info.files[i].path[j]);
					if (j != metadata->info.files[i].path_length - 1) { printf("/"); }
				}
				printf("\n\n");
			}
		} break;
	}

	printf("\tname: %s\n", metadata->info.name);
	printf("\tpiece length: %lu\n", metadata->info.piece_length);
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
	if (metadata->announce_list) {
		for (usize i = 0; i < metadata->announce_list_length; i++) {
			for (usize j = 0; j < metadata->announce_list[i].urls_length; j++) {
				if (metadata->announce_list[i].urls[j]) { free(metadata->announce_list[i].urls[j]); }
				free(metadata->announce_list[i].urls);
			}
		}
		free(metadata->announce_list);
	}

	if (metadata->comment) { free(metadata->comment); }
	if (metadata->created_by) { free(metadata->created_by); }
	if (metadata->encoding) { free(metadata->encoding); }

	if (metadata->info.pieces) {
		for (usize i = 0; i < metadata->info.piece_count; i++) {
			if (metadata->info.pieces[i]) { free(metadata->info.pieces[i]); }
		}
		free(metadata->info.pieces);
	}
	if (metadata->info.name) { free(metadata->info.name); }
	if (metadata->info.files) {
		for (usize i = 0; i < metadata->info.files_length; i++) {
			for (usize j = 0; j < metadata->info.files[i].path_length; j++) {
				if (metadata->info.files[i].path[j]) { free(metadata->info.files[i].path[j]); }
			}
			free(metadata->info.files[i].path);
		}
		free(metadata->info.files);
	}

	free(metadata);
}
