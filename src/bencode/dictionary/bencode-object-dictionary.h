#pragma once

#include "bencode/bencode-object.h"
#include "bencode/byte-string/bencode-object-byte-string.h"

typedef struct BencodeObjectDictionaryKeyValuePair {
	struct BencodeObjectByteString* key;
	struct BencodeObject* value;
} BencodeObjectDictionaryKeyValuePair;

typedef struct BencodeObjectDictionary {
	BencodeObject object;

	BencodeObjectDictionaryKeyValuePair* elements;
	u32 elements_length;
} BencodeObjectDictionary;

BencodeObjectDictionary* bencode_object_dictionary_parse(u8* bencode_data, u32 bencode_data_length, usize* i);
BencodeObject* bencode_object_dictionary_get(BencodeObjectDictionary* dictionary, const char* key);
void bencode_object_dictionary_print(BencodeObjectDictionary* dictionary);
void bencode_object_dictionary_destroy(BencodeObjectDictionary* dictionary);
