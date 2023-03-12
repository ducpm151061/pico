/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "as5600.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  AS5600 encoder;

  long revolutions = 0; // number of revolutions the encoder has made
  double position = 0;  // the calculated value the encoder is at
  double pos;           // raw value from AS5600
  long lastOutput;      // last output from AS5600
  float angle;
  pos = encoder.getPosition();
  lastOutput = pos;
  position = pos;
  while (true) {
    printf("LED ON\r\n");
    gpio_put(LED_PIN, 1);
    sleep_ms(250);

    pos = encoder.getPosition(); // get the raw value of the encoder

    if ((lastOutput - pos) > 2047) // check if a full rotation has been made
      revolutions++;
    if ((lastOutput - pos) < -2047)
      revolutions--;

    position =
        revolutions * 4096 + pos; // calculate the position the the encoder is
                                  // at based off of the number of revolutions

    printf("position= %f\r\n", position);

    lastOutput = pos;

    angle = encoder.getAngle();
    printf("angle = %f\r\n", angle);
    printf("LED OFF\r\n");
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
  }
#endif
}