#include "RunetekColor.h"

#include "helperColors.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include <iostream>
#include <tuple>
#include <unordered_map>

namespace imp::math {
uint32_t* RunetekColor::m_hslToRGB = nullptr;
static std::once_flag initFlag;
static std::unordered_map<uint32_t, uint16_t> exactMap;
static std::unordered_map<uint32_t, uint16_t> cache;

void RunetekColor::InitColorTables()
{
    constexpr float kOneThird = 1.0f / 3.0f;
    constexpr float kTwoThirds = 2.0f / 3.0f;
    assert(!m_hslToRGB);
    m_hslToRGB = new uint32_t[0x10000];
    for (uint32_t hsl = 0; hsl < 0x10000; hsl++) {
        float hue = (static_cast<float>(hsl >> 10) + 0.5f) / 64.0f;
        float sat = (static_cast<float>(hsl >> 7 & 0x7) + 0.5f) / 8.0f;
        float lum = static_cast<float>(hsl & 0x7f) / 128.0f;
        float red = lum;
        float green = lum;
        float blue = lum;
        if (sat != 0.0) {
            float q;
            if (lum >= 0.5) {
                q = lum + sat - lum * sat;
            } else {
                q = lum * (sat + 1.0f);
            }
            float p = 2.0f * lum - q;

            float t1 = hue + kOneThird;
            if (t1 > 1.0f) {
                t1 -= 1.0;
            }
            float t2 = hue - kOneThird;
            if (t2 < 0.0) {
                t2 += 1.0;
            }

            if (6.0f * t1 < 1.0f) {
                red = p + (q - p) * 6.0f * t1;
            } else if (2.0f * t1 < 1.0f) {
                red = q;
            } else if (3.0f * t1 >= 2.0) {
                red = 2.0f * lum - q;
            } else {
                red = p + (q - p) * (kTwoThirds - t1) * 6.0f;
            }

            if (6.0f * hue < 1.0f) {
                green = p + (q - p) * 6.0f * hue;
            } else if (2.0f * hue < 1.0f) {
                green = q;
            } else if (3.0f * hue < 2.0) {
                green = p + (q - p) * (kTwoThirds - hue) * 6.0f;
            } else {
                green = 2.0f * lum - q;
            }

            if (6.0f * t2 < 1.0f) {
                blue = p + (q - p) * 6.0f * t2;
            } else if (2.0f * t2 < 1.0f) {
                blue = q;
            } else if (3.0f * t2 < 2.0) {
                blue = p + (q - p) * (kTwoThirds - t2) * 6.0f;
            } else {
                blue = 2.0f * lum - q;
            }
        }
        int32_t unormRed = static_cast<int32_t>(red * 256.0f);
        int32_t unormGreen = static_cast<int32_t>(green * 256.0f);
        int32_t unormBlue = static_cast<int32_t>(blue * 256.0f);
        m_hslToRGB[hsl] = unormRed << 16 | unormGreen << 8 | unormBlue;
    }
}

void RunetekColor::DestroyColorTables()
{
    if (m_hslToRGB) {
        delete[] m_hslToRGB;
        m_hslToRGB = nullptr;
    }
}
uint16_t RunetekColor::RGBToHSL(uint32_t rgb)
{
    std::call_once(initFlag, [&]() {
        exactMap.reserve(65536);
        for (uint32_t i = 0; i < 65536; ++i) {
            exactMap[m_hslToRGB[i]] = i;
        }
        cache.reserve(4096);
    });

    if (auto it = exactMap.find(rgb); it != exactMap.end()) {
        return it->second;
    }
    if (auto it = cache.find(rgb); it != cache.end()) {
        return it->second;
    }

    uint32_t r0 = (rgb >> 16) & 0xff;
    uint32_t g0 = (rgb >> 8) & 0xff;
    uint32_t b0 = (rgb) & 0xff;

    uint32_t bestDiff = std::numeric_limits<uint32_t>::max();
    uint32_t bestIndex = 0;

    for (uint32_t i = 0; i < 65536; ++i) {
        uint32_t color = m_hslToRGB[i];
        uint32_t dr = r0 - (color >> 16 & 0xff);
        uint32_t dg = g0 - (color >> 8 & 0xff);
        uint32_t db = b0 - (color & 0xff);

        uint32_t diff = (dr * dr) + (dg * dg) + (db * db);
        if (diff < bestDiff) {
            bestDiff = diff;
            bestIndex = i;
            if (diff == 0) {
                break;
            }
        }
    }
    cache[rgb] = bestIndex;
    return bestIndex;
}

uint32_t RunetekColor::HSLToRGB(uint16_t hsl)
{
    if (!m_hslToRGB) {
        InitColorTables();
        assert(m_hslToRGB != nullptr);
    }
    return m_hslToRGB[hsl];
}

uint32_t RunetekColor::HelperToRGB(uint16_t value)
{
    uint8_t r = static_cast<uint8_t>(s_helperColors[value][0] * 255.0f);
    uint8_t g = static_cast<uint8_t>(s_helperColors[value][1] * 255.0f);
    uint8_t b = static_cast<uint8_t>(s_helperColors[value][2] * 255.0f);
    return (r << 16) | (g << 8) | b;
}
}
