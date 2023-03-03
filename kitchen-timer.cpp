#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "oled.h"
#include "Graphics.h"
#include "display.h"
#include "alarm-counter.h"
#include "alarm-sound.h"
#include "switches.h"

// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//     // Put your timeout handler code in here
//     return 0;
// }

void handle_switches()
{
    switches::onTick();
    for (auto i = 0; i < switches::NumSwId; ++i)
    {
        if (switches::changed(i) && switches::isOn(i))
        {
            switch (i)
            {
            case switches::SwId10s:
                alarmCounter::addTarget10s();
                break;
            case switches::SwId1m:
                alarmCounter::addTarget1m();
                break;
            case switches::SwId10m:
                alarmCounter::addTarget10m();
                break;
            case switches::SwIdStartStop:
                alarmCounter::toggleStartStop();
                break;
            case switches::SwIdReset:
                alarmCounter::reset();
                break;
            
            default:
                break;
            }
            alarmSound::reset();
            alarmSound::select(alarmSound::seq_button_pressed);
            alarmSound::start();
        }
    }
    switches::clearFlag();
}

int main()
{
    stdio_init_all();

    oled::init();
    graphics::clear();
    graphics::setToDispBuffer();
    oled::transmitDma();
    display::init();
    alarmCounter::init();
    alarmSound::init();
    switches::init();

    sleep_ms(500);
    display::start();

#if 0 // test display and counter
    puts("slp1");
    sleep_ms(1000);
    puts("add1m");
    alarmCounter::add1m();
    puts("slp1");
    sleep_ms(1000);
    puts("add10s");
    alarmCounter::addTarget10s();
    display::update();
    puts("slp1");
    sleep_ms(1000);
    puts("add10s");
    alarmCounter::addTarget10s();
    display::update();
    puts("slp1");
    sleep_ms(1000);
    alarmCounter::startCountDown();
#endif
    display::update();

    // // Timer example code - This example fires off the callback after 2000ms
    // add_alarm_in_ms(2000, alarm_callback, NULL, false);

    while (true)
    {
        sleep_ms(10);
        display::update();

        static uint32_t cnt = 0;
        if (!(++cnt & 0x01))
        {
            continue;
        }
        handle_switches();
    }

    return 0;
}
