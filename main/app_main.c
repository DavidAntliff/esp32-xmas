#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#define TAG "APP"

#define BLUE_LED_GPIO (GPIO_NUM_2)
#define PIN_LED_CLOCK (GPIO_NUM_25)
#define PIN_LED_DATA (GPIO_NUM_26)
#define PIN_LED_CS (GPIO_NUM_13)

void led_task(void * pvParameter)
{
	gpio_pad_select_gpio(BLUE_LED_GPIO);
	gpio_set_direction(BLUE_LED_GPIO, GPIO_MODE_OUTPUT);
	while(1)
	{
		gpio_set_level(BLUE_LED_GPIO, 0);
		vTaskDelay(1000 / portTICK_RATE_MS);
		gpio_set_level(BLUE_LED_GPIO, 1);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void led_start(spi_device_handle_t handle)
{
    uint8_t data[4] = { 0, 0, 0, 0 };
    spi_transaction_t trans_desc = {
        .address = 0,
        .command = 0,
        .flags = 0,
        .length = 4 * 8,  // bits
        .rxlength = 0,
        .tx_buffer = data,
        .rx_buffer = data,
    };

    ESP_LOGD(TAG, "... Transmitting.");
    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
}

void led_end(spi_device_handle_t handle, int num_leds)
{
    int num_end_bits = (num_leds + 1) / 2;
    int num_end_bytes = (num_end_bits + 7) / 8;

    // for now, just send four zero bytes
    uint8_t data[4] = { 0, 0, 0, 0 };
    spi_transaction_t trans_desc = {
        .address = 0,
        .command = 0,
        .flags = 0,
        .length = 4 * 8,  // bits
        .rxlength = 0,
        .tx_buffer = data,
        .rx_buffer = data,
    };

    ESP_LOGD(TAG, "... Transmitting.");
    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
}

void led_set(spi_device_handle_t handle, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t data[4] = { 0xe0 + brightness, blue, green, red };
    spi_transaction_t trans_desc = {
        .address = 0,
        .command = 0,
        .flags = 0,
        .length = 4 * 8,  // bits
        .rxlength = 0,
        .tx_buffer = data,
        .rx_buffer = data,
    };

    ESP_LOGD(TAG, "... Transmitting.");
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
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1 /*DMA channel*/));

    spi_device_handle_t handle;
    spi_device_interface_config_t dev_config = {
        .address_bits = 0,
        .command_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_posttrans = 0,
        .cs_ena_pretrans = 0,
        .clock_speed_hz = 10000000,
        .spics_io_num = PIN_LED_CS, //-1,
        .flags = 0,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &handle));

//    led_set(handle, 0x0f, 0xff, 0x00, 0x00);
//    led_set(handle, 0x0f, 0xff, 0x3f, 0x00);
//    led_set(handle, 0x0f, 0xff, 0xff, 0x00);
//    led_set(handle, 0x0f, 0x00, 0xff, 0x00);
//    led_set(handle, 0x0f, 0x00, 0x00, 0xff);
//    led_set(handle, 0x0f, 0xff, 0x00, 0xff);

    int num_leds = 60;
    uint8_t brightness = 0x01;
    uint8_t red = 0xff;
    uint8_t green = 0x00;
    uint8_t blue = 0x00;

    while (1)
    {
        led_start(handle);
        for (int i = 0; i < num_leds; ++i)
        {
            led_set(handle, brightness, red, green, blue);
        }
        led_end(handle, num_leds);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        uint8_t tmp = blue;
        blue = green;
        green = red;
        red = tmp;
    }


    ESP_LOGI(TAG, "... Removing device.");
    ESP_ERROR_CHECK(spi_bus_remove_device(handle));

    ESP_LOGI(TAG, "... Freeing bus.");
    ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));

    vTaskDelete(NULL);
}

void app_main()
{
    xTaskCreate(&led_task, "led_task", 2048, NULL, 5, NULL);
    xTaskCreate(&test_spi_task, "test_spi_task", 2048, NULL, 5, NULL);
}

