#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <filesystem>

#include "lzg.h"

struct Arguments
{
    std::vector<std::string> inputFiles;
    std::string outputFile;
};

void printUsage(const std::string &commandName)
{
    std::cerr << "Usage: " << commandName << " INPUT... OUTPUT\n\n"
              << "Compresses given INPUT(s) to given output file." << std::endl;
}

std::optional<Arguments> parseCommandLineArguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        return {};
    }

    Arguments arguments;
    for (int i = 1; i < argc - 1; ++i)
    {
        arguments.inputFiles.push_back(argv[i]);
    }

    arguments.outputFile = argv[argc - 1];

    return arguments;
}

std::vector<unsigned char> compressData(const std::vector<unsigned char>& data)
{
    // Determine maximum size of compressed data
    lzg_uint32_t maxEncSize = LZG_MaxEncodedSize(data.size());
    std::vector<unsigned char> encBuf(maxEncSize);

    // Compress
    lzg_uint32_t encSize = LZG_Encode(data.data(), data.size(), encBuf.data(), maxEncSize, NULL);
    if (!encSize)
    {
        throw std::runtime_error("Compression failed!");
    }

    encBuf.resize(encSize);
    return encBuf;
}

std::vector<unsigned char> readFile(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        throw std::runtime_error("Could not open file \"" + filename + "\".");
    }
    fseek(fp, 0L, SEEK_END);
    auto fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    std::vector<unsigned char> data(fileSize);
    auto bytesRead = fread(data.data(), 1, fileSize, fp);

    if (bytesRead != fileSize)
    {
        throw std::runtime_error("Could not read file \"" + filename + "\".");
    }

    fclose(fp);

    return data;
}

const std::string FILE_FORMAT_HEADER = "\x01" "yar";


template<typename TP>
std::time_t to_time_t(TP tp) {
    auto system_clock_time_point = std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - TP::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(system_clock_time_point);
}


uint16_t toDosDate(int year, int month, int day)
{
    return 
        (day << 0) | 
        (month << 5) |
        ((year - 1980) << 9);
}

uint16_t toDosTime(int hours, int minutes, int seconds)
{
    return
        (seconds / 2) |
        (minutes << 5) |
        (hours << 11);
}

class CompressedArchive
{
public:
    void addFile(const std::string& filename)
    {
        filesToBeCompressed.push_back(filename);
    }

    void writeArchive(const std::string& filename)
    {
        if (std::filesystem::exists(filename))
        {
            throw std::runtime_error("Output file already exists \"" + filename + "\".");
        }
        FILE* fp = fopen(filename.c_str(), "wb");
        if (!fp)
        {
            throw std::runtime_error("Could not open file \"" + filename + "\".");
        }

        // write file header magic bytes
        fwrite(FILE_FORMAT_HEADER.data(), 1, FILE_FORMAT_HEADER.size(), fp);

        // write number of files in archive
        uint32_t fileCount = filesToBeCompressed.size();
        fwrite(&fileCount, sizeof(fileCount), 1, fp);

        // now compress each file
        for (const std::string& filenameFull : filesToBeCompressed)
        {
            auto time = std::filesystem::last_write_time(filenameFull);
            auto t = to_time_t(time);

            tm local_tm = *localtime(&t);

            int year = local_tm.tm_year + 1900;
            int month = local_tm.tm_mon + 1;
            int day = local_tm.tm_mday;
            int hour = local_tm.tm_hour;
            int minute = local_tm.tm_min;
            int second = local_tm.tm_sec;

            std::filesystem::path path(filenameFull);
            std::string filename = path.filename();

            auto uncompressed = readFile(filenameFull);
            uint32_t uncompressedSize = uncompressed.size();
            auto compressed = compressData(uncompressed);
            uint32_t compressedSize = compressed.size();
            uint32_t filenameSize = filename.size();
            uint16_t dosDate = toDosDate(hour, minute, second);
            uint16_t dosTime = toDosTime(year, month, day);


            std::cout << "File: " << filename << " (" << (((float)compressedSize / uncompressedSize) * 100) << "%)\n";

            // write filename size in bytes, 32bit unsigned
            fwrite(&filenameSize, sizeof(filenameSize), 1, fp);
            // write filename
            fwrite(filename.c_str(), filename.size(), 1, fp);

            // write DOS date
            fwrite(&dosDate, sizeof(dosDate), 1, fp);
            // write DOS time
            fwrite(&dosTime, sizeof(dosTime), 1, fp);

            // write uncompressed size in bytes, 32bit unsigned
            fwrite(&uncompressedSize, sizeof(uncompressedSize), 1, fp);
            // write compressed size in bytes, 32bit unsigned
            fwrite(&compressedSize, sizeof(compressedSize), 1, fp);
            // write compressed data
            fwrite(compressed.data(), compressed.size(), 1, fp);
        }

        fclose(fp);
    }

private:
    std::vector<std::string> filesToBeCompressed;
};

int main(int argc, char *argv[])
{
    try
    {
        auto arguments = parseCommandLineArguments(argc, argv);

        if (!arguments)
        {
            std::string commandName;
            if (argc > 0)
            {
                commandName = argv[0];
            }
            printUsage(commandName);
            return 1;
        }

        CompressedArchive archive;
        for (const std::string file : arguments->inputFiles)
        {
            archive.addFile(file);
        }
        archive.writeArchive(arguments->outputFile);

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
