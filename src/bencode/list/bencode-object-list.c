#include "bencode/list/bencode-object-list.h"
#include "bencode/bencode-object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BencodeObjectList* bencode_object_list_parse(u8* bencode_data, u32 bencode_data_length, usize* i) {
	BencodeObjectList* bencode_list = (BencodeObjectList*) malloc(sizeof(BencodeObjectList));
	if (!bencode_list) {
		fprintf(stderr, "[ERROR] [Bencode] Failed to allocate memory for bencode object list!\n");
		return NULL;
	}

	usize i_start = *i;

	*i += 1; // for 'l'

	BencodeObject** elements = NULL;
	u32 elements_length = 0;

	// realloc is sort of slow, maybe do this another way
	while (bencode_data[*i] != 'e') {
		BencodeObject** temp = (BencodeObject**) realloc(elements, sizeof(BencodeObject*) * (elements_length + 1));
		if (!temp) {
			fprintf(stderr, "[ERROR] [Bencode] [List] Failed to reallocate memory for list elements!\n");
			free(bencode_list);
			return NULL;
		}

		elements = temp;

		elements[elements_length] = bencode_object_parse_recursive(bencode_data, bencode_data_length, i);
		elements_length++;
	}

	*i += 1; // for 'e'

	usize i_end = *i;

	u8 bencode_byte_string_bencode_data_length = i_end - i_start;
	u8* bencode_byte_string_bencode_data = (u8*) malloc(sizeof(u8) * bencode_byte_string_bencode_data_length);
	if (!bencode_byte_string_bencode_data) {
		fprintf(stderr, "[ERROR] [Bencode] [Integer] Failed to allocate memory for integer bencode data!\n");

		free(bencode_list);
		return NULL;
	}

	memcpy(bencode_byte_string_bencode_data, bencode_data + i_start, i_end - i_start);

	*bencode_list = (BencodeObjectList) {
		.object = (BencodeObject) {
			.type = BENCODE_OBJECT_TYPE_LIST,
			.bencode_data = bencode_byte_string_bencode_data,
			.bencode_data_length = bencode_byte_string_bencode_data_length,
		},
		.elements = elements,
		.elements_length = elements_length
	};

	return bencode_list;
}

void bencode_object_list_print(BencodeObjectList* list) {
	printf("list elements: \n");
	for (usize i = 0; i < list->elements_length; i++) {
		bencode_object_print(list->elements[i]);
	}
	printf("list length: %u\n", list->elements_length);
}

void bencode_object_list_destroy(BencodeObjectList* list) {
	for (usize i = 0; i < list->elements_length; i++) {
		bencode_object_destroy(list->elements[i]);
	}
	free(list->elements);
	free(list);
}
