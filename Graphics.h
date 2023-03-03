/**
 * @file Graphics.h
 * @brief
 * 
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <cstdint>
#include <string>
#include "font.h"

namespace graphics
{

void point(int32_t x, int32_t y, bool reverse = false);
void drawGlyph(const Glyph &g, const Font &f, int32_t x, int32_t y, bool reverse = false);
void drawString(const std::string &s, const Font &f, int32_t x, int32_t y, bool reverse = false);
void drawRect(int32_t x, int32_t y, int32_t width, int32_t height, bool reverse = false);
void clear();
void setToDispBuffer();

} // namespace graphics


#endif // GRAPHICS_H
