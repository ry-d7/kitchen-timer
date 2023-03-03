/**
 * @file switches.cpp
 * @brief
 *
 */

#include "switches.h"
#include "hardware/gpio.h"

namespace switches
{
namespace
{
enum
{
    SwitchOff,
    SwitchOn,
    SwitchHold,
    SwitchRepeat,
};

enum
{
    HoldCount = 10,
    RepeatCount = 9,
};

uint8_t SwGpios[NumSwId] = {
    0, // SwId10s
    1, // SwId1m
    2, // SwId10m
    3, // SwIdStartStop
    4, // SwIdReset
};

uint8_t prev_state[NumSwId];
bool changed_from_prev[NumSwId];

} // namespace

void init()
{
    for (auto i = 0; i < NumSwId; ++i)
    {
        auto pin = SwGpios[i];
        gpio_init(pin);
        gpio_set_dir(pin, false);         // input
        gpio_set_pulls(pin, true, false); // pull-up

        prev_state[i] = gpio_get(pin); // initial state
        changed_from_prev[i] = false;
    }
}

void onTick()
{
    for (auto i = 0; i < NumSwId; ++i)
    {
        auto pin = SwGpios[i];
        auto state = gpio_get(pin);
        if (prev_state[i] != state)
        {
            prev_state[i] = state;
            changed_from_prev[i] = true;
        }
    }
}

bool changed(int id)
{
    return changed_from_prev[id];
}

bool isOn(int id)
{
    return !prev_state[id];
}

void clearFlag()
{
    for (auto i = 0; i < NumSwId; ++i)
    {
        changed_from_prev[i] = false;
    }
}

} // namespace switches
