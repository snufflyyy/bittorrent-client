#include "bencode-object.h"

#include <stdio.h>
#include <stdlib.h>

#include "bencode/byte-string/bencode-object-byte-string.h"
#include "bencode/integer/bencode-object-integer.h"
#include "bencode/list/bencode-object-list.h"
#include "bencode/dictionary/bencode-object-dictionary.h"

BencodeObject* bencode_object_parse(u8* bencode_data, usize bencode_data_length) {
	usize i = 0;
	return bencode_object_parse_recursive(bencode_data, bencode_data_length, &i);
}

BencodeObject* bencode_object_parse_recursive(u8* bencode_data, usize bencode_data_length, usize* i) {
	BencodeObject* object = NULL;
	switch (bencode_data[*i]) {
		default: object = (BencodeObject*) bencode_object_byte_string_parse(bencode_data, bencode_data_length, i); break; // string
		case 'i': object = (BencodeObject*) bencode_object_integer_parse(bencode_data, bencode_data_length, i); break; // integer
		case 'l': object = (BencodeObject*) bencode_object_list_parse(bencode_data, bencode_data_length, i); break; // list
		case 'd': object = (BencodeObject*) bencode_object_dictionary_parse(bencode_data, bencode_data_length, i); break; // dictionary
	}

	return object;
}

void bencode_object_print(BencodeObject* bencode_object) {
	switch (bencode_object->type) {
		case BENCODE_OBJECT_TYPE_STRING: bencode_object_byte_string_print((BencodeObjectByteString*) bencode_object); break;
		case BENCODE_OBJECT_TYPE_INTEGER: bencode_object_integer_print((BencodeObjectInteger*) bencode_object); break;
		case BENCODE_OBJECT_TYPE_LIST: bencode_object_list_print((BencodeObjectList*) bencode_object); break;
		case BENCODE_OBJECT_TYPE_DICTIONARY: bencode_object_dictionary_print((BencodeObjectDictionary*) bencode_object); break;
	}
}

void bencode_object_destroy(BencodeObject* bencode_object) {
	switch (bencode_object->type) {
		case BENCODE_OBJECT_TYPE_STRING: bencode_object_byte_string_destroy((BencodeObjectByteString*) bencode_object); break;
		case BENCODE_OBJECT_TYPE_INTEGER: bencode_object_integer_destroy((BencodeObjectInteger*) bencode_object); break;
		case BENCODE_OBJECT_TYPE_LIST: bencode_object_list_destroy((BencodeObjectList*) bencode_object); break;
		case BENCODE_OBJECT_TYPE_DICTIONARY: bencode_object_dictionary_destroy((BencodeObjectDictionary*) bencode_object); break;
	}
}
