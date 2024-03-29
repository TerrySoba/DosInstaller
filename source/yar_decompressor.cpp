#include "yar_decompressor.h"
#include "exodecrunch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <dos.h>
#include <malloc.h>
#include <iostream>

#include <sys/types.h>
#include <direct.h>

const char* FILE_FORMAT_HEADER = "\x01" "ya2";

uint16_t crc16(const char *ptr, int32_t count, uint16_t initialValue = 0)
{
    uint16_t crc;
    int8_t i;
    crc = initialValue;
    while (--count >= 0)
    {
        crc = crc ^ (int)*ptr++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        } while (--i);
    }
    return (crc);
}


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
        if (uncompressedBytes >= uncompressedSize)
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


void decompressArchive(const char* archiveFilename, const char* outputDirectory)
{
    // first open file
    FILE* fp = fopen(archiveFilename, "rb");
    if (!fp)
    {
        throw std::string("Could not open archive file: ") + archiveFilename;
    }

    // now check signature of file
    size_t headerSize = strlen(FILE_FORMAT_HEADER);
    std::vector<char> header(headerSize + 1);
    header[headerSize] = '\0';
    fread(&header[0], headerSize, 1, fp);
    if (strcmp(FILE_FORMAT_HEADER, &header[0]) != 0)
    {
        throw std::string("Header of archive is incorrect: ") + archiveFilename;
    }

    // read number of files in archive
    uint32_t fileCount = 0;
    fread(&fileCount, sizeof(fileCount), 1, fp);

    printf("No. of files: %d\n", fileCount);

    // create output directory
    if (outputDirectory && outputDirectory[0] != '\0')
    {
        mkdir(outputDirectory);
    }

    // now loop over each file in the archive
    for (int i = 0; i < fileCount; ++i)
    {
        // read filename
        uint32_t filenameLegth = 0;
        int bytes = fread(&filenameLegth, sizeof(filenameLegth), 1, fp);

        // read filename
        std::vector<char> filename(filenameLegth + 1);
        filename[filenameLegth] = '\0';
        fread(&filename[0], filenameLegth, 1, fp);
        printf("Decompressing: %s ", &filename[0]);
        fflush(stdout);

        // read dos date
        uint16_t dosDate = 0;
        fread(&dosDate, sizeof(dosDate), 1, fp);

        // read dos time
        uint16_t dosTime = 0;
        fread(&dosTime, sizeof(dosTime), 1, fp);

        // read crc16 checksum
        uint16_t checksum;
        fread(&checksum, sizeof(checksum), 1, fp);

        // read number of chunks
        uint32_t numberOfChunks;
        fread(&numberOfChunks, sizeof(numberOfChunks), 1, fp);

        std::string outputFilename;
        if (outputDirectory && outputDirectory[0] != '\0')
        {   
            outputFilename += std::string(outputDirectory) + "\\";
        }
        outputFilename += std::string(&filename[0]);

        FILE* out = fopen(outputFilename.c_str(), "wb");
        if (!out)
        {
            throw std::string("Could not open file for output: ") + &filename[0];
        }

        uint16_t crc = 0;

        for (uint32_t chunk = 0; chunk < numberOfChunks; ++chunk)
        {
            // now read compressed data
            uint32_t uncompressedSize = 0;
            fread(&uncompressedSize, sizeof(uncompressedSize), 1, fp);
            uint32_t compressedSize = 0;
            fread(&compressedSize, sizeof(compressedSize), 1, fp);

            std::vector<char> compressed(compressedSize);
            fread(&compressed[0], compressedSize, 1, fp);

            // now uncompress data
            std::vector<char> uncompressed(uncompressedSize);
            size_t decSize = uncompress(&compressed[0], compressed.size(), &uncompressed[0], uncompressed.size());

            if (!decSize)
            {
                throw std::string("Decompression failed: ") + &filename[0];
            }

            // calculate crc16 checksum of uncompressed data
            crc = crc16(&uncompressed[0], uncompressedSize, crc);

            fwrite(&uncompressed[0], uncompressedSize, 1, out);
            printf(".");
            fflush(stdout);
        }
        fclose(out);

        printf("\n");

        if (checksum != crc)
        {
            throw std::string("Checksum is incorrect.");
        }

        int handle = 0;

        _dos_open(&filename[0], _A_NORMAL, &handle);
        _dos_setftime(handle, dosDate, dosTime);
        _dos_close(handle);
    }

    fclose(fp);
}
