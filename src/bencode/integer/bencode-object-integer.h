#pragma once

#include "bencode/bencode-object.h"

typedef struct BencodeObjectInteger {
	BencodeObject object;

	i32 integer;
} BencodeObjectInteger;

BencodeObjectInteger* bencode_object_integer_parse(u8* bencode_data, u32 bencode_data_length, usize* i);
void bencode_object_integer_print(BencodeObjectInteger* integer);
void bencode_object_integer_destroy(BencodeObjectInteger* integer);
