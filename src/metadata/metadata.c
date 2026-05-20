#include "metadata/metadata.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "file/file.h"
#include "bencode/bencode-object.h"
#include "bencode/byte-string/bencode-object-byte-string.h"
#include "bencode/integer/bencode-object-integer.h"
#include "bencode/list/bencode-object-list.h"
#include "bencode/dictionary/bencode-object-dictionary.h"

TorrentMetadata* torrent_metadata_create(const char* metadata_file) {
	u32 bencode_data_length = 0;
	u8* bencode_data = file_to_byte_array(metadata_file, &bencode_data_length);
	if (!bencode_data) {
		fprintf(stderr, "[ERROR] [Metadata] Failed to convert torrent metadata into byte array!\n");
		goto error;
	}

	BencodeObject* metadata_bencode_object = bencode_object_parse(bencode_data, bencode_data_length);
	if (!metadata_bencode_object) {
		fprintf(stderr, "[ERROR] [Metadata] Failed to parse bencode!\n");
		goto error;
	}

	TorrentMetadata* metadata = (TorrentMetadata*) malloc(sizeof(TorrentMetadata));
	if (!metadata) {
		fprintf(stderr, "[ERROR] [Metadata] Failed to allocate memory for torrent metadata!\n");
		goto error;
	}

	BencodeObjectByteString* announce_bencode_object = (BencodeObjectByteString*) bencode_object_dictionary_get((BencodeObjectDictionary*) metadata_bencode_object, "announce");
	if (!announce_bencode_object) {
		fprintf(stderr, "[ERROR] [Metadata] Failed to get announce bencode object!\n");
		goto error;
	}

	metadata->announce = (char*) malloc(sizeof(char) * announce_bencode_object->byte_string_length);
	if (!metadata->announce) {
		fprintf(stderr, "[ERROR] [Metadata] Failed to allocate memory for announce string!\n");
		goto error;
	}

	memcpy(metadata->announce, announce_bencode_object->byte_string, announce_bencode_object->byte_string_length);

	BencodeObjectInteger* creation_date_bencode_object = (BencodeObjectInteger*) bencode_object_dictionary_get((BencodeObjectDictionary*) metadata_bencode_object, "creation date");
	if (announce_bencode_object) {
		metadata->creation_date = creation_date_bencode_object->integer;
	}

	BencodeObjectByteString* comment_bencode_object = (BencodeObjectByteString*) bencode_object_dictionary_get((BencodeObjectDictionary*) metadata_bencode_object, "comment");
	if (comment_bencode_object) {
		metadata->comment = (char*) malloc(sizeof(char) * comment_bencode_object->byte_string_length);
		if (!metadata->comment) {
			fprintf(stderr, "[ERROR] [Metadata] Failed to allocate memory for comment string!\n");
			goto error;
		}

		memcpy(metadata->comment, comment_bencode_object->byte_string, comment_bencode_object->byte_string_length);
	}

	BencodeObjectByteString* created_by_bencode_object = (BencodeObjectByteString*) bencode_object_dictionary_get((BencodeObjectDictionary*) metadata_bencode_object, "created by");
	if (created_by_bencode_object) {
		metadata->created_by = (char*) malloc(sizeof(char) * created_by_bencode_object->byte_string_length);
		if (!metadata->created_by) {
			fprintf(stderr, "[ERROR] [Metadata] Failed to allocate memory for created by string!\n");
			goto error;
		}

		memcpy(metadata->created_by, created_by_bencode_object->byte_string, created_by_bencode_object->byte_string_length);
	}

	// info

	BencodeObjectDictionary* info_bencode_object = (BencodeObjectDictionary*) bencode_object_dictionary_get((BencodeObjectDictionary*) metadata_bencode_object, "info");
	if (!info_bencode_object) {
		fprintf(stderr, "[ERROR] [Metadata] [Info] Failed to get info bencode object!\n");
		goto error;
	}

	BencodeObjectInteger* piece_length_bencode_object = (BencodeObjectInteger*) bencode_object_dictionary_get(info_bencode_object, "piece length");
	if (!piece_length_bencode_object) {
		fprintf(stderr, "[ERROR] [Metadata] [Info] Failed to get piece length bencode object!\n");
		goto error;
	}
	metadata->info.piece_length = piece_length_bencode_object->integer;

	BencodeObjectByteString* pieces_bencode_object = (BencodeObjectByteString*) bencode_object_dictionary_get(info_bencode_object, "pieces");
	if (!pieces_bencode_object) {
		fprintf(stderr, "[ERROR] [Metadata] [Info] Failed to get pieces bencode object!\n");
		goto error;
	}

	metadata->info.pieces = (u8*) malloc(sizeof(u8) * pieces_bencode_object->byte_string_length);
	if (!metadata->info.pieces) {
		fprintf(stderr, "[ERROR] [Metadata] [Info] Failed to allocate memory for pieces string!\n");
		goto error;
	}

	memcpy(metadata->info.pieces, pieces_bencode_object->byte_string, pieces_bencode_object->byte_string_length);

	metadata->info.pieces_length = pieces_bencode_object->byte_string_length;

	BencodeObjectList* files_bencode_object = (BencodeObjectList*) bencode_object_dictionary_get(info_bencode_object, "files");
	if (!files_bencode_object) {
		metadata->info.type = TORRENT_METADATA_TYPE_SINGLE;
	} else {
		metadata->info.type = TORRENT_METADATA_TYPE_MULTI;
	}

	switch (metadata->info.type) {
		case TORRENT_METADATA_TYPE_SINGLE: {
			BencodeObjectByteString* name_bencode_object = (BencodeObjectByteString*) bencode_object_dictionary_get(info_bencode_object, "name");
			if (!name_bencode_object) {
				fprintf(stderr, "[ERROR] [Metadata] [Info] [Single] Failed to get name bencode object!\n");
				goto error;
			}

			metadata->info.single.file_name = (char*) malloc(sizeof(char) * name_bencode_object->byte_string_length);
			if (!metadata->info.pieces) {
				fprintf(stderr, "[ERROR] [Metadata] [Info] [Single] Failed to allocate memory for file name string!\n");
				goto error;
			}

			memcpy(metadata->info.single.file_name, name_bencode_object->byte_string, name_bencode_object->byte_string_length);

			BencodeObjectInteger* length_bencode_object = (BencodeObjectInteger*) bencode_object_dictionary_get(info_bencode_object, "length");
			if (!length_bencode_object) {
				fprintf(stderr, "[ERROR] [Metadata] [Info] [Single] Failed to get length bencode object!\n");
				goto error;
			}
			metadata->info.single.file_length = length_bencode_object->integer;
		} break;
		case TORRENT_METADATA_TYPE_MULTI: {
			metadata->info.multi.files_length = files_bencode_object->elements_length;

			metadata->info.multi.files = (TorrentMetadataInfoMultiFile*) malloc(sizeof(TorrentMetadataInfoMulti) * metadata->info.multi.files_length);
			if (!metadata->info.multi.files) {
				fprintf(stderr, "[ERROR] [Metadata] [Info] [Multi] Failed to allocate memory for files!\n");
				goto error;
			}

			for (usize i = 0; i < metadata->info.multi.files_length; i++) {

			}
		} break;
	}

	bencode_object_destroy(metadata_bencode_object);
	free(bencode_data);

	return metadata;

error:
	if (metadata->info.multi.files) { free(metadata->info.multi.files); }
	if (metadata->info.single.file_name) { free(metadata->info.pieces); }
	if (metadata->info.pieces) { free(metadata->info.pieces); }
	if (metadata->created_by) { free(metadata->created_by); }
	if (metadata->comment) { free(metadata->comment); }
	if (metadata->announce) { free(metadata->announce); }
	if (metadata) { free(metadata); }
	if (metadata_bencode_object) { free(metadata_bencode_object); }
	if (bencode_data) { free(bencode_data); }
	return NULL;
}

void torrent_metadata_print(TorrentMetadata* metadata) {
	printf("announce: %s\n", metadata->announce);

	if (metadata->creation_date) {
		time_t epoch_time = metadata->creation_date;
		struct tm* epoch_local_time = localtime(&epoch_time);
	 	char creation_date_time_string_buffer[128];
	    strftime(creation_date_time_string_buffer, sizeof(creation_date_time_string_buffer), "%a %Y-%m-%d %H:%M:%S %Z", epoch_local_time);
		printf("creation date: %lld (%s)\n", metadata->creation_date, creation_date_time_string_buffer);
	}

	if (metadata->comment) { printf("comment: %s\n", metadata->comment); }
	if (metadata->created_by) { printf("created by: %s\n", metadata->created_by); }

	printf("info: {\n");
	printf("\tpiece length: %u\n", metadata->info.piece_length);
	printf("\tpieces:");

	const usize pieces_print_length = 20;
	for (usize i = 0; i < pieces_print_length; i++) {
		if (i < pieces_print_length) { printf(" "); }
		printf("0x%0.2X", metadata->info.pieces[i]);
	}
	printf(" (%lu more)\n", (metadata->info.pieces_length - pieces_print_length));
	printf("\tpieces length: %lu\n", metadata->info.pieces_length);
	printf("\ttype: %s\n", (metadata->info.type) ? "multi" : "single");
	switch (metadata->info.type) {
		case TORRENT_METADATA_TYPE_SINGLE: {
			printf("\t\tfile name: %s\n", metadata->info.single.file_name);
			printf("\t\tfile length: %u\n", metadata->info.single.file_length);
		} break;
		case TORRENT_METADATA_TYPE_MULTI: {
			printf("\t\troot folder name: %s\n", metadata->info.multi.root_folder_name);
			printf("\t\tfiles: [\n");
			for (usize i = 0; i < metadata->info.multi.files_length; i++) {
				printf("\t\t\t{\n");
				printf("\t\t\t}\n");
			}
			printf("\t\t]\n");
		} break;
	}
	printf("}\n");
}

void torrnet_metadata_destroy(TorrentMetadata* metadata) {
	switch (metadata->info.type) {
		case TORRENT_METADATA_TYPE_SINGLE: {
			free(metadata->info.single.file_name);
		} break;
		case TORRENT_METADATA_TYPE_MULTI: {
			free(metadata->info.multi.files);
		} break;
	}
	free(metadata->info.pieces);

	if (metadata->created_by) { free(metadata->created_by); }
	if (metadata->comment) { free(metadata->comment); }
	free(metadata->announce);
	free(metadata);
}
