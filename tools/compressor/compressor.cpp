#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <filesystem>
#include <algorithm>

#include "exo_helper.h"

struct Arguments
{
    std::vector<std::string> inputFiles;
    std::string outputFile;
};

void printUsage(const std::string &commandName)
{
    std::cerr << "Usage: " << commandName << " INPUT... OUTPUT\n\n"
              << "Compresses given INPUT(s) to given output file.\n\n"
              << "Build date: " << BUILD_DATE << "\n"
              << "https://github.com/TerrySoba/DosInstaller" << std::endl;
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
    struct crunch_options options = CRUNCH_OPTIONS_DEFAULT;
    options.direction_forward = 1;
    struct crunch_info info = STATIC_CRUNCH_INFO_INIT;
    struct buf inbuf;
    struct buf outbuf;

    buf_init(&inbuf);
    buf_init(&outbuf);

    inbuf.data = (unsigned char*)data.data();
    inbuf.size = data.size();
    inbuf.capacity = data.size();

    crunch(&inbuf, 0, NULL, &outbuf, &options, &info);
    print_crunch_info(LOG_NORMAL, &info);

    std::vector<unsigned char> output((unsigned char*)outbuf.data, (unsigned char*)outbuf.data + outbuf.size);

    buf_free(&outbuf);
    return output;
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

const std::string FILE_FORMAT_HEADER = "\x01" "ya2";


template<typename TP>
std::time_t to_time_t(TP tp) {
    auto system_clock_time_point = std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - TP::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(system_clock_time_point);
}


uint16_t toDosDate(uint16_t year, uint16_t month, uint16_t day)
{
    year -= 1980;
    return 
        (day << 0) | 
        (month << 5) |
        (year << 9);
}

uint16_t toDosTime(uint16_t hours, uint16_t minutes, uint16_t seconds)
{
    return
        (seconds / 2) |
        (minutes << 5) |
        (hours << 11);
}

uint16_t crc16(const unsigned char *ptr, int32_t count)
{
    uint16_t crc;
    int8_t i;
    crc = 0;
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

uint16_t crc16(const std::vector<unsigned char>& data)
{
    return crc16(data.data(), data.size());
}

const uint32_t MAX_CHUNK_SIZE = 32000;

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
            std::cout << "Compressing file \"" << filenameFull << "\" " << std::flush;
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
            uint16_t checksum = crc16(uncompressed);
            uint32_t filenameSize = filename.size();
            uint16_t dosDate = toDosDate(year, month, day);
            uint16_t dosTime = toDosTime(hour, minute, second);

            // write filename size in bytes, 32bit unsigned
            fwrite(&filenameSize, sizeof(filenameSize), 1, fp);
            // write filename
            fwrite(filename.c_str(), filename.size(), 1, fp);

            // write DOS date
            fwrite(&dosDate, sizeof(dosDate), 1, fp);
            // write DOS time
            fwrite(&dosTime, sizeof(dosTime), 1, fp);

            // write CRC-16 checksum
            fwrite(&checksum, sizeof(checksum), 1, fp);

            uint32_t numberOfChunks = uncompressedSize / MAX_CHUNK_SIZE;
            if (uncompressedSize % MAX_CHUNK_SIZE != 0)
            {
                ++numberOfChunks;
            }

            // write number of chunks, 32bit unsigned
            fwrite(&numberOfChunks, sizeof(numberOfChunks), 1, fp);

            for (uint32_t chunk = 0; chunk < numberOfChunks; ++chunk)
            {
                unsigned char* chunkData = &uncompressed[chunk * MAX_CHUNK_SIZE];
                uint32_t rest = uncompressedSize - (chunk * MAX_CHUNK_SIZE);
                std::vector<unsigned char> uncompressedChunk(chunkData, chunkData + std::min(MAX_CHUNK_SIZE, rest));

                uint32_t uncompressedChunkSize = uncompressedChunk.size();

                auto compressedChunk = compressData(uncompressedChunk);
                uint32_t compressedChunkSize = compressedChunk.size();
                
                std::cout << "." << std::flush;

                // write uncompressed size in bytes, 32bit unsigned
                fwrite(&uncompressedChunkSize, sizeof(uncompressedChunkSize), 1, fp);

                // write compressed size in bytes, 32bit unsigned
                fwrite(&compressedChunkSize, sizeof(compressedChunkSize), 1, fp);

                // write compressed data
                fwrite(compressedChunk.data(), compressedChunk.size(), 1, fp);
            }
            std::cout << std::endl;
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
