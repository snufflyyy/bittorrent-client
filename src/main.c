#include <stdio.h>
#include <stdlib.h>

#include "file/file.h"
#include "bencode/bencode-object.h"

int main() {
	u32 bencode_data_length = 0;
	u8* bencode_data = file_to_byte_array("resources/example.torrent", &bencode_data_length);
	if (!bencode_data) {
		fprintf(stderr, "[ERROR] Failed to get bencode data from file!\n");
		return -1;
	}

	BencodeObject* object = bencode_object_parse(bencode_data, bencode_data_length);
	if (!object) {
		fprintf(stderr, "[ERROR] Failed to parse bencode!\n");
		free(bencode_data);
		return -1;
	}

	bencode_object_print(object);

	bencode_object_destroy(object);
	free(bencode_data);
	return 0;
}
