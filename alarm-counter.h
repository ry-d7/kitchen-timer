/**
 * @file counter.h
 * @brief
 *
 */

#ifndef ALARM_COUNTER_H
#define ALARM_COUNTER_H

#include <cstdint>

namespace alarmCounter
{
enum CountState
{
    CountReset,
    CountIdle,
    CountSetting,
    CountDown,
    CountDownPause,
    CountUp,
    CountUpPause,
};
enum
{
    TickPerSec = 10, // tick interval = 100msec
};
struct Count {
    int32_t count;
    int16_t minutes;
    int8_t seconds;
    int8_t ticks;

    Count()
        : count(0), minutes(0), seconds(0), ticks(0)
    {
    }

    Count(int16_t m, int8_t s)
        : count(0), minutes(m), seconds(s), ticks(0)
    {
        setCountFromTime();
    }

    void reset()
    {
        count = 0;
        minutes = 0;
        seconds = 0;
        ticks = 0;
    }

    void setCountFromTime()
    {
        count = (minutes * 60 + seconds) * TickPerSec + ticks;
    }

    bool increment() // return true if the seconds changed
    {
        ++count;
        ++ticks;
        if (TickPerSec <= ticks)
        {
            ticks -= TickPerSec;
            ++seconds;
            if (60 <= seconds)
            {
                ++minutes;
                seconds -= 60;
            }
            return true;
        }

        return false;
    }

    Count diffFrom(const Count &c)
    {
        int diffS = c.seconds - seconds;
        int diffM = c.minutes - minutes;
        if (diffS < 0)
        {
            diffS += 60;
            diffM -= 1;
        }

        Count r;
        r.minutes = diffM;
        r.seconds = diffS;
        r.setCountFromTime();
        return Count(diffM, diffS);
    }

    void add10s()
    {
        seconds += 10;
        if (60 <= seconds)
        {
            seconds -= 60;
            ++minutes;
        }
    }

    void add1m()
    {
        minutes += 1;
    }

    void add10m()
    {
        minutes += 10;
    }
};

void init();
void reset();

void addTarget10s();
void addTarget1m();
void addTarget10m();

void startCount();
void stop();
void toggleStartStop();

void tick();

Count getTimeForDisp();
} // namespace alarmCounter

#endif // ALARM_COUNTER_H
