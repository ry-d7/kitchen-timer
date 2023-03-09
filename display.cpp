/**
 * @file display.cpp
 * @brief
 *
 */

#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "display.h"
#include "alarm-counter.h"
#include "Graphics.h"
#include "fonts-headers.h"
#include "oled.h"

#define GPIO_T1 (16)

namespace display
{
namespace
{
enum
{
    BlinkCountMax = 5,
};

uint8_t state;
uint8_t prev_state;

uint8_t blink_state;
uint16_t blink_count;

struct repeating_timer timer;
struct alarm_pool *ap;

bool elapsed = false;

int16_t prev_m;
int8_t prev_s;

bool blinkStateOnUpdate() // return true if state changed
{
    if (++blink_count < BlinkCountMax)
    {
        return false;
    }
    blink_count = 0;
    blink_state ^= 1;

    return true;
}

bool timerCallback(struct repeating_timer *t)
{
    // puts("dispcb");
    elapsed = true;
    // update();
    return true;
}

} // namespace

void init()
{
    state = DispStateNormal;
    prev_state = DispStateNormal;
    blink_state = 0;
    blink_count = 0;
    ap = alarm_pool_create(0, 4);

    prev_m = -1;
    prev_s = -1;
    gpio_init(GPIO_T1);
    gpio_set_dir(GPIO_T1, true);
}

void start()
{
    puts("dispstart");
    alarm_pool_add_repeating_timer_ms(ap, -100, timerCallback, nullptr, &timer);
}

void setState(uint8_t s)
{
    if (s == DispStateBlink || s == DispStateBlinkFast)
    {
        if (state != s)
        {
            blink_state = 1;
            blink_count = 0;
        }
    }
    else
    {
        blink_state = 1;
    }
    prev_state = state;
    state = s;
}

void update()
{
    // puts("dispud");
    oled::waitTransmissionEnd();
    if (!elapsed)
    {
        return;
    }

    elapsed = false;
    graphics::clear();

    bool changed_blink = false;

    if (state == DispStateBlink || state == DispStateBlinkFast)
    {
        changed_blink = blinkStateOnUpdate();
        if (!blink_state)
        {
            graphics::setToDispBuffer();
            // oled::transmit();
            oled::transmitDma();
            return;
        }
    }

    // puts("draw");
    auto count = alarmCounter::getTimeForDisp();
    auto m = std::abs(count.minutes);
    auto s = std::abs(count.seconds);

    auto changed_time = (m != prev_m) || (s != prev_s);
    prev_m = m;
    prev_s = s;

    if (!changed_time && !changed_blink && (prev_state == state))
    {
        return;
    }
    prev_state = state;

    // gpio_put(GPIO_T1, true);
    // printf("%dm %ds\n", m, s);
    char buf[5];
    std::memset(buf, 0, sizeof(buf));
    std::sprintf(buf, "%02d", m);
    graphics::drawString(std::string(buf), Font_fortimer_small, 1, 4);
    std::memset(buf, 0, sizeof(buf));
    std::sprintf(buf, "%02d", s);
    graphics::drawString(std::string(buf), Font_fortimer_small, 64, 4);

    graphics::setToDispBuffer();

    // oled::transmit();
    oled::transmitDma();
    // gpio_put(GPIO_T1, false);

    return;
}

} // namespace display
