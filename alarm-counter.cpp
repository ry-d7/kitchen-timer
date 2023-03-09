/**
 * @file alarm-counter.cpp
 * @brief
 *
 */

#include <cstdio>
#include "alarm-counter.h"
#include "alarm-sound.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "display.h"

#define GPIO_T2 (17)

namespace alarmCounter
{
namespace
{
uint8_t state;

Count target;
Count current;

struct repeating_timer timer;

bool processCountDown() // return true if the countdown finished
{
    // puts("pcd");
    current.increment();
    if (target.count == current.count)
    {
        return true;
    }
    return false;
}

bool processCountUp() // return true if need to change the state (change won't happen by normal count up but by the reset button etc, so this func always returns false)
{
    current.increment();
    return false;
}

void syncTargetCountWithTime()
{
    target.setCountFromTime();
}

bool timerCallback(struct repeating_timer *t)
{
    // puts("accb");
    if (state != CountDown && state != CountUp)
    {
        puts("cancel");
        cancel_repeating_timer(t);
        return true;
    }
    // puts("tick");
    // gpio_put(GPIO_T2, true);
    tick();
    // gpio_put(GPIO_T2, false);
    return true;
}

} // namespace

void init()
{
    reset();
}

void reset()
{
    state = CountReset;
    target.reset();
    current.reset();
    // gpio_init(GPIO_T2);
    // gpio_set_dir(GPIO_T2, true);
}

void addTarget10s()
{
    if (state == CountReset)
    {
        state = CountSetting;
        display::setState(display::DispStateBlink);
    }
    else if (CountDown <= state)
    {
        return;
    }

    target.add10s();
}

void addTarget1m()
{
    if (state == CountReset)
    {
        state = CountSetting;
        display::setState(display::DispStateBlink);
    }
    else if (CountDown <= state)
    {
        return;
    }
    target.add1m();
}

void addTarget10m()
{
    if (state == CountReset)
    {
        state = CountSetting;
        display::setState(display::DispStateBlink);
    }
    else if (CountDown <= state)
    {
        return;
    }
    target.add10m();
}

void startCount()
{
    switch (state)
    {
    case CountDown:
    case CountUp:
        return;
    case CountDownPause:
        state = CountDown;
        break;
    case CountReset:
    case CountUpPause:
        state = CountUp;
        break;
    default:
        syncTargetCountWithTime();
        state = CountDown;
        break;
    
    }

    add_repeating_timer_ms(-100, timerCallback, nullptr, &timer);
    display::setState(display::DispStateNormal);

}

void stop()
{
    if (state <= CountSetting)
    {
        reset();
        return;
    }

    if (state == CountDown)
    {
        state = CountDownPause;
        return;
    }
    if (state == CountUp)
    {
        state = CountUpPause;
        return;
    }

    state = CountIdle;
}
void toggleStartStop()
{
    switch (state)
    {
    case CountDown:
    case CountUp:
        stop();
        break;
    default:
        startCount();
        break;
    }
}

void tick()
{
    // puts("actick");
    switch (state)
    {
    case CountDown:
        if (processCountDown())
        {
            state = CountUp;
            // @todo start alarm sound
            alarmSound::reset();
            alarmSound::select(alarmSound::seq_time_has_come);
            alarmSound::start();
        }
        break;
    case CountUp:
        processCountUp();
        break;

    default:
        break;
    }
}

Count getTimeForDisp()
{
    switch (state)
    {
    case CountDown:
    case CountDownPause:
        return current.diffFrom(target);

    case CountUp:
    case CountUpPause:
        return target.diffFrom(current);

    default:
        break;
    }

    return target;
}

} // namespace alarmCounter
