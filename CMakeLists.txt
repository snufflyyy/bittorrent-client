cmake_minimum_required(VERSION 3.15)
project(bittorrent-client C)

set(CMAKE_C_STANDARD 99)

# needed for zed
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

include_directories(include)

add_executable(
	${PROJECT_NAME}
	src/main.c

	src/utils/file.c
	src/utils/url.c

	src/metadata.c
	src/bencode.c
)

target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto OpenSSL::SSL)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
