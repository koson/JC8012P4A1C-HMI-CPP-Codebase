#pragma once

/**
 * @brief TH Niramit AS Thai+Latin fonts for LVGL
 *
 * Generated from "TH Niramit AS.ttf" (Windows system font)
 * Unicode ranges: 0x0020-0x007F (Basic Latin) + 0x0E00-0x0E7F (Thai)
 * Bpp: 4 (anti-aliased)
 *
 * Available sizes: 16, 20, 24, 32
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

    extern const lv_font_t th_niramit_16;
    extern const lv_font_t th_niramit_20;
    extern const lv_font_t th_niramit_24;
    extern const lv_font_t th_niramit_32;
    extern const lv_font_t th_niramit_40;
    extern const lv_font_t th_niramit_48;

    /**
     * @brief Select the closest Thai font for a given pixel size
     * @param size Desired font size in pixels
     * @return Pointer to the closest available lv_font_t
     */
    static inline const lv_font_t *th_niramit_select(int size)
    {
        if (size <= 18)
            return &th_niramit_16;
        if (size <= 22)
            return &th_niramit_20;
        if (size <= 28)
            return &th_niramit_24;
        if (size <= 36)
            return &th_niramit_32;
        if (size <= 44)
            return &th_niramit_40;
        return &th_niramit_48;
    }

    /**
     * @brief Check if a UTF-8 string contains any Thai characters (0x0E00-0x0E7F)
     * @param utf8 Null-terminated UTF-8 string
     * @return true if Thai characters found
     */
    static inline bool th_has_thai(const char *utf8)
    {
        if (!utf8)
            return false;
        const unsigned char *p = (const unsigned char *)utf8;
        while (*p)
        {
            // Thai Unicode block 0x0E00-0x0E7F encodes as:
            //   0xE0 0xB8 0x80 (U+0E00) ... 0xE0 0xB9 0xBF (U+0E7F)
            // Lead byte 0xE0, second byte 0xB8 or 0xB9
            if (p[0] == 0xE0 && (p[1] == 0xB8 || p[1] == 0xB9))
            {
                return true;
            }
            // Advance by UTF-8 sequence length
            if (p[0] < 0x80)
                p += 1;
            else if (p[0] < 0xE0)
                p += 2;
            else if (p[0] < 0xF0)
                p += 3;
            else
                p += 4;
        }
        return false;
    }

#ifdef __cplusplus
}
#endif
