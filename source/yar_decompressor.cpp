#include "yar_decompressor.h"
#include "lzg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <stdint.h>
#include <dos.h>

const char* FILE_FORMAT_HEADER = "\x01" "yar";

bool decompressArchive(const char* archiveFilename, const char* outputDirectory)
{
    // first open file
    FILE* fp = fopen(archiveFilename, "rb");
    if (!fp)
    {
        return false;
    }

    // now check signature of file
    size_t headerSize = strlen(FILE_FORMAT_HEADER);
    std::vector<char> header(headerSize + 1);
    header[headerSize] = '\0';
    fread(&header[0], headerSize, 1, fp);
    if (strcmp(FILE_FORMAT_HEADER, &header[0]) != 0)
    {
        return false;
    }

    // read number of files in archive
    uint32_t fileCount = 0;
    fread(&fileCount, sizeof(fileCount), 1, fp);

    printf("No. of files: %d\n", fileCount);

    // now loop over each file in the archive
    for (int i = 0; i < fileCount; ++i)
    {
        // read filename
        uint32_t filenameLegth = 0;
        int bytes = fread(&filenameLegth, sizeof(filenameLegth), 1, fp);
        // printf("bytes: %d\n", bytes);
        // printf("len: %d\n", filenameLegth);

        // read filename
        std::vector<char> filename(filenameLegth + 1);
        filename[filenameLegth] = '\0';
        fread(&filename[0], filenameLegth, 1, fp);
        printf("Filename: %s\n", &filename[0]);

        // read dos date
        uint16_t dosDate = 0;
        fread(&dosDate, sizeof(dosDate), 1, fp);

        // read dos time
        uint16_t dosTime = 0;
        fread(&dosTime, sizeof(dosTime), 1, fp);

        // now read compressed data
        uint32_t uncompressedSize = 0;
        fread(&uncompressedSize, sizeof(uncompressedSize), 1, fp);
        uint32_t compressedSize = 0;
        fread(&compressedSize, sizeof(compressedSize), 1, fp);

        // printf("orig: %ld comp: %ld\n", uncompressedSize, compressedSize);

        std::vector<char> compressed(compressedSize);
        fread(&compressed[0], compressedSize, 1, fp);

        // now uncompress data
        std::vector<char> uncompressed(uncompressedSize);
        size_t decSize = LZG_Decode(
            (unsigned char*)&compressed[0],
            compressedSize,
            (unsigned char*)&uncompressed[0],
            uncompressedSize);

        if (!decSize)
        {
            return false;
        }

        FILE* out = fopen(&filename[0], "wb");
        if (!out)
        {
            return false;
        }

        fwrite(&uncompressed[0], uncompressedSize, 1, out);
        fclose(out);

        int handle = 0;
        _dos_open(&filename[0], _A_NORMAL, &handle);
        _dos_setftime(handle, dosDate, dosTime);
        _dos_close(handle);
    }

    return true;
}
