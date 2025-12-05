#pragma once

typedef struct URLSplitResult {
	char host[256];
	char port[16];
	char path[256];
} URLSplitResult;

URLSplitResult url_split(const char* url);
