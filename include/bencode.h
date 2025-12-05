#pragma once

#include "types.h"

typedef enum BencodeObjectType {
	STRING,
	INTEGER,
	LIST,
	DICTIONARY
} BencodeObjectType;

typedef struct BencodeObjectKeyValue {
	struct BencodeObject* key;
	struct BencodeObject* value;
} BencodeObjectKeyValue;

typedef struct BencodeObject {
	BencodeObjectType type;

	u8* string;
	usize string_length;
	i32 number;
	struct BencodeObject** list;
	usize list_length;
	BencodeObjectKeyValue* dictionary;
	usize dictionary_length;
} BencodeObject;

BencodeObject* bencode_object_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index);
BencodeObject* bencode_object_dictionary_get(BencodeObject* dictionary, const char* key);
const char* bencode_object_dictionary_export(BencodeObject* dictionary);
void bencode_object_print(BencodeObject* object);
void bencode_object_destroy(BencodeObject* object);
