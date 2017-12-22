#include "esp_log.h"
#include "patterns.h"
#include "palettes.h"

#define TAG "patterns"

patterns_config g_patterns_config = { 0 };

static const rgba * get_palette(uint8_t palette_index)
{
    if (palette_index >= NUM_PALETTES)
    {
        palette_index = NUM_PALETTES - 1;
    }
    return palettes[palette_index];
}

static void do_rgb(led_state * leds, uint32_t num_leds, const patterns_config * patterns_config)
{
    const pattern0_config * config = &patterns_config->pattern0;
    static unsigned int count = 0;
    if (config->mode == 0)
    {
        for (uint32_t i = 0; i < num_leds; ++i)
        {
            leds[i].brightness = patterns_config->global.brightness;
            leds[i].red = config->red;
            leds[i].green = config->green;
            leds[i].blue = config->blue;
        }
    }
    else
    {
        const rgba * current_palette = get_palette(patterns_config->global.palette);

        static int cycle_direction = 1;
        static int cycle_pos = 0;
        bool cycling = config->cycle;
        unsigned int current_palette_pos = cycling ? cycle_pos : config->palette_pos;

        if (config->palette_show)
        {
            float palette_pitch = (float)PALETTE_SIZE / (float)num_leds;
            for (uint32_t i = 0; i < num_leds; ++i)
            {
                current_palette_pos = (unsigned int)(palette_pitch * i);
                leds[i].brightness = patterns_config->global.brightness;
                leds[i].red = current_palette[current_palette_pos].r;
                leds[i].green = current_palette[current_palette_pos].g;
                leds[i].blue = current_palette[current_palette_pos].b;
            }
        }
        else
        {
            if (cycling)
            {
                int palette_step = config->cycle_speed < 10 ? 10 - config->cycle_speed : 1;
                int count_period = config->cycle_speed / 10;

                if (count_period == 0 || count % count_period == 0)
                {
                    cycle_pos += cycle_direction * palette_step;
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
                leds[i].brightness = patterns_config->global.brightness;
                leds[i].red = current_palette[current_palette_pos].r;
                leds[i].green = current_palette[current_palette_pos].g;
                leds[i].blue = current_palette[current_palette_pos].b;
            }
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

static void do_tracer(led_state * leds, uint32_t num_leds, const patterns_config * patterns_config)
{
    const pattern1_config * config = &patterns_config->pattern1;
    static uint32_t count = 0;
    static uint32_t pos = 0;
    leds[pos].brightness = patterns_config->global.brightness;

    const rgba * current_palette = get_palette(patterns_config->global.palette);
    unsigned current_palette_pos = config->palette_step;

    leds[pos].red = current_palette[current_palette_pos].r;
    leds[pos].green = current_palette[current_palette_pos].g;
    leds[pos].blue = current_palette[current_palette_pos].b;
    pos = ((++count) / 4) % num_leds;
}

static void do_flasher(led_state * leds, uint32_t num_leds, const patterns_config * patterns_config)
{
    const rgba * current_palette = get_palette(patterns_config->global.palette);
    rgba colour[FLASHER_BUTTONS] = { 0 };
    rgba * col = &colour[0];

    for (int i = 0; i < FLASHER_BUTTONS; ++i)
    {
        if (patterns_config->pattern2.push[i])
        {
            colour[i] = current_palette[patterns_config->pattern2.pos[i]];
            col = &colour[i];  // last one set
        }
    }

    for (uint32_t i = 0; i < num_leds; ++i)
    {
        if (patterns_config->pattern2.split)
        {
            col = &colour[i % FLASHER_BUTTONS];
        }
        leds[i].brightness = patterns_config->global.brightness;
        leds[i].red = col->r;
        leds[i].green = col->g;
        leds[i].blue = col->b;
    }
}

void patterns_init(void)
{
    ESP_LOGI(TAG, "%d palettes available", NUM_PALETTES);

    // defaults
    g_patterns_config.global.brightness = 16;
    g_patterns_config.pattern0.mode = 0;
    g_patterns_config.pattern0.cycle_speed = 10;

    g_patterns_config.pattern2.pos[0] = 0;
    g_patterns_config.pattern2.pos[1] = 51;
    g_patterns_config.pattern2.pos[2] = 102;
    g_patterns_config.pattern2.pos[3] = 153;
    g_patterns_config.pattern2.pos[4] = 204;
    g_patterns_config.pattern2.pos[5] = 255;
}

void do_pattern(led_state * leds, uint32_t num_leds, const patterns_config * patterns_config)
{
    switch (patterns_config->global.active_pattern)
    {
        case 0:
            do_rgb(leds, num_leds, patterns_config);
            break;
        case 1:
            do_tracer(leds, num_leds, patterns_config);
            break;
        case 2:
            do_flasher(leds, num_leds, patterns_config);
            break;
        default:
            ESP_LOGE(TAG, "Unsupported pattern ID %d", patterns_config->global.active_pattern);
            break;
    }
}


