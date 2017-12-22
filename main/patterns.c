#include <stdlib.h>
#include <string.h>

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

static void do_tracer(led_state * leds, uint32_t num_leds, const patterns_config * patterns_config)
{
    const pattern1_config * config = &patterns_config->pattern1;
    static uint32_t count = 0;
    const rgba * current_palette = get_palette(patterns_config->global.palette);
    static unsigned int current_palette_pos = 0;

//    // for safety:
//    if (config->speed == 0)
//    {
//        config->speed = 1;
//    }

    count = (count + 1) % (num_leds * config->speed);
    uint32_t pos = (count / config->speed);

    static bool direction = false;
    if (!config->bounce)
    {
        direction = config->direction;
    }

    if (count == 0)
    {
        ESP_LOGI(TAG, "current_palette_pos %d, config->palette_step %d", current_palette_pos, config->palette_step);
        current_palette_pos = (current_palette_pos + config->palette_step) % PALETTE_SIZE;
        ESP_LOGI(TAG, "new current_palette_pos %d", current_palette_pos);

        if (config->bounce)
        {
            direction = !direction;
            ESP_LOGI(TAG, "bounce: direction %d", direction);
        }
    }

    if (direction)
    {
        pos = num_leds - pos - 1;
    }

    leds[pos].brightness = patterns_config->global.brightness;
    leds[pos].red = current_palette[current_palette_pos].r;
    leds[pos].green = current_palette[current_palette_pos].g;
    leds[pos].blue = current_palette[current_palette_pos].b;
}

static void do_flasher(led_state * leds, uint32_t num_leds, const patterns_config * patterns_config)
{
    const pattern2_config * config = &patterns_config->pattern2;

    const rgba * current_palette = get_palette(patterns_config->global.palette);
    rgba colour[FLASHER_BUTTONS] = { 0 };
    rgba * col = &colour[0];

    for (int i = 0; i < FLASHER_BUTTONS; ++i)
    {
        if (config->push[i])
        {
            colour[i] = current_palette[config->pos[i]];
            col = &colour[i];  // last one set
        }
    }

    for (uint32_t i = 0; i < num_leds; ++i)
    {
        if (config->split)
        {
            col = &colour[i % FLASHER_BUTTONS];
        }
        leds[i].brightness = patterns_config->global.brightness;
        leds[i].red = col->r;
        leds[i].green = col->g;
        leds[i].blue = col->b;
    }
}

static void do_chaser(led_state * leds, uint32_t num_leds, const patterns_config * patterns_config)
{
    const pattern3_config * config = &patterns_config->pattern3;

    const rgba * current_palette = get_palette(patterns_config->global.palette);
    static unsigned int current_palette_pos = 0;
    static uint32_t count = 0;
    static uint32_t num_locations = 0;

    // keep copies to detech changes
    static uint8_t brightness = 0;
    static uint8_t palette = 0;
    static uint8_t number = 0;
    static uint8_t length = 0;
    static uint8_t gap = 0;
    static uint8_t palette_step = 0;

    static led_state * buffer = 0;
    static uint32_t buffer_size = 0;

    bool reset = false;

    if (brightness != patterns_config->global.brightness)
    {
        brightness = patterns_config->global.brightness;
        reset = true;
    }

    if (palette != patterns_config->global.palette)
    {
        palette = patterns_config->global.palette;
        reset = true;
    }

    if (number != config->number)
    {
        number = config->number;
        reset = true;
    }

    if (length != config->length)
    {
        length = config->length;
        reset = true;
    }

    if (gap != config->gap)
    {
        gap = config->gap;
        reset = true;
    }

    if (palette_step != config->palette_step)
    {
        palette_step = config->palette_step;
        reset = true;
    }

    if (reset)
    {
        // render the chasers into a side buffer
        num_locations = number * (length + gap);
        if (num_locations < num_leds)
            num_locations = num_leds;
        buffer_size = sizeof(led_state) * num_locations;

        free(buffer);
        buffer = malloc(buffer_size);
        memset(buffer, 0, buffer_size);

        for (int i = 0; i < number; ++i)
        {
            current_palette_pos = i * palette_step;
            for (int j = 0; j < length; ++j)
            {
                uint32_t pos = i * (length + gap) + j;
                buffer[pos].brightness = brightness;
                buffer[pos].red = current_palette[current_palette_pos].r;
                buffer[pos].green = current_palette[current_palette_pos].g;
                buffer[pos].blue = current_palette[current_palette_pos].b;
            }
        }
    }

    // slide the viewport over the unchanging array of chasers
    count = (count + 1) % (num_locations * config->speed);
    uint32_t pos = (count / config->speed);

    for (uint32_t i = 0; i < num_leds; ++i)
    {
        uint32_t index = (i + pos) % num_locations;
        if (config->direction)
        {
            leds[num_leds - i - 1] = buffer[index];
        }
        else
        {
            leds[i] = buffer[index];
        }
    }
}

void patterns_init(void)
{
    ESP_LOGI(TAG, "%d palettes available", NUM_PALETTES);

    // defaults
    g_patterns_config.global.brightness = 16;
    g_patterns_config.global.palette = 73;
    g_patterns_config.pattern0.mode = 0;
    g_patterns_config.pattern0.cycle_speed = 10;

    g_patterns_config.pattern1.palette_step = 32;
    g_patterns_config.pattern1.direction = 0;
    g_patterns_config.pattern1.speed = 4;

    g_patterns_config.pattern2.pos[0] = 0;
    g_patterns_config.pattern2.pos[1] = 51;
    g_patterns_config.pattern2.pos[2] = 102;
    g_patterns_config.pattern2.pos[3] = 153;
    g_patterns_config.pattern2.pos[4] = 204;
    g_patterns_config.pattern2.pos[5] = 255;

    g_patterns_config.pattern3.number = 8;
    g_patterns_config.pattern3.length = 8;
    g_patterns_config.pattern3.gap = 8;
    g_patterns_config.pattern3.palette_step = 32;
    g_patterns_config.pattern3.speed = 4;
    g_patterns_config.pattern3.direction = 0;
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
        case 3:
            do_chaser(leds, num_leds, patterns_config);
            break;
        default:
            ESP_LOGE(TAG, "Unsupported pattern ID %d", patterns_config->global.active_pattern);
            break;
    }
}


