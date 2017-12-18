#include "esp_log.h"
#include "patterns.h"
#include "palettes.h"

#define TAG "patterns"

patterns_config g_patterns_config = { 0 };

static void do_rgb(led_state * leds, uint32_t num_leds, const pattern0_config * config)
{
    static unsigned int count = 0;
    if (config->mode == 0)
    {
        for (uint32_t i = 0; i < num_leds; ++i)
        {
            leds[i].brightness = config->brightness;
            leds[i].red = config->red;
            leds[i].green = config->green;
            leds[i].blue = config->blue;
        }
    }
    else
    {
        bool cycling = config->cycle;
        static int cycle_direction = 1;
        static int cycle_pos = 0;

        unsigned int current_palette_index = config->palette;
        if (current_palette_index >= NUM_PALETTES)
        {
            current_palette_index = NUM_PALETTES - 1;
        }
        rgba * current_palette = palettes[current_palette_index];
        unsigned int current_palette_pos = cycling ? cycle_pos : config->palette_pos;

        if (cycling)
        {
            if (config->cycle_speed == 0 || count % config->cycle_speed == 0)
            {
                cycle_pos += cycle_direction;
            }
            if (cycle_pos >= PALETTE_SIZE)
            {
                cycle_pos = PALETTE_SIZE - 1;
                cycle_direction = -1;
            }
            if (cycle_pos < 0)
            {
                cycle_pos = 0;
                cycle_direction = 1;
            }
            current_palette_pos = cycle_pos;
        }

        for (uint32_t i = 0; i < num_leds; ++i)
        {
            leds[i].brightness = config->brightness;
            leds[i].red = current_palette[current_palette_pos].r;
            leds[i].green = current_palette[current_palette_pos].g;
            leds[i].blue = current_palette[current_palette_pos].b;
        }
    }

    ++count;
}

static void do_animated_strip(led_state * leds, uint32_t num_leds, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    static uint32_t count = 0;
    static uint32_t pos = 0;
    leds[pos].brightness = brightness;
    leds[pos].red = red;
    leds[pos].green = green;
    leds[pos].blue = blue;
    pos = ((++count) / 4) % num_leds;
}

static void do_tracer(led_state * leds, uint32_t num_leds, const pattern1_config * config)
{
    static uint32_t count = 0;
    static uint32_t pos = 0;
    leds[pos].brightness = config->brightness;

    unsigned int num_palettes = sizeof(palettes) / sizeof(palettes[0]);
    unsigned int current_palette_index = config->palette;

    if (current_palette_index >= num_palettes)
    {
        current_palette_index = num_palettes - 1;
    }
    rgba * current_palette = palettes[current_palette_index];

    unsigned current_palette_pos = config->palette_step;

    leds[pos].red = current_palette[current_palette_pos].r;
    leds[pos].green = current_palette[current_palette_pos].g;
    leds[pos].blue = current_palette[current_palette_pos].b;
    pos = ((++count) / 4) % num_leds;
}

void patterns_init(void)
{
    ESP_LOGI(TAG, "%d palettes available", NUM_PALETTES);

    g_patterns_config.pattern0.brightness = 16;
    g_patterns_config.pattern0.mode = 0;
}

void do_pattern(led_state * leds, uint32_t num_leds, const patterns_config * config)
{
    switch (config->active)
    {
        case 0:
            do_rgb(leds, num_leds, &config->pattern0);
            break;
        case 1:
            do_tracer(leds, num_leds, &config->pattern1);
            break;
        default:
            ESP_LOGE(TAG, "Unsupported pattern ID %d", config->active);
            break;
    }
}


