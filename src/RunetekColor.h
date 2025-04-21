#pragma once

#include <cinttypes>

namespace imp::math {
class RunetekColor {
public:
    static void InitColorTables();
    static void DestroyColorTables();

    static uint16_t RGBToHSL(uint32_t rgb);
    static uint32_t HSLToRGB(uint16_t hsl);

    static uint32_t HelperToRGB(uint16_t label);

private:
    static uint32_t* m_hslToRGB;
};
}
