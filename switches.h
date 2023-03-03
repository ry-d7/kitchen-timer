/**
 * @file switches.h
 * @brief
 * 
 */

#ifndef SWITCHES_H
#define SWITCHES_H

#include <cstdint>

namespace switches
{

enum
{
    SwId10s,
    SwId1m,
    SwId10m,
    SwIdStartStop,
    SwIdReset,

    NumSwId,
};


void init();
void onTick();

bool changed(int id);
bool isOn(int id);
void clearFlag();

} // namespace switches

#endif // SWITCHES_H
