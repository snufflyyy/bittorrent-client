#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "metadata.h"
#include "utils/url.h"

int main() {
	TorrentMetadata* metadata = torrent_metadata_create("../debian-13.2.0-amd64-DVD-1.iso.torrent");

	URLSplitResult url = url_split(metadata->announce);

 	struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC; // allows for either IPv4 or IPv6, it doesnt matter to us
    hints.ai_socktype = SOCK_STREAM; // use TCP

    struct addrinfo* address_info;
    i32 result;
    if ((result = getaddrinfo(url.host, url.port, &hints, &address_info)) != 0) {
        fprintf(stderr, "[ERROR] [NETWORK] Failed to get address infomation: %s\n", gai_strerror(result));

        torrent_metadata_destroy(metadata);
    }

    i32 s = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    connect(s, address_info->ai_addr, address_info->ai_addrlen);

    send(s, "adsfkl;jasdfljk;", 16, 0);

    char buffer[512];
    recv(s, buffer, 512, 0);

    printf("%s\n", buffer);

	torrent_metadata_destroy(metadata);
}
