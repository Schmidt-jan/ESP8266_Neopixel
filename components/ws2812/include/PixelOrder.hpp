#pragma once

#include <stdint.h>

/**
 * @brief Settings to define the color order.
 * Each entry is defined by 4 numbers in the octal system and
 * defines in which order the color-values are sent to the strip.
 * Also the number defines if the strip has a white led. 
 * 
 * The default ordering for the numbers is: White, Red, Green, Blue.
 * 
 * For an example we use a strip which requires the values in the order
 * BRWG. The BRWG value is defined by the octal number 2130. To get the
 * offsets for the colors we need to split the number into its digits (still octal).
 * B R W G
 * 2 1 3 0
 * 
 * If the R value is equal to the W value, the strip has no white led.
 * As this is not the case in our example, the strip has a white led.
 * 
 * For the implementation we need an array which holds the values for
 * pixel. As each pixel consists of 4 LEDs and each LED requires one byte
 * storage (0 - 255), we need 4 bytes for each pixel.
 * The values for the pixels are stored in an uint8_t array.
 * Lets assume we have 2 pixels which should light up in the colors
 * red and green. The array we need to send to the strip looks like this:
 * 
 *               |        Pixel 0            |       Pixel 1             |
 * Color(offset) | B(0) | R(1) | W(2) | G(3) | B(0) | R(1) | W(2) | G(3) |
 * Value         [ 0x00 , 0xff , 0x00 , 0xff , 0x00 , 0xff , 0x00 , 0xff ]
 *                  ￪
 *            uint8_t* pixels
 * The pixels array is defined as uint8_t* pixels. The first pixel colors 
 * are defined in the first four bytes. To access the bytes you can use 
 * the pixels pointer and add the offset of the color you want to access.
 * Remind that the offset resulted from the color order.
 */
namespace PixelOrder {
    enum PixelOrder: uint16_t {
        RGB = 00012,  ///< Red,   Green, Blue  (0012)
        RBG = 00021,  ///< Red,   Blue,  Green (0021)
        GRB = 00102,  ///< Green, Red,   Blue  (0102)
        GBR = 00120,  ///< Green, Blue,  Red   (0120)
        BRG = 00201,  ///< Blue,  Red,   Green (0201)
        BGR = 00210,   ///< Blue,  Green, Red   (0210)
        WRGB = 00123, ///< White, Red,   Green, Blue (0123)
        WRBG = 00132, ///< White, Red,   Blue,  Green (0132)
        WGRB = 00213, ///< White, Green, Red,   Blue (0213)
        WGBR = 00312, ///< White, Green, Blue,  Red  (0312)
        WBRG = 00231, ///< White, Blue,  Red,   Green (0231)
        WBGR = 00321, ///< White, Blue,  Green, Red   (0321)
        RWGB = 01023, ///< Red,   White, Green, Blue (1023)
        RWBG = 01032, ///< Red,   White, Blue,  Green (1032)
        RGWB = 02013, ///< Red,   Green, White, Blue (2013)
        RGBW = 03012, ///< Red,   Green, Blue,  White (3012)
        RBWG = 02031, ///< Red,   Blue,  White, Green (2031)
        RBGW = 03021, ///< Red,   Blue,  Green, White (3021)
        GWRB = 01203, ///< Green, White, Red,   Blue (0132)
        GWBR = 01302, ///< Green, White, Blue,  Red  (0312)
        GRWB = 02103, ///< Green, Red,   White, Blue (0231)
        GRBW = 03102, ///< Green, Red,   Blue,  White (0321)
        GBWR = 02301, ///< Green, Blue,  White, Red   (2031)  
        GBRW = 03201, ///< Green, Blue,  Red,   White (3021)
        BWRG = 01230, ///< Blue,  White, Red,   Green (0132)
        BWGR = 01320, ///< Blue,  White, Green, Red   (0312)
        BRWG = 02130, ///< Blue,  Red,   White, Green (0231)
        BRGW = 03120, ///< Blue,  Red,   Green, White (0321)
        BGWR = 02310, ///< Blue,  Green, White, Red   (2031)
        BGRW = 03210, ///< Blue,  Green, Red,   White (3021)
    };
}