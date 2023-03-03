/**
 * @file display.h
 * @brief
 * 
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstdint>
#include "hardware/timer.h"

namespace display
{
enum
{
    DispStateNormal,
    DispStateBlink,
    DispStateBlinkFast,
    DispStateMute,
};

void init();
void start();

void setState(uint8_t s);

void update(); // get time values and make the string to show
    
} // namespace display


#endif // DISPLAY_H
