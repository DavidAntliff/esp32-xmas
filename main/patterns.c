#include "esp_log.h"
#include "patterns.h"

#define TAG "patterns"

static void do_mono_strip(led_state * leds, uint32_t num_leds, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    for (uint32_t i = 0; i < num_leds; ++i)
    {
        leds[i].brightness = brightness;
        leds[i].red = red;
        leds[i].green = green;
        leds[i].blue = blue;
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


void do_pattern(led_state * leds, uint32_t num_leds, uint8_t pattern_id, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    switch (pattern_id)
    {
        case 0:
            do_mono_strip(leds, num_leds, brightness, red, green, blue);
            break;
        case 1:
            do_animated_strip(leds, num_leds, brightness, red, green, blue);
            break;
        default:
            ESP_LOGE(TAG, "Unsupported pattern ID %d", pattern_id);
            break;
    }
}
