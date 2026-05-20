#include "bencode/integer/bencode-object-integer.h"
#include "bencode/bencode-object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BencodeObjectInteger* bencode_object_integer_parse(u8* bencode_data, u32 bencode_data_length, usize* i) {
	BencodeObjectInteger* bencode_integer = (BencodeObjectInteger*) malloc(sizeof(BencodeObjectInteger));
	if (!bencode_integer) {
		fprintf(stderr, "[ERROR] [Bencode] Failed to allocate memory for bencode object integer!\n");
		return NULL;
	}

	usize i_start = *i;

	*i += 1; // for 'i'

	// find end of number
	usize number_length = 0;
	while (bencode_data[*i + number_length] != 'e') {
		number_length++;
	}

	char* integer_string = (char*) malloc(sizeof(char) * (number_length + 1));
	if (!integer_string) {
		fprintf(stderr, "[ERROR] [Bencode] [Integer] Failed to allocate memory for bencode integer!\n");
		free(bencode_integer);
		return NULL;
	}

	memcpy(integer_string, bencode_data + *i, number_length);
	integer_string[number_length] = '\0';

	char* end;
	i32 integer = strtol(integer_string, &end, 10);
	if (*end != '\0') {
		fprintf(stderr, "[ERROR] [Bencode] [Integer] Failed to convert integer string into integer!\n");
		free(integer_string);
		free(bencode_integer);
		return NULL;
	}

	free(integer_string);

	*i += number_length + 1;

	usize i_end = *i;

	usize bencode_integer_bencode_data_length = i_end - i_start;
	u8* bencode_integer_bencode_data = (u8*) malloc(sizeof(u8) * bencode_integer_bencode_data_length);
	if (!bencode_integer_bencode_data) {
		fprintf(stderr, "[ERROR] [Bencode] [Integer] Failed to allocate memory for integer bencode data!\n");
		free(bencode_integer);
		return NULL;
	}

	memcpy(bencode_integer_bencode_data, bencode_data + i_start, i_end - i_start);

	*bencode_integer = (BencodeObjectInteger) {
		.object = (BencodeObject) {
			.type = BENCODE_OBJECT_TYPE_INTEGER,
			.bencode_data = bencode_integer_bencode_data,
			.bencode_data_length = bencode_integer_bencode_data_length,
		},
		.integer = integer,
	};

	return bencode_integer;
}

void bencode_object_integer_print(BencodeObjectInteger* integer) {
	printf("%d", integer->integer);
}

void bencode_object_integer_destroy(BencodeObjectInteger* integer) {
	free(integer->object.bencode_data);
	free(integer);
}
