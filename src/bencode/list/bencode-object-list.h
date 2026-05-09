#pragma once

#include "bencode/bencode-object.h"

typedef struct BencodeObjectList {
	BencodeObject object;

	BencodeObject** elements;
	u32 elements_length;
} BencodeObjectList;

BencodeObjectList* bencode_object_list_parse(u8* bencode_data, u32 bencode_data_length, usize* i);
void bencode_object_list_print(BencodeObjectList* list);
void bencode_object_list_destroy(BencodeObjectList* list);
