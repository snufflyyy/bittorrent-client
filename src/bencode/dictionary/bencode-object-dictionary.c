#include "bencode/dictionary/bencode-object-dictionary.h"
#include "bencode/bencode-object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BencodeObjectDictionary* bencode_object_dictionary_parse(u8* bencode_data, u32 bencode_data_length, usize* i) {
	BencodeObjectDictionary* bencode_dictionary = (BencodeObjectDictionary*) malloc(sizeof(BencodeObjectDictionary));
	if (!bencode_dictionary) {
		fprintf(stderr, "[ERROR] [Bencode] Failed to allocate memory for bencode object dictionary!\n");
		return NULL;
	}

	usize i_start = *i;

	*i += 1; // for 'd'

	BencodeObjectDictionaryKeyValuePair* elements = NULL;
	u32 elements_length = 0;

	while (bencode_data[*i] != 'e') {
		BencodeObjectDictionaryKeyValuePair* temp = (BencodeObjectDictionaryKeyValuePair*) realloc(elements, sizeof(BencodeObjectDictionaryKeyValuePair) * (elements_length + 1));
		if (!temp) {
			fprintf(stderr, "[ERROR] [Bencode] [Dictionary] Failed to reallocate memory for dictionary's elements!\n");
			free(bencode_dictionary);
			return NULL;
		}

		elements = temp;

		elements[elements_length].key = bencode_object_byte_string_parse(bencode_data, bencode_data_length, i);
		elements[elements_length].value = bencode_object_parse_recursive(bencode_data, bencode_data_length, i);
		elements_length++;
	}

	*i += 1; // for 'e'

	usize i_end = *i;

	u8 bencode_byte_string_bencode_data_length = i_end - i_start;
	u8* bencode_byte_string_bencode_data = (u8*) malloc(sizeof(u8) * bencode_byte_string_bencode_data_length);
	if (!bencode_byte_string_bencode_data) {
		fprintf(stderr, "[ERROR] [Bencode] [elements_length] Failed to allocate memory for dictionary's bencode data!\n");

		free(bencode_dictionary);
		return NULL;
	}

	memcpy(bencode_byte_string_bencode_data, bencode_data + i_start, i_end - i_start);

	*bencode_dictionary = (BencodeObjectDictionary) {
		.object = (BencodeObject) {
			.type = BENCODE_OBJECT_TYPE_DICTIONARY,
			.bencode_data = bencode_byte_string_bencode_data,
			.bencode_data_length = bencode_byte_string_bencode_data_length,
		},
		.elements = elements,
		.elements_length = elements_length
	};

	return bencode_dictionary;
}

void bencode_object_dictionary_print(BencodeObjectDictionary* dictionary) {
	printf("dictionary elements: \n");
	for (usize i = 0; i < dictionary->elements_length; i++) {
		bencode_object_byte_string_print(dictionary->elements[i].key);
		bencode_object_print(dictionary->elements[i].value);
	}
	printf("dictionary length: %u\n", dictionary->elements_length);
}

void bencode_object_dictionary_destroy(BencodeObjectDictionary* dictionary) {
	for (usize i = 0; i < dictionary->elements_length; i++) {
		bencode_object_byte_string_destroy(dictionary->elements[i].key);
		bencode_object_destroy(dictionary->elements[i].value);
	}
	free(dictionary->elements);
	free(dictionary);
}
