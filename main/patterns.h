
#ifndef PATTERNS_H
#define PATTERNS_H

#include <stdint.h>

#define FLASHER_BUTTONS 6

typedef struct
{
    uint8_t brightness;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_state;

typedef struct
{
    uint8_t active_pattern;
    uint8_t brightness;
    uint8_t palette;      // current palette index
} global_config;

typedef struct
{
    uint8_t mode;   // 0 = RGB, 1 = Palette
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t palette_pos;  // current palette colour
    uint8_t cycle;
    uint8_t cycle_speed;
} pattern0_config;

typedef struct
{
    uint8_t brightness;
    uint8_t palette;
    uint8_t palette_step;
    uint8_t length;
    uint8_t speed;
    uint8_t direction;
    uint8_t bounce;
} pattern1_config;

typedef struct
{
    uint8_t split;
    uint8_t push[FLASHER_BUTTONS];
    uint8_t pos[FLASHER_BUTTONS];
} pattern2_config;

typedef struct
{
    global_config global;
    pattern0_config pattern0;
    pattern1_config pattern1;
    pattern2_config pattern2;
} patterns_config;

extern patterns_config g_patterns_config;

void patterns_init(void);
void do_pattern(led_state * leds, uint32_t num_leds, const patterns_config * config);

#endif // PATTERNS_H
