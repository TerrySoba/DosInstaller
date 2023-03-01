#include <stdio.h>
#include <string>
#include "version.h"
#include "yar_decompressor.h"


struct ComandlineParameters
{
    const char* archiveName;
    const char* outputDirectory;
    bool ok;
};


ComandlineParameters pargeCommandline(int argc, char* argv[])
{
    ComandlineParameters parameters;

    if (argc < 2)
    {
        parameters.ok = false;
        return parameters;
    }
    parameters.ok = true;
    parameters.archiveName = argv[1];
    parameters.outputDirectory = "";
    if (argc > 2)
    {
        parameters.outputDirectory = argv[2];
    }

    return parameters;
}


void printUsage(const char* programName)
{
    printf("Usage: %s ARCHIVE [OUTPUT DIRECTORY]\n", programName);
    printf("Decompresses yar archives into the given output directory or\ninto the current directory if no output directory is given.\n");
}


int main(int argc, char* argv[])
{
    ComandlineParameters parameters = pargeCommandline(argc, argv);
    if (!parameters.ok)
    {
        printUsage(argv[0]);
        return 1;
    }


    printf("UnYar build: %s\n", BUILD_DATE);
    printf("Decompressing file %s\n", parameters.archiveName);

    try
    {
        decompressArchive(parameters.archiveName, parameters.outputDirectory);   
    }
    catch(const std::string& str)
    {
        printf("Error: %s\n", str.c_str());
        return 1;
    }
    
    
    return 0;
}
