#pragma once

#include "types.h"

typedef struct URLSplitResult {
	char host[256];
	char port[16];
	char path[256];
} URLSplitResult;

URLSplitResult url_split(const char* url);
char* url_encode(u8* bytes, usize bytes_length);
