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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "mqtt_support.h"
#include "mqtt.h"
#include "constants.h"

#define TAG "mqtt_support"

// let's try a global variable for now
mqtt_client * g_client = NULL;
extern bool g_mqtt_disconnected;

extern uint8_t g_brightness;
extern uint8_t g_red;
extern uint8_t g_green;
extern uint8_t g_blue;

void connected_cb(mqtt_client * client, mqtt_event_data_t * event_data)
{
    ESP_LOGI(TAG, "MQTT Connected");
    gpio_set_level(BLUE_LED_GPIO, 1);

    // need to delay a few seconds to avoid Connect being mixed into Subscribe requests
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // send a device status update
    const char * value = "MQTT connected";
    mqtt_publish(client, "xmas/status", value, strlen(value), 0, 0);

    // subscribe
    mqtt_subscribe(client, "xmas/brightness", 0);
    mqtt_subscribe(client, "xmas/red", 0);
    mqtt_subscribe(client, "xmas/blue", 0);
    mqtt_subscribe(client, "xmas/green", 0);
}

void disconnected_cb(mqtt_client * client, mqtt_event_data_t * event_data)
{
    ESP_LOGI(TAG, "MQTT Disconnected");
    gpio_set_level(BLUE_LED_GPIO, 0);

    g_mqtt_disconnected = true;
}

void reconnect_cb(mqtt_client * client, mqtt_event_data_t * event_data)
{
    // send a device status update
    const char * value = "MQTT reconnected";
    mqtt_publish(client, "xmas/status", value, strlen(value), 0, 0);
}

void subscribe_cb(mqtt_client * client, mqtt_event_data_t * event_data)
{
}

void publish_cb(mqtt_client * client, mqtt_event_data_t * event_data)
{
}

void data_cb(mqtt_client * client, mqtt_event_data_t * event_data)
{
    if (event_data->data_offset == 0) {
        char *topic = malloc(event_data->topic_length + 1);
        memcpy(topic, event_data->topic, event_data->topic_length);
        topic[event_data->topic_length] = 0;
        ESP_LOGD(TAG":mqtt", "[APP] Publish topic: %s", topic);

        char *data = malloc(event_data->data_length + 1);
        memcpy(data, event_data->data, event_data->data_length);
        data[event_data->data_length] = 0;

        // data is null-terminated so can be treated like a string if required

        ESP_LOGD(TAG":mqtt", "[APP] Publish data[%d/%d bytes]",
                 event_data->data_length + event_data->data_offset,
                 event_data->data_total_length);
        //esp_log_buffer_hex(TAG":mqtt", data, event_data->data_length + 1);

        if (strcmp(topic, "xmas/brightness") == 0)
        {
            float fval = atof(data);
            g_brightness = fval * MAX_BRIGHTNESS;
            ESP_LOGI(TAG, "brightness %f: %d", fval, g_brightness);
        }
        else if (strcmp(topic, "xmas/red") == 0)
        {
            float fval = atof(data);
            g_red = fval * MAX_RED;
            ESP_LOGI(TAG, "red %f: %d", fval, g_red);
        }
        else if (strcmp(topic, "xmas/green") == 0)
        {
            float fval = atof(data);
            g_green = fval * MAX_GREEN;
            ESP_LOGI(TAG, "green %f: %d", fval, g_green);
        }
        else if (strcmp(topic, "xmas/blue") == 0)
        {
            float fval = atof(data);
            g_blue = fval * MAX_BLUE;
            ESP_LOGI(TAG, "blue %f: %d", fval, g_blue);
        }
        else
        {
            ESP_LOGI(TAG, "%s not handled", topic);
        }

        free(topic);
        free(data);
    }
    else
    {
        // TODO: how do we deal with this? When does it occur?
        ESP_LOGW(TAG":mqtt", "event_data->data_offset is not zero: %d", event_data->data_offset);
    }
}

mqtt_settings g_settings = {
    .host = CONFIG_MQTT_BROKER_IP_ADDRESS,
#if defined(CONFIG_MQTT_SECURITY_ON)
    .port = 8883, // encrypted
#else
    .port = CONFIG_MQTT_BROKER_TCP_PORT, // unencrypted
#endif
    .client_id = "xmas_mqtt_client",
    .username = "user",
    .password = "pass",
    .clean_session = 0,
    .keepalive = 120,
    .lwt_topic = "/lwt",
    .lwt_msg = "offline",
    .lwt_qos = 0,
    .lwt_retain = 0,
    .connected_cb = connected_cb,
    .disconnected_cb = disconnected_cb,
    //.reconnect_cb = reconnect_cb,
    .subscribe_cb = subscribe_cb,
    .publish_cb = publish_cb,
    .data_cb = data_cb
};
