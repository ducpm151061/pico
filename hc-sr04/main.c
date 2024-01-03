#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include <hardware/gpio.h>
#include <stdio.h>

#ifndef PICO_DEFAULT_LED_PIN
#error blink example requires a board with a regular LED
#endif

#define ECHO_PIN 4
#define TRIGGER_PIN 5
#define TIME_OUT 5000

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
  uint64_t start = time_us_64();
  uint64_t abort = start + timeout;

  if (pin > 29) {
    printf("ERROR: Illegal pin in pulseIn (%d)\n", pin);
    return 0;
  }

  // Wait for deassert, if needed
  while ((!!gpio_get(pin) != !state) && (time_us_64() < abort))
    ;
  if (time_us_64() >= abort) {
    return 0;
  }

  // Wait for assert
  while ((!!gpio_get(pin) != !!state) && (time_us_64() < abort))
    ;
  uint64_t begin = time_us_64();
  if (begin >= abort) {
    return 0;
  }

  // Wait for deassert
  while ((!!gpio_get(pin) != !state) && (time_us_64() < abort))
    ;
  uint64_t end = time_us_64();
  if (end >= abort) {
    return 0;
  }

  return end - begin;
}

float GetDistance() {
  unsigned long duration;
  float distanceCm;

  gpio_put(TRIGGER_PIN, 0);
  sleep_us(2);
  gpio_put(TRIGGER_PIN, 1);
  sleep_us(10);
  gpio_put(TRIGGER_PIN, 0);

  duration = pulseIn(ECHO_PIN, 1, TIME_OUT);
  // convert to distance
  distanceCm = (duration * 0.0343) / 2;

  return distanceCm;
}

int main() {
  stdio_init_all();
  gpio_init(TRIGGER_PIN);
  gpio_init(ECHO_PIN);
  gpio_set_dir(TRIGGER_PIN, GPIO_OUT);
  gpio_set_dir(ECHO_PIN, GPIO_IN);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  while (true) {
    printf("Distance: %f\n", GetDistance());
  }
}
