#include "blue_menu.h"

#include <stdint.h>
#include <dos.h>
#include <stdio.h>

enum BiosColor
{
    BIOS_BLACK = 0,
    BIOS_BLUE = 1,
    BIOS_GREEN = 2,
    BIOS_CYAN = 3,
    BIOS_RED = 4,
    BIOS_MAGENTA = 5,
    BIOS_BROWN = 6,
    BIOS_LIGHT_GRAY = 7,
    BIOS_DARK_GRAY = 8,
    BIOS_LIGHT_BLUE = 9,
    BIOS_LIGHT_GREEN = 10,
    BIOS_LIGHT_CYAN = 11,
    BIOS_LIGHT_RED = 12,
    BIOS_LIGHT_MAGENTA = 13,
    BIOS_YELLOW = 14,
    BIOS_WHITE = 15,
};

int strlen(const char* str)
{
    int len = 0;
    while (*str++)
    {
        ++len;
    }
    return len;
}


void findBoundingBox(const char* str, int& width, int& height)
{
    int x = 0;
    int y = 0;
    int maxX = 0;
    int maxY = 0;
    while (*str)
    {
        if (*str == '\n')
        {
            maxX = x > maxX ? x : maxX;
            maxY = y > maxY ? y : maxY;
            x = 0;
            ++y;
        }
        else
        {
            ++x;
        }
        ++str;
    }
    maxX = x > maxX ? x : maxX;
    maxY = y > maxY ? y : maxY;
    width = maxX;
    height = maxY;
}


int getScreenWidth()
{
    // read current DOS screen mode
    union REGS regs;
    regs.h.ah = 0x0F;
    int86(0x10, &regs, &regs);
    int screenWidth = regs.h.ah;
    return screenWidth;
}

void setCursorPosition(uint8_t x, uint8_t y)
{
    // set cursor position
    union REGS regs;
    regs.h.ah = 0x02;
    regs.h.bh = 0x00;
    regs.h.dh = y;
    regs.h.dl = x;
    int86(0x10, &regs, &regs);
}

void writeCharacter(uint8_t ch, uint8_t fgColor, uint8_t bgColor, uint16_t count = 1)
{
    uint8_t color = (bgColor << 4) | fgColor;

    // write character
    union REGS regs;
    regs.h.ah = 0x09;
    regs.h.bh = 0x00;
    regs.h.bl = color;
    regs.h.al = ch;
    regs.x.cx = count;
    int86(0x10, &regs, &regs);
}

void setActiveDisplayPage(uint8_t page)
{
    // set active display page
    union REGS regs;
    regs.h.ah = 0x05;
    regs.h.al = page;
    int86(0x10, &regs, &regs);
}

void setTextModeCursorShape(uint16_t shape)
{
    // set text mode cursor shape
    union REGS regs;
    regs.h.ah = 0x01;
    regs.x.cx = shape;
    int86(0x10, &regs, &regs);
}


void drawBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    // draw left border
    for (uint8_t y = y1; y <= y2; ++y)
    {
        setCursorPosition(x1, y);
        writeCharacter('\xb3', BIOS_LIGHT_GRAY, BIOS_BLUE);
    }

    // draw right border
    for (uint8_t y = y1; y <= y2; ++y)
    {
        setCursorPosition(x2, y);
        writeCharacter('\xb3', BIOS_LIGHT_GRAY, BIOS_BLUE);
    }
    
    // draw top border
    setCursorPosition(x1+1, y1);
    writeCharacter('\xc4', BIOS_LIGHT_GRAY, BIOS_BLUE, x2 - x1 - 1);
    
    // draw bottom border
    setCursorPosition(x1+1, y2);
    writeCharacter('\xc4', BIOS_LIGHT_GRAY, BIOS_BLUE, x2 - x1 - 1);

    // draw top left corner
    setCursorPosition(x1, y1);
    writeCharacter('\xda', BIOS_LIGHT_GRAY, BIOS_BLUE);

    // draw top right corner
    setCursorPosition(x2, y1);
    writeCharacter('\xbf', BIOS_LIGHT_GRAY, BIOS_BLUE);

    // draw bottom left corner
    setCursorPosition(x1, y2);
    writeCharacter('\xc0', BIOS_LIGHT_GRAY, BIOS_BLUE);

    // draw bottom right corner
    setCursorPosition(x2, y2);
    writeCharacter('\xd9', BIOS_LIGHT_GRAY, BIOS_BLUE);

    // draw drop shadow
    setCursorPosition(x1+1, y2+1);
    writeCharacter('\xb0', BIOS_LIGHT_GRAY, BIOS_BLACK, x2 - x1);
    for (uint8_t y = y1+1; y <= y2+1; ++y)
    {
        setCursorPosition(x2+1, y);
        writeCharacter('\xb0', BIOS_LIGHT_GRAY, BIOS_BLACK);
    }

    // fill box
    for (uint8_t y = y1+1; y < y2; ++y)
    {
        setCursorPosition(x1+1, y);
        writeCharacter(' ', BIOS_LIGHT_GRAY, BIOS_BLUE, x2 - x1 - 1);
    }

}

void printMultiLineText(uint8_t x, uint8_t y, const char* text)
{
    setCursorPosition(x, y);
    while (*text)
    {
        if (*text == '\n')
        {
            ++y;
            setCursorPosition(x, y);
        }
        else
        {
            printf("%c", *text);
            fflush(stdout);
        }
        ++text;
    }
}

void BlueMenu::drawBoxWithCenteredText(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, const char* text)
{
    drawBox(x1, y1, x2, y2);

    int width, height;
    findBoundingBox(text, width, height);

    int textX = x1 + (x2 - x1 - width) / 2;
    int textY = y1 + (y2 - y1 - height) / 2;
    printMultiLineText(textX, textY, text);
}

void BlueMenu::clearScreen()
{
    setActiveDisplayPage(0);
    
    for (int y = 0; y <= 25; ++y)
    {
        setCursorPosition(0, y);
        writeCharacter('\xb1', BIOS_LIGHT_GRAY, BIOS_BLUE, m_screenWidth);
    }
}

BlueMenu::BlueMenu()
{
    m_screenWidth = getScreenWidth();
    setTextModeCursorShape(0x2607); // invisible cursor
    clearScreen();
}

BlueMenu::~BlueMenu()
{

    // clear screen
    for (int y = 0; y <= 25; ++y)
    {
        setCursorPosition(0, y);
        writeCharacter(' ', BIOS_LIGHT_GRAY, BIOS_BLACK, m_screenWidth);
    }
    setCursorPosition(0,0);
    // reenable cursor
    setTextModeCursorShape(0x0607);
}
