#include "bencode/byte-string/bencode-object-byte-string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bencode/bencode-object.h"

BencodeObjectByteString* bencode_object_byte_string_parse(u8* bencode_data, u32 bencode_data_length, usize* i) {
	BencodeObjectByteString* bencode_byte_string = (BencodeObjectByteString*) malloc(sizeof(BencodeObjectByteString));
	if (!bencode_byte_string) {
		fprintf(stderr, "[ERROR] [Bencode] Failed to allocate memory for bencode object byte string!\n");
		return NULL;
	}

	usize i_start = *i;

	// find length of string size number
	usize string_length_string_length = 0;
	while (bencode_data[*i + string_length_string_length] != ':') {
		string_length_string_length++;
	}

	char* string_length_string = (char*) malloc(sizeof(char) * (string_length_string_length + 1)); // +1 for '\0'
	if (!string_length_string) {
		fprintf(stderr, "[ERROR] [Bencode] [String] Failed to allocate memory for bencode integer!\n");
		free(bencode_byte_string);
		return NULL;
	}

	memcpy(string_length_string, bencode_data + *i, string_length_string_length);
	string_length_string[string_length_string_length] = '\0';

	*i += string_length_string_length + 1; // + 1 for ':' in bencode data

	// convert string length string into integer
	char* end;
	usize string_length = strtol(string_length_string, &end, 10); // 10 is for number base 10
	if (*end != '\0') {
		fprintf(stderr, "[ERROR] [BENCODE] [STRING] Failed to convert string into integer!\n");
		free(string_length_string);
		free(bencode_byte_string);
		return NULL;
	}

	free(string_length_string);

	u8* byte_string = (u8*) malloc(sizeof(u8) * string_length);
	if (!byte_string) {
		fprintf(stderr, "[ERROR] [Bencode] [String] Failed to allocate memory for byte string!\n");
		free(bencode_byte_string);
		return NULL;
	}

	memcpy(byte_string, bencode_data + *i, string_length);
	*i += string_length;

	usize i_end = *i;

	u8 bencode_byte_string_bencode_data_length = i_end - i_start;
	u8* bencode_byte_string_bencode_data = (u8*) malloc(sizeof(u8) * bencode_byte_string_bencode_data_length);
	if (!bencode_byte_string_bencode_data) {
		fprintf(stderr, "[ERROR] [Bencode] [String] Failed to allocate memory for byte string bencode data!\n");
		free(byte_string);
		free(bencode_byte_string);
		return NULL;
	}

	memcpy(bencode_byte_string_bencode_data, bencode_data + i_start, i_end - i_start);

	*bencode_byte_string = (BencodeObjectByteString) {
		.object = (BencodeObject) {
			.type = BENCODE_OBJECT_TYPE_STRING,
			.bencode_data = bencode_byte_string_bencode_data,
			.bencode_data_length = bencode_byte_string_bencode_data_length,
		},
		.byte_string = byte_string,
		.byte_string_length = string_length,
	};

	return bencode_byte_string;
}

void bencode_object_byte_string_print(BencodeObjectByteString* byte_string) {
	printf("byte string: ");
	for (usize i = 0; i < byte_string->byte_string_length; i++) {
		printf("%c", byte_string->byte_string[i]);
	}
	printf("\n");
	printf("byte string length: %u\n", byte_string->byte_string_length);
}

void bencode_object_byte_string_destroy(BencodeObjectByteString* byte_string) {
	free(byte_string->byte_string);
	free(byte_string->object.bencode_data);
	free(byte_string);
}
