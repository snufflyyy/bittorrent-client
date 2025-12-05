#include "utils/file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

u8* file_to_byte_array(const char* path, usize* length) {
	FILE* file = fopen(path, "rb");
  	if (!file) {
  		fprintf(stderr, "[ERROR] [FILE] Failed to open file when converting file to byte array!\n");
   		*length = 0;
    	return NULL;
   	}

   	fseek(file, 0, SEEK_END);
    *length = ftell(file);
    rewind(file);

    u8* array = (u8*) malloc(sizeof(u8) * (*length));
    if (!array) {
    	fprintf(stderr, "[ERROR] [FILE] Failed to allocate memory for array when converting file to byte array!\n");
     	fclose(file);
      	*length = 0;
     	return NULL;
    }

    u32 bytes_read = fread(array, 1, *length, file);
    if (bytes_read != *length) {
    	fprintf(stderr, "[ERROR] [FILE} Failed to read entire file to array when converting file to byte array!\n");
     	free(array);
     	fclose(file);
      *length = 0;
      	return NULL;
    }

    fclose(file);
    return array;
}
