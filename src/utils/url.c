#include "utils/url.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

URLSplitResult url_split(const char* url) {
	URLSplitResult result = {0};

	char* host = strstr(url, "://");
	if (host) {
		host += 3;
	} else {
		host = (char*) url;
	}
	char* host_end = host;
	while (*host_end != ':' && *host_end != '/' && *host_end != '\0') {
		host_end++;
	}

	strncpy(result.host, host, (host_end - host));

	char* port = strchr(host, ':');
	if (port) {
		port += 1;

		char* port_end = port;
		while (*port_end != '/' && *port_end != '\0') {
			port_end++;
		}

		strncpy(result.port, port, (port_end - port));
	} else {
		port = host;
	}

	char* path = strchr(port, '/');
	if (path) {
		char* path_end = path;
		while (*path_end != '\0') {
			path_end++;
		}

		strncpy(result.path, path, (path_end - path));
	}

	return result;
}

/* MUST BE FREED */
char* url_encode(u8* bytes, usize bytes_length) {
    usize string_length = (bytes_length * (sizeof(char) * 3));
    char* string = (char*) malloc(sizeof(char) * (string_length + 1));
    if (!string) {
        fprintf(stderr, "[ERROR] [URL] Failed to allocate memory for string!\n");
        return NULL;
    }

    char* position = string;
    for (usize i = 0; i < bytes_length; i++) {
        position += snprintf(position, string_length, "%%%02x", bytes[i]);
    }

    *position = '\0';
    return string;
}
