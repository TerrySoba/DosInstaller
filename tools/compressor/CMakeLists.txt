cmake_minimum_required(VERSION 3.22)

add_executable(compressor 
    compressor.cpp
    3rd_party/liblzg/src/lib/encode.c
    3rd_party/liblzg/src/lib/checksum.c)

target_include_directories(compressor PRIVATE
    3rd_party/liblzg/src/include)