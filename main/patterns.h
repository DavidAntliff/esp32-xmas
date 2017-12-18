
#ifndef PATTERNS_H
#define PATTERNS_H

#include <stdint.h>

#define MAX_PATTERNS 1

typedef struct
{
    uint8_t brightness;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_state;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t brightness;
} pattern0_config;

typedef struct
{
    uint8_t active;
    pattern0_config pattern0;
} patterns_config;

extern patterns_config g_patterns_config;

void do_pattern(led_state * leds, uint32_t num_leds, const patterns_config * config);

#endif // PATTERNS_H
