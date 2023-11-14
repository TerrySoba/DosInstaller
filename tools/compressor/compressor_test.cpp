#include "catch_amalgamated.hpp"

#include "exomizer_compression.h"
#include "exodecrunch.h"


#include <iostream>
#include <cstdio>

struct ByteReader
{
public:
    ByteReader(const char* data, size_t size) : m_data((const unsigned char*)data), m_pos(0), m_size(size)
    {
    }

    int readByte()
    {
        if (m_pos >= m_size)
        {
            return EOF;
        }
        return m_data[m_pos++];
    }

private:
    const unsigned char* m_data;
    size_t m_pos;
    size_t m_size;
};

static int read_byte(void* byteReader)
{
    return ((ByteReader*)byteReader)->readByte();
}

int uncompress(const char* compressed, size_t compressedSize, char* uncompressed, size_t uncompressedSize)
{
    ByteReader reader(compressed, compressedSize);
    exo_decrunch_ctx* context = exo_decrunch_new(MAX_OFFSET, read_byte, &reader);

    int c;
    int uncompressedBytes = 0; 

    while((c = exo_read_decrunched_byte(context)) != EOF)
    {
        if ((size_t)uncompressedBytes >= uncompressedSize)
        {
            printf("Uncompressed buffer is too small.\n");
            uncompressedBytes = 0;
            break;
        }
        uncompressed[uncompressedBytes++] = c;
    }

    exo_decrunch_delete(context);

    return uncompressedBytes;
}


TEST_CASE("lala")
{

    std::vector<unsigned char> data = { 0x00, 0x01, 0x02, 0x03 };

    for (int i = 0; i < 5; ++i)
    {
        data.insert(data.end(), data.begin(), data.end());
    }


    auto compressed = exomizerCompress(data);

    std::vector<unsigned char> decompressed(data.size());
    int b = uncompress((char*)compressed.data(), compressed.size(), (char*)decompressed.data(), decompressed.size());


    REQUIRE(data.size() == (size_t)b);

    REQUIRE(data == decompressed);


}
