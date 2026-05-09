#include "url.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

URLSplitResult url_split(const char* url) {
	URLSplitResult result = {0};

	char* host = strstr(url, "://");

	usize protocol_length = (host - url);
	if (protocol_length > (sizeof(result.protocol) / sizeof(char))) {
		fprintf(stderr, "[ERROR] [URL] Protocol is too long!\n");
		return (URLSplitResult) {0};
	}

	strncpy(result.protocol, url, protocol_length);
	result.protocol[protocol_length] = '\0';

	if (host) {
		host += 3; // for '://'
	} else {
		host = (char*) url;
	}
	char* host_end = host;
	while (*host_end != ':' && *host_end != '/' && *host_end != '\0') {
		host_end++;
	}

	usize host_length = (host_end - host);
	strncpy(result.host, host, host_length);
	result.host[host_length] = '\0';

	char* port = strchr(host, ':');
	if (port) {
		port += 1;

		char* port_end = port;
		while (*port_end != '/' && *port_end != '\0') {
			port_end++;
		}

		usize port_length = (port_end - port);
		strncpy(result.port, port, port_length);
		result.port[port_length] = '\0';
	}

	char* path = strchr(port, '/');
	if (path) {
		char* path_end = path;
		while (*path_end != '\0') {
			path_end++;
		}

		usize path_length = (path_end - path);
		strncpy(result.path, path, path_length);
		result.path[path_length] = '\0';
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
