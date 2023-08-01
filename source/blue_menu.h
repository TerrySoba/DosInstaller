#ifndef _BLUE_MENU_H_
#define _BLUE_MENU_H_

#include <stdint.h>

class BlueMenu
{
public:
    BlueMenu();
    ~BlueMenu();

    void drawBoxWithCenteredText(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, const char* text);
    uint8_t getScreenWidth() const { return m_screenWidth; }

    void clearScreen();

private:
    uint8_t m_screenWidth;
};


#endif