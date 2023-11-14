project(compressor)
cmake_minimum_required(VERSION 3.22)

string(TIMESTAMP TODAY UTC)
add_compile_definitions(BUILD_DATE=\"${TODAY}\")

add_executable(compressor
    compressor.cpp
    )
    
add_library(exomizer_lib STATIC
        3rd_party/exomizer-3.1.1/src/getflag.c
        3rd_party/exomizer-3.1.1/src/buf.c
        3rd_party/exomizer-3.1.1/src/log.c
        3rd_party/exomizer-3.1.1/src/match.c
        3rd_party/exomizer-3.1.1/src/search.c
        3rd_party/exomizer-3.1.1/src/exo_raw.c
        3rd_party/exomizer-3.1.1/src/optimal.c
        3rd_party/exomizer-3.1.1/src/output.c
        3rd_party/exomizer-3.1.1/src/buf_io.c
        3rd_party/exomizer-3.1.1/src/chunkpool.c
        3rd_party/exomizer-3.1.1/src/radix.c
        3rd_party/exomizer-3.1.1/src/exo_helper.c
        3rd_party/exomizer-3.1.1/src/exodec.c
        3rd_party/exomizer-3.1.1/src/progress.c
        3rd_party/exomizer-3.1.1/src/exo_util.c
        3rd_party/exomizer-3.1.1/src/vec.c)

target_include_directories(exomizer_lib PUBLIC
    3rd_party/exomizer-3.1.1/src)

target_link_libraries(compressor PUBLIC "-static")
target_link_libraries(compressor PUBLIC exomizer_lib)

add_custom_command(TARGET compressor
    POST_BUILD
    COMMAND upx compressor
)

# add test

add_library(exomizer_decompression_lib STATIC
    ../../source/exodecrunch.cpp
)

target_include_directories(exomizer_decompression_lib PUBLIC
    ../../source)




add_library(Catch2 STATIC
    3rd_party/catch2/catch_amalgamated.cpp)

target_include_directories(Catch2 PUBLIC
    3rd_party/catch2)

add_executable(compressor_test
    compressor_test.cpp
    exomizer_compression.cpp
    )


target_link_libraries(compressor_test 
    PUBLIC Catch2
    PUBLIC exomizer_lib
    PUBLIC exomizer_decompression_lib)
