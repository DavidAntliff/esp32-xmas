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
        .clock_speed_hz = 10000,
        .spics_io_num = PIN_LED_CS, //-1,
        .flags = 0,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &handle));

    uint8_t data[3] = { 0 };
    spi_transaction_t trans_desc = {
        .address = 0,
        .command = 0,
        .flags = 0,
        .length = 3 * 8,  // bits
        .rxlength = 0,
        .tx_buffer = data,
        .rx_buffer = data,
    };

    data[0] = 0x12;
    data[1] = 0x34;
    data[2] = 0x56;

    ESP_LOGI(TAG, "... Transmitting.");
    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));

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

