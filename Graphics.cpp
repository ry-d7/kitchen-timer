/**
 * @file Graphics.cpp
 * @brief
 * 
 */

#include "Graphics.h"
#include "font.h"
#include "oled.h"

namespace graphics
{
namespace
{
enum
{
    NormalNumXBytes = oled::OledWidth / 8,
    NormalNumYBytes = oled::OledHeight,
    TransNumXBytes = oled::OledWidth,
    TransNumYBytes = oled::OledHeight / 8,
};

uint8_t buf[oled::NumOledByte];
uint8_t transformBuf[oled::NumOledByte];
Glyph *searchGlyph(int8_t c, const Font &f)
{
    for (auto i = 0; i < f.num_chars; ++i)
    {
        if (f.glyph_table[i]->encoding == c)
        {
            return f.glyph_table[i];
        }
    }

    return nullptr;
}

void transform()
{
    const auto FromXIndexMax = oled::OledWidth >> 3;
    const auto ToYIndexMax = oled::OledHeight >> 3;
    for (auto y = 0; y < oled::OledHeight; ++y)
    {
        for (auto x = 0; x < oled::OledWidth; ++x)
        {
            const auto IndexFrom = (x >> 3) + y * FromXIndexMax;
            const auto ShiftFrom = x & 0x07;
            const auto DataFrom = (buf[IndexFrom] >> ShiftFrom) & 0x01;
            const auto IndexTo = x * ToYIndexMax + (y >> 3);
            const auto ShiftTo = y & 0x07;
            transformBuf[IndexTo] |= DataFrom << ShiftTo;
        }

    }
}

} // namespace

void point(int32_t x, int32_t y, bool reverse)
{
    if (oled::OledWidth <= x || oled::OledHeight <= y)
    {
        return;
    }
    auto byteX = x >> 3;
    uint8_t bitX = 0x01 << (x & 0x07);
    auto byteY = y;
    if (reverse)
    {
        buf[y * NormalNumXBytes + byteX] &= ~bitX;
        return;
    }
    buf[y * NormalNumXBytes + byteX] |= bitX;
    // printf("p(%d,%d)\n", x, y);
}

void drawGlyph(const Glyph &g, const Font &f, int32_t x, int32_t y, bool reverse)
{
    if (!g.bmp)
    {
        return;
    }
    // printf("glyph: %c\n", g.encoding);

    const auto Bottom = y + f.fbbx_y;
    const auto StartX = x + g.bbx_offx;
    const auto StartY = Bottom - g.bbx_y - g.bbx_offy;
    const auto GlyphBytesX = (g.bbx_x >> 3) + ((g.bbx_x & 0x07) != 0);

    for (auto j = 0; j < g.bbx_y; ++j)
    {
        for (auto i = 0; i < g.bbx_x; ++i)
        {
            auto byteX = i >> 3;
            auto bitX = 0x01 << (7 - (i & 0x07));
            const auto BmpIndex = j * GlyphBytesX + byteX;
            // printf("%d", (g.bmp[BmpIndex] & bitX) ? 1 : 0);
            if ((g.bmp)[BmpIndex] & bitX)
            {
                point(StartX + i, StartY + j, reverse);
            }
        }
        // puts("");
    }
}

void drawString(const std::string &s, const Font &f, int32_t x, int32_t y, bool reverse)
{
    const auto Size = s.size();
    auto currentX = x;
    const auto Baseline = y;
    for (uint32_t i = 0; i < Size; ++i)
    {
        const int8_t C = s.at(i);
        Glyph *g = searchGlyph(C, f);
        if (!g)
        {
            continue;
        }
        drawGlyph(*g, f, currentX, Baseline, reverse);
        // currentX += f.fbbx_x + f.fbbx_offx;
        currentX += g->dwidth;
    }
}

void drawRect(int32_t x, int32_t y, int32_t width, int32_t height, bool reverse)
{
}

void clear()
{
    for (auto i = 0; i < oled::NumOledByte; ++i)
    {
        buf[i] = 0;
        transformBuf[i] = 0;
    }
}

void setToDispBuffer()
{
    transform();
    oled::copyIntoBuffer(transformBuf, oled::NumOledByte);
}

} // namespace graphics
