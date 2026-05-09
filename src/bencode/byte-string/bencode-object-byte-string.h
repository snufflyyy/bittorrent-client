#pragma once

#include "bencode/bencode-object.h"

typedef struct BencodeObjectByteString {
	BencodeObject object;

	u8* byte_string;
	u32 byte_string_length;
} BencodeObjectByteString;

BencodeObjectByteString* bencode_object_byte_string_parse(u8* bencode_data, u32 bencode_data_length, usize* i);
void bencode_object_byte_string_print(BencodeObjectByteString* byte_string);
void bencode_object_byte_string_destroy(BencodeObjectByteString* byte_string);
