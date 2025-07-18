/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000  // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM (gpio_num_t)14

#define EXAMPLE_LED_NUMBERS 4
#define EXAMPLE_CHASE_SPEED_MS 1000

static const char *TAG = "example";

static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];

rmt_channel_handle_t led_chan = NULL;
rmt_encoder_handle_t led_encoder = NULL;
rmt_transmit_config_t tx_config = {
  .loop_count = 0,  // no transfer loop
};
void setup() {

  ESP_LOGI(TAG, "Create RMT TX channel");
  rmt_tx_channel_config_t tx_chan_config = {
    .gpio_num = RMT_LED_STRIP_GPIO_NUM,
    .clk_src = RMT_CLK_SRC_DEFAULT,  // select source clock
    .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
    .mem_block_symbols = 64,  // increase the block size can make the LED less flickering
    .trans_queue_depth = 4,   // set the number of transactions that can be pending in the background
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

  ESP_LOGI(TAG, "Install led strip encoder");
  led_strip_encoder_config_t encoder_config = {
    .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
  };
  ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

  ESP_LOGI(TAG, "Enable RMT TX channel");
  ESP_ERROR_CHECK(rmt_enable(led_chan));
  ESP_LOGI(TAG, "Start LED rainbow chase");
}


void loop() {
  for (int j = 0; j < EXAMPLE_LED_NUMBERS; j++) {
    led_strip_pixels[j * 3 + 0] = 255;
    led_strip_pixels[j * 3 + 1] = 0;
    led_strip_pixels[j * 3 + 2] = 0;
  }
  // Flush RGB values to LEDs
  ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
  vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));

  for (int j = 0; j < EXAMPLE_LED_NUMBERS; j++) {
    led_strip_pixels[j * 3 + 0] = 0;
    led_strip_pixels[j * 3 + 1] = 255;
    led_strip_pixels[j * 3 + 2] = 0;
  }
  // Flush RGB values to LEDs
  ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
  vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));

  for (int j = 0; j < EXAMPLE_LED_NUMBERS; j++) {
    led_strip_pixels[j * 3 + 0] = 0;
    led_strip_pixels[j * 3 + 1] = 0;
    led_strip_pixels[j * 3 + 2] = 255;
  }
  // Flush RGB values to LEDs
  ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
  vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
}

void vBlink1Led(uint32_t u32Position, uint8_t pu8PixelColor[3], uint32_t u8Duration_uS) {
  led_strip_pixels[u32Position * 3 + 0] = pu8PixelColor[0];
  led_strip_pixels[u32Position * 3 + 1] = pu8PixelColor[1];
  led_strip_pixels[u32Position * 3 + 2] = pu8PixelColor[2];
  delay(u8Duration_uS);
}
