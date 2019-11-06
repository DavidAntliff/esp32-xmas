#ifndef CONSTANTS
#define CONSTANTS

#define NUM_LEDS        120                               // number of LEDs in strip
#define NUM_START_BYTES 4                                 // number of zero bytes required at start of transaction
#define NUM_RESET_BYTES 4                                 // number of zero bytes required at end of transaction (SK9822)
#define BYTES_PER_LED   4                                 // number of bytes per LED (brightness, blue, green, red)
#define NUM_ZEROS       (((NUM_LEDS + 1) / 2 + 7) / 8)    // final zero bytes required to clock through all data
#define NUM_BYTES       (NUM_START_BYTES + BYTES_PER_LED * NUM_LEDS + NUM_RESET_BYTES + NUM_ZEROS)  // total number of bytes in SPI transaction

#define MAX_BRIGHTNESS 31
#define MAX_RED        255
#define MAX_GREEN      255
#define MAX_BLUE       255

#define BLUE_LED_GPIO (GPIO_NUM_2)

#define LED_FRAME_BASE 0xE0

#define SPI_DMA_CHANNEL 1
#define SPI_CLK_RATE    10000000   // Hz

#endif // CONSTANTS
