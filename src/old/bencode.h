#pragma once

#include "types.h"

typedef enum BencodeObjectType {
	STRING,
	INTEGER,
	LIST,
	DICTIONARY
} BencodeObjectType;

typedef struct BencodeObjectKeyValuePair {
	struct BencodeObject* key;
	struct BencodeObject* value;
} BencodeObjectKeyValuePair;

typedef struct BencodeObject {
	BencodeObjectType type;
	u8* bencode_data;
	usize bencode_data_length;

	u8* string;
	usize string_length;
	i32 number;
	struct BencodeObject** list;
	usize list_length;
	BencodeObjectKeyValuePair* dictionary;
	usize dictionary_length;
} BencodeObject;

BencodeObject* bencode_object_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index);
BencodeObject* bencode_object_dictionary_get(BencodeObject* dictionary, const char* key);
void bencode_object_print(BencodeObject* object);

BencodeObject* bencode_object_integer_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index);
BencodeObject* bencode_object_string_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index);
BencodeObject* bencode_object_list_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index);
BencodeObject* bencode_object_dictionary_parse(u8* bencoded_string, usize bencoded_string_length, usize* bencoded_string_index);

void bencode_object_integer_destroy(BencodeObject* object);
void bencode_object_string_destroy(BencodeObject* object);
void bencode_object_list_destroy(BencodeObject* object);
void bencode_object_dictionary_destroy(BencodeObject* object);

void bencode_object_destroy(BencodeObject* object);
