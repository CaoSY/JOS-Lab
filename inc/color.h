#ifndef JOS_INC_COLOR_H
#define JOS_INC_COLOR_H


/*
 * To control text color, each 8-bit are prefixed with another 8-bit attribute. If the character is
 * sent to a CGA device, all 16 bits are written into the crt buffer and a colored character is displayed.
 * If the character is sent to a serial port or a lpt port, the 16 bits is truncated to 8 bits. Only
 * 8-bit character part is sent.
 * 
 * +-----------------------------------------------+-----------------------------------------------+
 * |                   Attribute                   |                   Character                   |
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * |    Background color   |    Foreground color   |                  Code point                   |
 * +-----------------------+-----------------------+-----------------------------------------------+
 * 
 * 
 * reference:
 * 1. http://blog.csdn.net/scnu20142005027/article/details/51264186
 * 2. https://en.wikipedia.org/wiki/VGA-compatible_text_mode#Text_buffer
 * 3. https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
 * 4. http://ascii-table.com/ansi-escape-sequences.php
 * 5. http://rrbrandt.dee.ufcg.edu.br/en/docs/ansi/
 */


extern int __textcolor;                     // global variable for text color attribute

#define BACK_SHIFT                          12
#define FORE_SHIFT                          8
#define BACK_GROUND(_COLOR_)                ( ((_COLOR_) << BACK_SHIFT) | (TEXT_SILVER << FORE_SHIFT))
#define FORE_GROUND(_COLOR_)                ( (_COLOR_) << FORE_SHIFT )
#define TEXT_COLOR(_FORE_, _BACK_)          ( ((_FORE_) << FORE_SHIFT) | ((_BACK_) << BACK_SHIFT) )
#define SET_TEXT_COLOR(_color_)             ( __textcolor = _color_ )
#define RESET_TEXT_COLOR()                  ( __textcolor = TEXT_DF_COLOR )
#define SET_CHAR_COLOR(_char_, _color_)     ( _char_ |= (~0xff & _color_) )


#define TEXT_BLACK              0x00        // black foreground color, RGB: #000000
#define TEXT_NAVY_BLUE          0x01        // navy blue foreground color, RGB: #000080
#define TEXT_OFFICE_GREEN       0x02        // office green foreground color, RGB: #008000
#define TEXT_TEAL               0x03        // teal foreground color, RGB: #008080 
#define TEXT_MAROON             0x04        // maroon foreground color, RGB: #800000
#define TEXT_PURPLE             0x05        // purple foreground color, RGB: #800080
#define TEXT_OLIVE              0x06        // olive foreground color, RGB: #808000
#define TEXT_SILVER             0x07        // sliver foreground, RGB: #808000
#define TEXT_GRAY               0x08        // gray foreground color, RGB: #808000
#define TEXT_BLUE               0x09        // blue foreground color, RGB: #0000ff
#define TEXT_GREEN              0x0a        // green foreground color, RGB: #00ff00
#define TEXT_CYAN               0x0b        // cyan foreground color, RGB: #00ffff
#define TEXT_RED                0x0c        // red foregound color, RGB: #ff0000
#define TEXT_MAGENTA            0x0d        // magenta foreground color, RGB: #ff00ff
#define TEXT_YELLOW             0x0e        // yellow foreground color, RGB: #ffff00
#define TEXT_WHITE              0x0f        // white foreground color, RGB: #ffffff

#define TEXT_DF_COLOR           TEXT_COLOR(TEXT_SILVER, TEXT_BLACK)   // defualt text color


#endif /* !JOS_INC_COLOR_H */