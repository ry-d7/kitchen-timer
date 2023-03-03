#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "oled.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#define I2C_PORT i2c1
#define I2C_SDA 18
#define I2C_SCL 19

namespace oled
{
namespace
{
enum
{
    I2cAddress = 0x78 >> 1,
};

enum
{
    CoOnlyData = 0x00,
    CoControl = 0x80,
    DCCommand = 0x00,
    DCGddram = 0x40,

    DisplayOff = 0xAE,
    DisplayOn = 0xAF,
    EntireOff = 0xA4,
    EntireOn = 0xA5,
    MemoryAddressing = 0x20,
    ChargePump = 0x8D,
    ChargePumpEn = 0x14,
    ChargePumpDis = 0x10,

    SetColumnAddress = 0x21,
    SetPageAddress = 0x22,

    SetDispStartLine = 0x40,
};

uint8_t buffer[NumOledByte];


int dma_ch;
dma_channel_config dma_conf;

bool transmitting = false;

} // namespace


void init()
{
    puts("oled::init()");
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    uint8_t MemAddr[] = {CoOnlyData | DCCommand, MemoryAddressing, 1};
    if (i2c_write_blocking(I2C_PORT, I2cAddress, MemAddr, sizeof(MemAddr), false) < 0)
    {
        puts("OLED INIT ERROR -- 1");
        return;
    }
    sleep_ms(5);
    uint8_t ChrgPmp[] = {CoOnlyData | DCCommand, ChargePump, ChargePumpEn};
    if (i2c_write_blocking(I2C_PORT, I2cAddress, ChrgPmp, sizeof(ChrgPmp), false) < 0)
    {
        puts("OLED INIT ERROR -- 2");
        return;
    }
    sleep_ms(5);
    uint8_t EntireDispOn[] = {CoOnlyData | DCCommand, EntireOn};
    if (i2c_write_blocking(I2C_PORT, I2cAddress, EntireDispOn, sizeof(EntireDispOn), false) < 0)
    {
        puts("OLED INIT ERROR -- 3");
        return;
    }
    sleep_ms(5);
    EntireDispOn[1] = EntireOff;
    if (i2c_write_blocking(I2C_PORT, I2cAddress, EntireDispOn, sizeof(EntireDispOn), false) < 0)
    {
        puts("OLED INIT ERROR -- 4");
        return;
    }
    sleep_ms(5);
    uint8_t DispOn[] = {CoOnlyData | DCCommand, DisplayOn};
    if (i2c_write_blocking(I2C_PORT, I2cAddress, DispOn, sizeof(DispOn), false) < 0)
    {
        puts("OLED INIT ERROR -- 5");
        return;
    }
    sleep_ms(10);

    // Get a free channel, panic() if there are none
    dma_ch = dma_claim_unused_channel(true);

    // 8 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // No DREQ is selected, so the DMA transfers as fast as it can.

    dma_conf = dma_channel_get_default_config(dma_ch);
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_conf, true);
    channel_config_set_write_increment(&dma_conf, false);

    channel_config_set_dreq(&dma_conf, i2c_get_dreq(I2C_PORT, true));
    // dma_channel_set_irq0_enabled(dma_ch, true);
    // irq_set_exclusive_handler(DMA_IRQ_0, nullptr);

    puts("oled::init() end");
}

namespace
{
uint32_t indexForMakeData = 0;

uint16_t txBufferDma[NumOledByte + 1];
} // namespace

void waitTransmissionEnd()
{
    dma_channel_wait_for_finish_blocking(dma_ch);
    transmitting = false;
}

void transmitDma()
{
    if (transmitting)
    {
        return;
    }

    auto hw = i2c_get_hw(I2C_PORT);
    volatile uint8_t * dst = reinterpret_cast<volatile uint8_t *>(&hw->data_cmd);

    uint8_t SetClmnAddr[] ={SetColumnAddress | 0, 0, 127};
    uint8_t SetPageAddr[] ={SetPageAddress | 0, 0, 7};
    i2c_write_timeout_per_char_us(I2C_PORT, I2cAddress, SetClmnAddr, sizeof(SetClmnAddr), false, 100);
    sleep_us(10);
    i2c_write_timeout_per_char_us(I2C_PORT, I2cAddress, SetPageAddr, sizeof(SetPageAddr), false, 100);
    sleep_us(10);

    uint8_t DispStrtLn[] = {SetDispStartLine | 0};
    i2c_write_timeout_per_char_us(I2C_PORT, I2cAddress, DispStrtLn, sizeof(DispStrtLn), false, 100);
    sleep_us(10);
    // uint16_t TxStart[NumOledByte + 1];
    txBufferDma[0] = {CoOnlyData | DCGddram};
    for (uint32_t i = 0; i < NumOledByte; ++i)
    {
        txBufferDma[i + 1] = buffer[i];
    }
    txBufferDma[NumOledByte] |= (1 << I2C_IC_DATA_CMD_STOP_LSB);

    dma_channel_configure(
        dma_ch,          // Channel to be configured
        &dma_conf,            // The configuration we just created
        dst,           // The initial write address
        txBufferDma,           // The initial read address
        NumOledByte + 1, // Number of transfers; in this case each is 1 byte.
        true           // Start immediately.
    );
    transmitting = true;
}

int transmit()
{
    uint8_t SetClmnAddr[] ={SetColumnAddress | 0, 0, 127};
    uint8_t SetPageAddr[] ={SetPageAddress | 0, 0, 7};
    i2c_write_timeout_per_char_us(I2C_PORT, I2cAddress, SetClmnAddr, sizeof(SetClmnAddr), false, 100);
    sleep_us(10);
    i2c_write_timeout_per_char_us(I2C_PORT, I2cAddress, SetPageAddr, sizeof(SetPageAddr), false, 100);
    sleep_us(10);

    uint8_t DispStrtLn[] = {SetDispStartLine | 0};
    i2c_write_timeout_per_char_us(I2C_PORT, I2cAddress, DispStrtLn, sizeof(DispStrtLn), false, 100);
    sleep_us(10);
    uint8_t TxStart[NumOledByte + 1] = {CoOnlyData | DCGddram};
    memcpy(&TxStart[1], buffer, NumOledByte);
    auto ret = i2c_write_timeout_per_char_us(I2C_PORT, I2cAddress, TxStart, sizeof(TxStart), false, 100);
    // auto ret = i2c_write_blocking(I2C_PORT, I2cAddress, TxStart, sizeof(TxStart), false);
    sleep_us(10);

    return ret;
}

void copyIntoBuffer(uint8_t *data, uint32_t size)
{
    const auto Size = std::min<uint32_t>(size, NumOledByte);
    std::memcpy(buffer, data, Size);
}

uint8_t getSample()
{
    return buffer[0];
}
} // namespace oled
