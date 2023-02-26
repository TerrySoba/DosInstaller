#include <stdio.h>
#include "version.h"
#include "yar_decompressor.h"


int main()
{
    printf("Decompressing file test.yar\n");
    if (!decompressArchive("test.yar", ""))
    {
        fprintf(stderr, "Error during decompression.\n");
        return 1;
    }
    printf("Build date: %s\n", BUILD_DATE);
    return 0;
}
