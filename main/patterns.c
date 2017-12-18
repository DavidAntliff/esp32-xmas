#include "esp_log.h"
#include "patterns.h"

#define TAG "patterns"

patterns_config g_patterns_config;

static void do_direct(led_state * leds, uint32_t num_leds, const pattern0_config * config)
{
    for (uint32_t i = 0; i < num_leds; ++i)
    {
        leds[i].brightness = config->brightness;
        leds[i].red = config->red;
        leds[i].green = config->green;
        leds[i].blue = config->blue;
    }
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

static void do_tracer(led_state * leds, uint32_t num_leds)
{

}

void do_pattern(led_state * leds, uint32_t num_leds, const patterns_config * config)
{
    switch (config->active)
    {
        case 0:
            do_direct(leds, num_leds, &config->pattern0);
            break;
        case 1:
            //do_animated_strip(leds, num_leds, brightness, red, green, blue);
            do_tracer(leds, num_leds);
            break;
        default:
            ESP_LOGE(TAG, "Unsupported pattern ID %d", config->active);
            break;
    }
}
