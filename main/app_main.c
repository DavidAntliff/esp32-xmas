#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "constants.h"
#include "mqtt_support.h"
#include "wifi_support.h"
#include "patterns.h"

#define TAG "APP"

#define PIN_LED_CLOCK (GPIO_NUM_25)
#define PIN_LED_DATA (GPIO_NUM_26)
#define PIN_LED_CS (GPIO_NUM_13)

bool g_mqtt_disconnected = false;

uint8_t g_pattern = 0;
uint8_t g_brightness = 0;
uint8_t g_red = 0;
uint8_t g_green = 0;
uint8_t g_blue = 0;

void write_buffer(spi_device_handle_t handle, uint8_t * data, uint32_t len)
{
    spi_transaction_t trans_desc = {
        .address = 0,
        .command = 0,
        .flags = 0,
        .length = len * 8,  // bits
        .rxlength = 0,
        .tx_buffer = data,
        .rx_buffer = data,
    };

    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
}

void test_spi_task(void * pvParameter)
{
    spi_bus_config_t bus_config = {
        .miso_io_num = -1,
        .mosi_io_num = PIN_LED_DATA,
        .sclk_io_num = PIN_LED_CLOCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, SPI_DMA_CHANNEL));

    spi_device_handle_t handle;
    spi_device_interface_config_t dev_config = {
        .address_bits = 0,
        .command_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_posttrans = 0,
        .cs_ena_pretrans = 0,
        .clock_speed_hz = SPI_CLK_RATE,
        .spics_io_num = PIN_LED_CS, //-1,
        .flags = 0,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &handle));

    ESP_LOGI(TAG, "NUM_LEDS %d", NUM_LEDS);
    ESP_LOGI(TAG, "NUM_ZEROS %d", NUM_ZEROS);
    ESP_LOGI(TAG, "NUM_BYTES %d", NUM_BYTES);

    led_state leds[NUM_LEDS] = { 0 };

    while (1)
    {
        uint8_t buffer[NUM_BYTES] = { 0 };

        do_pattern(leds, NUM_LEDS, g_pattern, g_brightness, g_red, g_green, g_blue);

        for (int i = 0; i < NUM_LEDS; ++i)
        {
            buffer[NUM_START_BYTES + i * BYTES_PER_LED + 0] = LED_FRAME_BASE + leds[i].brightness;
            buffer[NUM_START_BYTES + i * BYTES_PER_LED + 1] = leds[i].blue;
            buffer[NUM_START_BYTES + i * BYTES_PER_LED + 2] = leds[i].green;
            buffer[NUM_START_BYTES + i * BYTES_PER_LED + 3] = leds[i].red;
        }
        write_buffer(handle, buffer, NUM_BYTES);

        vTaskDelay(1);
    }


    ESP_LOGI(TAG, "... Removing device.");
    ESP_ERROR_CHECK(spi_bus_remove_device(handle));

    ESP_LOGI(TAG, "... Freeing bus.");
    ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));

    vTaskDelete(NULL);
}

void app_main()
{
    gpio_pad_select_gpio(BLUE_LED_GPIO);
    gpio_set_direction(BLUE_LED_GPIO, GPIO_MODE_OUTPUT);

    mqtt_support_init();
    wifi_support_init();

    xTaskCreate(&test_spi_task, "test_spi_task", 8192, NULL, 4, NULL);
}

