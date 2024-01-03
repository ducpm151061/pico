/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <stdio.h>

/// \tag::hello_uart[]

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main() {
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  // Set up our UART with the required speed.
  uart_init(UART_ID, BAUD_RATE);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Use some the various UART functions to send out data UART

  // In a default system, printf will also output via the default
  // Send out a character without any conversions
  while (1) {

    gpio_put(LED_PIN, 0);
    if (uart_is_readable(UART_ID)) {
      char ch = uart_getc(UART_ID);
      if (ch == 'a') {
        uart_puts(UART_ID, " Hello, UART!\r\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
      }
    }
  }
}

/// \end::hello_uart[]