cmake_minimum_required(VERSION 3.15)
project(FTP VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OPENSSL_INCLUDE_DIR "C:/Program Files/OpenSSL-Win64/include")
set(OPENSSL_CRYPTO_LIBRARY "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib")

add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
set_target_properties(OpenSSL::Crypto PROPERTIES
    IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
)

add_library(OpenSSL::SSL UNKNOWN IMPORTED)
set_target_properties(OpenSSL::SSL PROPERTIES
    IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES OpenSSL::Crypto
)

set(SRC
    src/main.cpp
    src/server.cpp
    src/client.cpp
    src/serverSession.cpp
    src/checksum.cpp
)

add_executable(FTP ${SRC})

target_include_directories(FTP PRIVATE include)


set(TLS_KEY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tls_key")

file(COPY "${TLS_KEY_DIR}/server.crt" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${TLS_KEY_DIR}/server.key" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

find_package(Threads REQUIRED)

target_link_libraries(FTP
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        $<$<PLATFORM_ID:Windows>:ws2_32 mswsock>
)

message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL SSL Lib: ${OPENSSL_SSL_LIBRARY}")
message(STATUS "OpenSSL Crypto Lib: ${OPENSSL_CRYPTO_LIBRARY}")