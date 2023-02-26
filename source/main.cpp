#include <stdio.h>
#include <string>
#include "version.h"
#include "yar_decompressor.h"


int main()
{
    try
    {
        printf("Decompressing file test.yar\n");
        decompressArchive("test.yar", "");
        printf("Build date: %s\n", BUILD_DATE);
    }
    catch(const std::string& str)
    {
        printf("Error: %s\n", str.c_str());
        return 1;
    }
    
    
    return 0;
}
