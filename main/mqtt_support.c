/*
 * MIT License
 *
 * Copyright (c) 2017 David Antliff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "mqtt_support.h"
#include "constants.h"
#include "esp_mqtt.h"
#include "trie.h"
#include "patterns.h"

#define TAG "mqtt_support"

static trie * g_trie;

static void mqtt_status_callback(esp_mqtt_status_t status)
{
    ESP_LOGI(TAG, "mqtt_status_callback: %d", status);

    switch (status)
    {
        case ESP_MQTT_STATUS_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            gpio_set_level(BLUE_LED_GPIO, 1);

            // send a device status update
            const char * value = "MQTT connected";
            esp_mqtt_publish("xmas/status", (uint8_t*)value, strlen(value), 0, false);

            esp_mqtt_subscribe("xmas/#", 0);
            break;
        case ESP_MQTT_STATUS_DISCONNECTED:
            break;
        default:
            break;
    }
}

static void mqtt_message_callback(const char * topic, uint8_t * payload, size_t len)
{
    ESP_LOGI(TAG, "mqtt_message_callback: topic '%s', len %d", topic, len);
    esp_log_buffer_hex(TAG, payload, len);
    const char * data = (const char *)payload;

    // pattern selection is based on topic value: xmas/pattern/X
    if (strncmp(topic, "xmas/pattern/", strlen("xmas/pattern/")) == 0)
    {
        int index = 0;
        sscanf(topic, "xmas/pattern/%d", &index);
        g_patterns_config.global.active_pattern = index - 1;
        ESP_LOGI(TAG, "pattern %d", g_patterns_config.global.active_pattern);
    }
    else
    {
        // TODO: handle values other than uint8_ts
        uint8_t * result = trie_search(g_trie, topic);
        if (result)
        {
            *result = atoi(data);
            ESP_LOGI(TAG, "%s %d", topic, *result);
        }
        else
        {
            ESP_LOGI(TAG, "%s not handled", topic);
        }
    }
}

void mqtt_support_init(void)
{
    esp_mqtt_init(mqtt_status_callback, mqtt_message_callback, 256, 2000);

    g_trie = trie_create();
    trie_insert(g_trie, "xmas/global/brightness",  &g_patterns_config.global.brightness);
    trie_insert(g_trie, "xmas/global/palette",     &g_patterns_config.global.palette);

    trie_insert(g_trie, "xmas/1/mode",         &g_patterns_config.pattern0.mode);
    trie_insert(g_trie, "xmas/1/red",          &g_patterns_config.pattern0.red);
    trie_insert(g_trie, "xmas/1/green",        &g_patterns_config.pattern0.green);
    trie_insert(g_trie, "xmas/1/blue",         &g_patterns_config.pattern0.blue);
    trie_insert(g_trie, "xmas/1/palette_pos",  &g_patterns_config.pattern0.palette_pos);
    trie_insert(g_trie, "xmas/1/palette_show", &g_patterns_config.pattern0.palette_show);
    trie_insert(g_trie, "xmas/1/cycle",        &g_patterns_config.pattern0.cycle);
    trie_insert(g_trie, "xmas/1/cycle_speed",  &g_patterns_config.pattern0.cycle_speed);

    trie_insert(g_trie, "xmas/2/palette_step", &g_patterns_config.pattern1.palette_step);
    trie_insert(g_trie, "xmas/2/length",       &g_patterns_config.pattern1.length);
    trie_insert(g_trie, "xmas/2/speed",        &g_patterns_config.pattern1.speed);
    trie_insert(g_trie, "xmas/2/direction",    &g_patterns_config.pattern1.direction);
    trie_insert(g_trie, "xmas/2/bounce",       &g_patterns_config.pattern1.bounce);

    // assume FLASHER_BUTTONS = 6
    assert(FLASHER_BUTTONS == 6);
    trie_insert(g_trie, "xmas/3/split",       &g_patterns_config.pattern2.split);
    trie_insert(g_trie, "xmas/3/push1",       &g_patterns_config.pattern2.push[0]);
    trie_insert(g_trie, "xmas/3/push2",       &g_patterns_config.pattern2.push[1]);
    trie_insert(g_trie, "xmas/3/push3",       &g_patterns_config.pattern2.push[2]);
    trie_insert(g_trie, "xmas/3/push4",       &g_patterns_config.pattern2.push[3]);
    trie_insert(g_trie, "xmas/3/push5",       &g_patterns_config.pattern2.push[4]);
    trie_insert(g_trie, "xmas/3/push6",       &g_patterns_config.pattern2.push[5]);
    trie_insert(g_trie, "xmas/3/pos1",        &g_patterns_config.pattern2.pos[0]);
    trie_insert(g_trie, "xmas/3/pos2",        &g_patterns_config.pattern2.pos[1]);
    trie_insert(g_trie, "xmas/3/pos3",        &g_patterns_config.pattern2.pos[2]);
    trie_insert(g_trie, "xmas/3/pos4",        &g_patterns_config.pattern2.pos[3]);
    trie_insert(g_trie, "xmas/3/pos5",        &g_patterns_config.pattern2.pos[4]);
    trie_insert(g_trie, "xmas/3/pos6",        &g_patterns_config.pattern2.pos[5]);

    ESP_LOGI(TAG, "trie: count %d, size %d bytes", trie_count(g_trie, ""), trie_size(g_trie));
}
