project(compressor)
cmake_minimum_required(VERSION 3.22)

string(TIMESTAMP TODAY UTC)
add_compile_definitions(BUILD_DATE=\"${TODAY}\")

add_executable(compressor
    compressor.cpp
    3rd_party/liblzg/src/lib/encode.c
    3rd_party/liblzg/src/lib/checksum.c)

target_include_directories(compressor PRIVATE
    3rd_party/liblzg/src/include)

target_link_libraries(compressor PUBLIC "-static")

add_custom_command(TARGET compressor
    POST_BUILD
    COMMAND upx compressor
    )

