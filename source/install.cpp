#include "yar_decompressor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <direct.h>

bool isWhitespace(const char ch)
{
    return ch <= ' ';
}

enum ParserState
{
    STATE_NEW_LINE = 1,
    STATE_KEY_NAME = 2,
    STATE_KEY_NAME_READ = 3,
    STATE_VALUE = 4,
    STATE_VALUE_READ = 5,
};

const size_t BUFFER_SIZE = 512;


class KeyValueHandler
{
public:
    virtual void handleKeyValue(const char* key, const char* value) = 0;
};

struct InstallParameters : public KeyValueHandler {
    InstallParameters()
    {
        targetName = NULL;
        version = NULL;
        date = NULL;
        copyright = NULL;
        link = NULL;
        directoryName = NULL;
        archiveName = NULL;
        gameExecutable = NULL;
    }

    ~InstallParameters()
    {
        free(targetName);
        free(version);
        free(date);
        free(copyright);
        free(link);
        free(directoryName);
        free(archiveName);
        free(gameExecutable);
    }

    virtual void handleKeyValue(const char* key, const char* value)
    {
        if (strcmp(key, "target_name") == 0)
        {
            targetName = strdup(value);
        }
        if (strcmp(key, "version") == 0)
        {
            version = strdup(value);
        }
        if (strcmp(key, "date") == 0)
        {
            date = strdup(value);
        }
        if (strcmp(key, "copyright") == 0)
        {
            copyright = strdup(value);
        }
        if (strcmp(key, "link") == 0)
        {
            link = strdup(value);
        }
        if (strcmp(key, "directory_name") == 0)
        {
            directoryName = strdup(value);
        }
        if (strcmp(key, "archive_name") == 0)
        {
            archiveName = strdup(value);
        }
        if (strcmp(key, "game_executable") == 0)
        {
            gameExecutable = strdup(value);
        }
        
    }

    char* targetName;
    char* version;
    char* date;
    char* copyright;
    char* link;
    char* directoryName;
    char* archiveName;
    char* gameExecutable;
};


void parseInstallerIni(const char* filename, KeyValueHandler& handler)
{
    FILE* fp = fopen(filename, "rb");

    ParserState state = STATE_NEW_LINE;

    char* buf = (char*)malloc(BUFFER_SIZE);
    size_t bufPos = 0;
    char* key = NULL;
    char* value = NULL;
    char ch;
    while (!feof(fp))
    {
        switch(state)
        {
            case STATE_NEW_LINE:
                ch = getc(fp);
                if (!isWhitespace(ch))
                {
                    bufPos = 0;
                    state = STATE_KEY_NAME;
                }
                break;
            case STATE_KEY_NAME:
                buf[bufPos] = ch;
                ++bufPos;
                state = STATE_KEY_NAME_READ;
                break;
            case STATE_KEY_NAME_READ:
                ch = getc(fp);
                if (ch == '=')
                {
                    buf[bufPos] = '\0';
                    free(key);
                    key = strdup(buf);
                    bufPos = 0;
                    ch = getc(fp); // advance over '=' character
                    state = STATE_VALUE;
                }
                else
                {
                    state = STATE_KEY_NAME;
                }
                break;
            case STATE_VALUE:
                buf[bufPos] = ch;
                ++bufPos;
                state = STATE_VALUE_READ;
                break;
            case STATE_VALUE_READ:
                ch = getc(fp);
                if (ch == '\n' || ch == '\r')
                {
                    buf[bufPos] = '\0';
                    free(value);
                    value = strdup(buf);
                    bufPos = 0;

                    // printf("Found: (%s), (%s)\n", key, value);
                    handler.handleKeyValue(key, value);
                    state = STATE_NEW_LINE;
                }
                else
                {
                    state = STATE_VALUE;
                }
                break;
        }
    }

    free(key);
    free(value);

    fclose(fp);
}

int main(int argc, char* argv[])
{
    try
    {
        InstallParameters params;

        parseInstallerIni("install.ini", params);

        printf("%s installation\n", params.targetName);
        printf("Copyright (c) %s %s\n", params.date, params.copyright);

        printf("Install directory: %s\n", params.directoryName);

        char ch = 0;

        while (ch != 'n' && ch != 'N' && ch != 'y' && ch != 'Y')
        {
            printf("\nWould you like to change the install directory? [y/n] ");
            fflush(stdout);
            ch = getchar();
        }

        if (ch == 'y' || ch == 'Y')
        {
            printf("Please enter new install directory: ");
            fflush(stdout);
            char buf[128];
            scanf("%s", buf);
            free(params.directoryName);
            params.directoryName = strdup(buf);
        }

        decompressArchive(params.archiveName, params.directoryName);

        chdir(params.directoryName);

        printf("\nInstallation finished!\n\nType GAME.EXE to run game.\n");
    }
    catch(std::string ex)
    {
        fprintf(stderr, "Error: %s\n", ex.c_str());
        return 1;
    }

    return 0;
}
