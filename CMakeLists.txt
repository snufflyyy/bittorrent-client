cmake_minimum_required(VERSION 3.15)
project(bittorrent-client C)

set(CMAKE_C_STANDARD 99)

# needed for zed
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

include_directories(include src)

add_executable(
	${PROJECT_NAME}
	src/main.c

	src/file/file.c

	src/bencode/bencode-object.c
	src/bencode/byte-string/bencode-object-byte-string.c
	src/bencode/integer/bencode-object-integer.c
	src/bencode/list/bencode-object-list.c
	src/bencode/dictionary/bencode-object-dictionary.c

	src/metadata/metadata.c
)

target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
