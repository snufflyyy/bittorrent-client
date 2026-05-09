#include "bencode.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BencodeObject* bencode_object_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index) {
	usize start_index = *bencoded_string_index;

    BencodeObject* object = NULL;
	switch (bencoded_string[*bencoded_string_index]) {
		case 'i': object = bencode_object_integer_parse(bencoded_string, bencoded_string_length, bencoded_string_index); break;
		case 'l': object = bencode_object_list_parse(bencoded_string, bencoded_string_length, bencoded_string_index); break;
		case 'd': object = bencode_object_dictionary_parse(bencoded_string, bencoded_string_length, bencoded_string_index); break;
		default: object = bencode_object_string_parse(bencoded_string, bencoded_string_length, bencoded_string_index); break;
	}

	object->bencode_data_length = (*bencoded_string_index - start_index);
	object->bencode_data = (u8*) malloc(sizeof(u8) * object->bencode_data_length);
	memcpy(object->bencode_data, bencoded_string + start_index, object->bencode_data_length);

	return object;
}

BencodeObject* bencode_object_dictionary_get(BencodeObject* dictionary, const char* key) {
    for (usize i = 0; i < dictionary->dictionary_length; i++) {
		if (strncmp((char*) dictionary->dictionary[i].key->string, key, strlen(key)) == 0) {
			return dictionary->dictionary[i].value;
		}
	}

	return NULL;
}

void bencode_object_print(BencodeObject* object) {
	switch (object->type) {
		case STRING: {
			printf("\"");
			for (usize i = 0; i < object->string_length; i++) {
				if (object->string[i] >= 32 && object->string[i] <= 126) {
					printf("%c", object->string[i]);
				} else {
					printf("%x ", object->string[i]);
				}
			}
			printf("\"");
		} break;
		case INTEGER: printf("%d", object->number); break;
		case LIST: {
			printf("[");
			for (usize i = 0; i < object->list_length; i++) {
				printf(" ");
				bencode_object_print(object->list[i]);
				if (i != object->list_length - 1) { printf(","); }
				printf(" ");
			}
			printf("]");
		} break;
		case DICTIONARY: {
			printf("{");
			for (usize i = 0; i < object->dictionary_length; i++) {
				printf(" ");
				bencode_object_print(object->dictionary[i].key);
				printf(": ");
				bencode_object_print(object->dictionary[i].value);
				if (i != object->dictionary_length - 1) { printf(","); }
				printf(" ");
			}
			printf("}");
		} break;
	}
}

void bencode_object_destroy(BencodeObject* object) {
	free(object->bencode_data);

    switch (object->type) {
		case STRING: bencode_object_string_destroy(object); break;
		case INTEGER: bencode_object_integer_destroy(object); break;
		case LIST: bencode_object_list_destroy(object); break;
		case DICTIONARY: bencode_object_dictionary_destroy(object); break;
	}
}

BencodeObject* bencode_object_integer_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index) {
	BencodeObject* object = (BencodeObject*) malloc(sizeof(BencodeObject));
	if (!object) {
		fprintf(stderr, "[ERROR] [BENCODE] [INTEGER] Failed to allocate memory for bencode object!\n");
		return NULL;
	}

	*bencoded_string_index += 1; // for 'i'

	usize number_length = 0;
	while (bencoded_string[*bencoded_string_index + number_length] != 'e') {
		number_length++;
	}

	char* number_string = (char*) malloc(sizeof(char) * (number_length + 1)); // +1 for '\0'
	if (!number_string) {
		fprintf(stderr, "[ERROR] [BENCODE] [INTEGER] Failed to allocate memory for bencode integer!\n");
		free(object);
		return NULL;
	}

	memcpy(number_string, bencoded_string + *bencoded_string_index, number_length);
	number_string[number_length] = '\0';

	object->type = INTEGER;

	char* end;
	object->number = strtol(number_string, &end, 10);
	if (*end != '\0') {
		fprintf(stderr, "[ERROR] [BENCODE] [INTEGER] Failed to convert string into integer!\n");
		free(number_string);
		free(object);
		return NULL;
	}

	free(number_string);

	*bencoded_string_index += number_length + 1; // + 1 for 'e'

	return object;
}

BencodeObject* bencode_object_string_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index) {
	BencodeObject* object = (BencodeObject*) malloc(sizeof(BencodeObject));
	if (!object) {
		fprintf(stderr, "[ERROR] [BENCODE] [STRING] Failed to allocate memory for bencode object!\n");
		return NULL;
	}

	// find end of number
	usize number_length = 0;
	while (bencoded_string[*bencoded_string_index + number_length] != ':') {
		number_length++;
	}

	char* number_string = (char*) malloc(sizeof(char) * (number_length + 1)); // +1 for '\0'
	if (!number_string) {
		fprintf(stderr, "[ERROR] [BENCODE] [STRING] Failed to allocate memory for bencode integer!\n");
		free(object);
		return NULL;
	}

	memcpy(number_string, bencoded_string + *bencoded_string_index, number_length);
	number_string[number_length] = '\0';

	*bencoded_string_index += number_length + 1; // + 1 for ':'

	char* end;
	usize string_length = strtol(number_string, &end, 10);
	if (*end != '\0') {
		fprintf(stderr, "[ERROR] [BENCODE] [STRING] Failed to convert string into integer!\n");
		free(number_string);
		free(object);
		return NULL;
	}

	free(number_string);

	object->type = STRING;
	object->string_length = string_length;
	object->string = (u8*) malloc(sizeof(u8) * string_length);
	if (!object->string) {
		fprintf(stderr, "[ERROR] [BENCODE] [STRING] Failed to allocate memory for string!\n");
		free(object);
		return NULL;
	}

	memcpy(object->string, bencoded_string + *bencoded_string_index, string_length);
	*bencoded_string_index += string_length;

	return object;
}

BencodeObject* bencode_object_list_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index) {
	BencodeObject* object = (BencodeObject*) malloc(sizeof(BencodeObject));
	if (!object) {
		fprintf(stderr, "[ERROR] [BENCODE] [LIST] Failed to allocate memory for bencode object!\n");
		return NULL;
	}

	*bencoded_string_index += 1; // for 'l'

	object->type = LIST;
	object->list_length = 0;

	usize list_capacity = 1;
	object->list = (BencodeObject**) malloc(sizeof(BencodeObject*));
	if (!object->list) {
		fprintf(stderr, "[ERROR] [BENCODE] [LIST] Failed to allocate memory for list!\n");
		free(object);
		return NULL;
	}

	while (bencoded_string[*bencoded_string_index] != 'e') {
		if (list_capacity == object->list_length) {
			BencodeObject** temp = (BencodeObject**) realloc(object->list, sizeof(BencodeObject*) * (list_capacity * 2));
			if (!temp) {
				fprintf(stderr, "[ERROR] [BENCODE] [LIST] Failed to reallocate memory for list elements!\n");
				free(object->list);
				free(object);
				return NULL;
			}

			object->list = temp;
			list_capacity *= 2;
		}

		object->list[object->list_length] = bencode_object_parse(bencoded_string, bencoded_string_length, bencoded_string_index);
		object->list_length++;
	}

	// realloc to exact size of list
	BencodeObject** temp = (BencodeObject**) realloc(object->list, sizeof(BencodeObject*) * object->list_length);
	if (!temp) {
		fprintf(stderr, "[ERROR] [BENCODE] [LIST] Failed to reallocate memory for list!\n");
		free(object->list);
		free(object);
		return NULL;
	}

	object->list = temp;

	*bencoded_string_index += 1; // for 'e'

	return object;
}

// it might be worth to implement a hash table for this
BencodeObject* bencode_object_dictionary_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index) {
	BencodeObject* object = (BencodeObject*) malloc(sizeof(BencodeObject));
	if (!object) {
		fprintf(stderr, "[ERROR] [BENCODE] [DICTIONARY] Failed to allocate memory for bencode object!\n");
		return NULL;
	}

	*bencoded_string_index += 1;

	object->type = DICTIONARY;
	object->dictionary_length = 0;

	usize dictionary_capacity = 1;
	object->dictionary = (BencodeObjectKeyValuePair*) malloc(sizeof(BencodeObjectKeyValuePair));
	if (!object->dictionary) {
		fprintf(stderr, "[ERROR] [BENCODE] [DICTIONARY] Failed to allocate memory for dictionary!\n");
		free(object);
		return NULL;
	}

	while (bencoded_string[*bencoded_string_index] != 'e') {
		BencodeObjectKeyValuePair* temp = (BencodeObjectKeyValuePair*) realloc(object->dictionary, sizeof(BencodeObjectKeyValuePair) * (dictionary_capacity * 2));
		if (!temp) {
			fprintf(stderr, "[ERROR] [BENCODE] [DICTIONARY] Failed to reallocate memory for dictionary elements!\n");
			free(object->dictionary);
			free(object);
			return NULL;
		}

		object->dictionary = temp;
		dictionary_capacity *= 2;

		object->dictionary[object->dictionary_length].key = bencode_object_parse(bencoded_string, bencoded_string_length, bencoded_string_index);
		object->dictionary[object->dictionary_length].value = bencode_object_parse(bencoded_string, bencoded_string_length, bencoded_string_index);
		object->dictionary_length++;
	}

	// realloc to exact size of dictionary
	BencodeObjectKeyValuePair* temp = (BencodeObjectKeyValuePair*) realloc(object->dictionary, sizeof(BencodeObjectKeyValuePair) * object->list_length);
	if (!temp) {
		fprintf(stderr, "[ERROR] [BENCODE] [DICTIONARY] Failed to reallocate memory for dictionary!\n");
		free(object->list);
		free(object);
		return NULL;
	}

	object->dictionary = temp;

	*bencoded_string_index += 1; // for 'e'

	return object;
}

inline void bencode_object_integer_destroy(BencodeObject* object) {
	if (!object) { return; }
}

inline void bencode_object_string_destroy(BencodeObject* object) {
	if (!object) { return; }

	if (object->string) { free(object->string); }
	free(object);
}

void bencode_object_list_destroy(BencodeObject* object) {
	if (!object) { return; }

	if (object->list) {
		for (usize i = 0; i < object->list_length; i++) {
			if (object->list[i]) { bencode_object_destroy(object->list[i]); }
		}
		free(object->list);
	}
	free(object);
}

void bencode_object_dictionary_destroy(BencodeObject* object) {
	if (!object) { return; }

	if (object->dictionary) {

		for (usize i = 0; i < object->dictionary_length; i++) {
			if (object->dictionary[i].key) { bencode_object_destroy(object->dictionary[i].key); }
			if (object->dictionary[i].value) { bencode_object_destroy(object->dictionary[i].value); }
		}
		free(object->dictionary);
	}
	free(object);
}
