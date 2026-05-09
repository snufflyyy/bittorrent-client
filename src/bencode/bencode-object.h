#pragma once

#include "types.h"

typedef enum BencodeObjectType {
	BENCODE_OBJECT_TYPE_STRING,
	BENCODE_OBJECT_TYPE_INTEGER,
	BENCODE_OBJECT_TYPE_LIST,
	BENCODE_OBJECT_TYPE_DICTIONARY
} BencodeObjectType;

typedef struct BencodeObject {
	BencodeObjectType type;
	u8* bencode_data;
	usize bencode_data_length;
} BencodeObject;

BencodeObject* bencode_object_parse(u8* bencode_data, usize bencode_data_length);
BencodeObject* bencode_object_parse_recursive(u8* bencode_data, usize bencode_data_length, usize* i);
void bencode_object_print(BencodeObject* bencode_object);
void bencode_object_destroy(BencodeObject* bencode_object);
