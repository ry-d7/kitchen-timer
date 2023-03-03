#pragma once

namespace oled
{
enum
{
    OledWidth = 128,
    OledHeight = 64,

    NumOledByte = OledWidth * OledHeight / 8,
};

void init();
int transmit();
void waitTransmissionEnd();
void transmitDma();
void copyIntoBuffer(uint8_t *data, uint32_t size);
uint8_t getSample();
} // namespace oled
