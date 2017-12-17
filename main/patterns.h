
#ifndef __PATTERNS_H__
#define __PATTERNS_H__

#include <stdint.h>

#define MAX_PATTERNS 1

typedef struct
{
    uint8_t brightness;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_state;

void do_pattern(led_state * leds, uint32_t num_leds, uint8_t pattern_id, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue);

#endif // __PATTERNS_H__
